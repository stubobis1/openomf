#include "ap_mechlab.h"

#include <math.h>
#include <string.h>
#include <ctype.h>

#include "archipelago/apconnect.h"
#include "archipelago/apitems.h"
#include "archipelago/apstate.h"
#include "formats/pilot.h"
#include "game/game_state.h"
#include "game/scenes/mechlab.h"
#include "utils/log.h"

// --- Private state ---

static scene   *g_scene     = NULL;
static int64_t  g_scout_loc = 0; // location ID of last armed scout; stale responses are suppressed

// --- Private helpers ---

static void ap_sync_pilot(sd_pilot *pilot) {
    int har = pilot->har_id;
    if(har >= 0 && har < 11) {
        pilot->arm_power       = APItems.har_stats[har][AP_STAT_ARM_POWER];
        pilot->leg_power       = APItems.har_stats[har][AP_STAT_LEG_POWER];
        pilot->arm_speed       = APItems.har_stats[har][AP_STAT_ARM_SPEED];
        pilot->leg_speed       = APItems.har_stats[har][AP_STAT_LEG_SPEED];
        pilot->armor           = APItems.har_stats[har][AP_STAT_ARMOR];
        pilot->stun_resistance = APItems.har_stats[har][AP_STAT_STUN_RESIST];
    }
    pilot->power      = APItems.pilot_stats[AP_PILOT_POWER];
    pilot->agility    = APItems.pilot_stats[AP_PILOT_AGILITY];
    pilot->endurance  = APItems.pilot_stats[AP_PILOT_ENDURANCE];
    pilot->har_trades = APItems.har_unlocked;
}

static void ap_drain_money(sd_pilot *pilot) {
    if(APStats.pending_money != 0) {
        log_debug("AP - money drain: +%d (pilot had %d)", APStats.pending_money, pilot->money);
        pilot->money += APStats.pending_money;
        APStats.pending_money = 0;
    }
}

// Format hint text: "ITEM - PLAYER", abbreviate "PROGRESSIVE " -> "PROG. ", wrap at 39 chars.
static void ap_format_hint(const char *item_name, const char *player_name, char *out, size_t out_size) {
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "%s - %s", item_name, player_name);
    for(int i = 0; tmp[i]; i++) tmp[i] = (char)toupper((unsigned char)tmp[i]);

    const char *from = "PROGRESSIVE ";
    const char *to   = "PROG. ";
    size_t flen = strlen(from), tlen = strlen(to);
    char sub[128];
    size_t si = 0, ti = 0;
    while(tmp[ti] && si + 1 < sizeof(sub)) {
        if(strncmp(tmp + ti, from, flen) == 0 && si + tlen < sizeof(sub)) {
            memcpy(sub + si, to, tlen); si += tlen; ti += flen;
        } else {
            sub[si++] = tmp[ti++];
        }
    }
    sub[si] = '\0';

    if(strlen(sub) > 39) {
        int wrap = -1;
        for(int i = 38; i >= 0; i--) { if(sub[i] == ' ') { wrap = i; break; } }
        if(wrap >= 0) { sub[wrap] = '\n'; sub[wrap + 1 + 39] = '\0'; }
        else          { sub[39]  = '\0'; }
    }
    snprintf(out, out_size, "%s", sub);
}

// Scout result cache — persists for the lifetime of the process.
// Avoids redundant server round-trips when revisiting the same buy/train slot.
#define SCOUT_CACHE_MAX 1024
typedef struct { int64_t loc; char item_name[80]; char player_name[48]; } scout_entry_t;
static scout_entry_t g_scout_cache[SCOUT_CACHE_MAX];
static int           g_scout_cache_len = 0;

static void ap_cache_store(int64_t loc, const char *item_name, const char *player_name) {
    for(int i = 0; i < g_scout_cache_len; i++) {
        if(g_scout_cache[i].loc == loc) return; // already cached
    }
    if(g_scout_cache_len < SCOUT_CACHE_MAX) {
        g_scout_cache[g_scout_cache_len].loc = loc;
        strncpy(g_scout_cache[g_scout_cache_len].item_name,   item_name,   79);
        strncpy(g_scout_cache[g_scout_cache_len].player_name, player_name, 47);
        g_scout_cache[g_scout_cache_len].item_name[79]   = '\0';
        g_scout_cache[g_scout_cache_len].player_name[47] = '\0';
        g_scout_cache_len++;
    }
}

// Returns true and sets the mechlab hint if loc is already in the cache.
static bool ap_cache_hit(scene *s, int64_t loc) {
    for(int i = 0; i < g_scout_cache_len; i++) {
        if(g_scout_cache[i].loc == loc) {
            char buf[79];
            ap_format_hint(g_scout_cache[i].item_name, g_scout_cache[i].player_name, buf, sizeof(buf));
            mechlab_set_hint(s, buf);
            log_debug("AP - scout cache hit: loc %lld: %s", (long long)loc, g_scout_cache[i].item_name);
            return true;
        }
    }
    return false;
}

