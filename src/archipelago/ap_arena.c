#include "ap_arena.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "archipelago/apconnect.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/utils/score.h"
#include "utils/log.h"

static scene *g_arena_scene = NULL;

#define TEXT_HUD_COLOR  0xE7
#define TEXT_HUD_SHADOW 0xF8

#define PENDING_MAX 20
typedef struct { char item[80]; char player[48]; } pending_item_t;
static pending_item_t g_pending[PENDING_MAX];
static int g_pending_len = 0;

void ap_arena_queue_item(const char *item_name, const char *player_name) {
    if(g_pending_len >= PENDING_MAX) return;
    snprintf(g_pending[g_pending_len].item,   sizeof(g_pending[0].item),   "%s", item_name);
    snprintf(g_pending[g_pending_len].player, sizeof(g_pending[0].player), "%s", player_name);
    g_pending_len++;
    log_debug("AP arena - queued item for next fight: %s (from %s)", item_name, player_name);
}

static void format_item_text(const char *item_name, const char *player_name, char *out, size_t out_size) {
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "%s from %s", item_name, player_name);

    // lowercase
    for(int i = 0; tmp[i]; i++) tmp[i] = (char)tolower((unsigned char)tmp[i]);

    // abbreviations: single-pass, first match wins
    static const char *const from[] = {"progressive ", "stun resist", "endurance"};
    static const char *const to[]   = {"prog. ",       "stun res",    "endur."   };
    char buf[128];
    int si = 0, ti = 0;
    while(tmp[ti] && si + 1 < (int)sizeof(buf)) {
        bool matched = false;
        for(int a = 0; a < 3; a++) {
            int flen = (int)strlen(from[a]);
            int tlen = (int)strlen(to[a]);
            if(strncmp(tmp + ti, from[a], flen) == 0 && si + tlen < (int)sizeof(buf)) {
                memcpy(buf + si, to[a], tlen);
                si += tlen; ti += flen;
                matched = true;
                break;
            }
        }
        if(!matched) buf[si++] = tmp[ti++];
    }
    buf[si] = '\0';

    // trim trailing whitespace
    while(si > 0 && buf[si - 1] == ' ') buf[--si] = '\0';

    snprintf(out, out_size, "%s", buf);
}

void ap_chr_score_add(chr_score *score, const char *str, int points, vec2i pos, float position) {
    text *obj = text_create_with_font_and_size(FONT_SMALL, 155, 6);
    text_set_color(obj, TEXT_HUD_COLOR);
    text_set_shadow_color(obj, TEXT_HUD_SHADOW);
    text_set_shadow_style(obj, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    text_set_word_wrap(obj, true);
    text_set_from_c(obj, str);
    text_generate_layout(obj);
    chr_score_add_obj(score, obj, points, pos, position);
}

void ap_show_score_item(chr_score *score, const char *item_name, const char *player_name) {
    char buf[70];
    format_item_text(item_name, player_name, buf, sizeof(buf));
    ap_chr_score_add(score, buf, 0, vec2i_create(160, 110), 1.0f);
}

static void on_foreign_item(const char *item_name, const char *player_name) {
    if(!g_arena_scene) return;
    log_debug("AP arena - item received: %s (from %s)", item_name, player_name);
    chr_score *score = game_player_get_score(game_state_get_player(g_arena_scene->gs, 0));
    ap_show_score_item(score, item_name, player_name);
}

void ap_arena_attach(scene *s) {
    g_arena_scene = s;
    Archipelago_SetForeignItemCallback(on_foreign_item);

    if(g_pending_len > 0) {
        chr_score *score = game_player_get_score(game_state_get_player(s->gs, 0));
        for(int i = 0; i < g_pending_len; i++) {
            ap_show_score_item(score, g_pending[i].item, g_pending[i].player);
        }
        log_debug("AP arena - flushed %d queued item(s) into score text", g_pending_len);
        g_pending_len = 0;
    }
}

void ap_arena_detach(void) {
    Archipelago_SetForeignItemCallback(NULL);
    g_arena_scene = NULL;
}
