#include "ap_arena.h"

#include "ap_alert.h"
#include "archipelago/apconnect.h"
#include "utils/log.h"

static scene    *g_arena_scene = NULL;
static ap_alert  g_arena_alert;

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

static void on_foreign_item(const char *item_name, const char *player_name) {
    if(!g_arena_scene) return;
    log_debug("AP arena - item received: %s (from %s)", item_name, player_name);
    ap_alert_show_item(&g_arena_alert, item_name, player_name);
}

void ap_arena_attach(scene *s) {
    g_arena_scene = s;
    ap_alert_create(&g_arena_alert);
    ap_alert_set_pos(&g_arena_alert, 160, 30);
    Archipelago_SetForeignItemCallback(on_foreign_item);

    if(g_pending_len > 0) {
        for(int i = 0; i < g_pending_len; i++) {
            ap_alert_show_item(&g_arena_alert, g_pending[i].item, g_pending[i].player);
        }
        log_debug("AP arena - flushed %d queued item(s)", g_pending_len);
        g_pending_len = 0;
    }
}

void ap_arena_detach(void) {
    Archipelago_SetForeignItemCallback(NULL);
    ap_alert_free(&g_arena_alert);
    g_arena_scene = NULL;
}

void ap_arena_tick(void) {
    ap_alert_tick(&g_arena_alert);
}

void ap_arena_render(void) {
    ap_alert_render(&g_arena_alert);
}