// --- AP Callbacks ---

static void on_item_received(const char *item_name, const char *player_name) {
    if(!g_scene) return;
    log_debug("AP - item received: %s (from %s)", item_name, player_name);
    // Hint shows what's at the NEXT location (scout result); never update it here.
    // Sync stats immediately so progressive items take effect before on_items_done fires.
    game_player *p1 = game_state_get_player(g_scene->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) ap_sync_pilot(pilot);
}

static void on_items_done(void) {
    if(!g_scene) return;
    game_player *p1 = game_state_get_player(g_scene->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) {
        ap_drain_money(pilot);
        ap_sync_pilot(pilot);
    }
    log_debug("AP - items_done: pilot synced");
    mechlab_update(g_scene);
}

static void on_buy_hint(int64_t location_id, const char *item_name, const char *player_name) {
    ap_cache_store(location_id, item_name, player_name);
    if(!g_scene) return;
    if(location_id != g_scout_loc) {
        log_debug("AP - stale scout suppressed (loc %lld expected %lld): %s",
                  (long long)location_id, (long long)g_scout_loc, item_name);
        return;
    }
    log_debug("AP - scout hint: %s (from %s)", item_name, player_name);
    char buf[79];
    ap_format_hint(item_name, player_name, buf, sizeof(buf));
    mechlab_set_hint(g_scene, buf);
}

// --- Public API ---

void ap_mechlab_attach(scene *s) {
    g_scene     = s;
    g_scout_loc = 0;

    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot) {
        ap_drain_money(pilot);
        ap_sync_pilot(pilot);
    }

    Archipelago_SetItemReceivedCallback(on_item_received);
    Archipelago_SetBuyHintCallback(on_buy_hint);
    Archipelago_SetItemsDoneCallback(on_items_done);
}

void ap_mechlab_detach(void) {
    Archipelago_SetItemReceivedCallback(NULL);
    Archipelago_SetBuyHintCallback(NULL);
    Archipelago_SetItemsDoneCallback(NULL);
    g_scene     = NULL;
    g_scout_loc = 0;
}

void ap_do_train(scene *s, int stat) {
    int level = APChecks.pilot_train[stat];
    log_debug("AP - train stat %d: check level %d", stat, level + 1);
    Archipelago_SendCheck(ap_train_location_id(stat, level + 1));
    APChecks.pilot_train[stat]++;
    mechlab_set_hint(s, "");
}

void ap_focus_train(scene *s, int stat) {
    int64_t loc = ap_train_location_id(stat, APChecks.pilot_train[stat] + 1);
    g_scout_loc = loc;
    if(!ap_cache_hit(s, loc))
        Archipelago_ScoutBuyLocation(loc);
}

int32_t ap_buy_price(int32_t vanilla_price, int current_level) {
    if(!ap_mode || APSeedSettings.buy_cost_factor == 100 || current_level <= 1)
        return vanilla_price;
    double factor = APSeedSettings.buy_cost_factor / 100.0;
    return (int32_t)(vanilla_price * pow(factor, current_level - 1));
}

void ap_do_buy_har(scene *s, int har_id, int stat) {
    int buy_level = APChecks.har_buy[har_id][stat];
    log_debug("AP - buy HAR %d stat %d: level %d", har_id, stat, buy_level + 1);
    Archipelago_SendCheck(ap_har_buy_location_id(har_id, stat, buy_level + 1));
    APChecks.har_buy[har_id][stat]++;
    mechlab_set_hint(s, "");
    mechlab_update(s);
}

void ap_focus_buy_har(scene *s, int har_id, int stat) {
    int buy_level = APChecks.har_buy[har_id][stat];
    int64_t loc   = ap_har_buy_location_id(har_id, stat, buy_level + 1);
    g_scout_loc   = loc;
    if(!ap_cache_hit(s, loc))
        Archipelago_ScoutBuyLocation(loc);
}

void ap_on_match_win(int trn_index) {
    log_debug("AP - match win: trn_index %d offset %d", trn_index, APTournament.match_offset);
    Archipelago_SendCheck(ap_match_location_id(APTournament.match_offset + trn_index));
}

void ap_on_tournament_win(void) {
    int tidx = APTournament.tournament_idx;
    log_info("AP - tournament win: idx %d", tidx);
    Archipelago_SendCheck(ap_tournament_win_id(tidx));
    APSave.tournaments_won_mask |= (uint8_t)(1u << tidx);
    bool goal_met = (APSeedSettings.goal_tournament == tidx) ||
                    (APSeedSettings.goal_tournament == AP_TOURNAMENT_ALL &&
                     (APSave.tournaments_won_mask & 0x0Fu) == 0x0Fu);
    if(goal_met) {
        log_info("AP - goal complete");
        Archipelago_GoalComplete();
    }
}
