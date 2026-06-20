#ifndef AP_ALERT_H
#define AP_ALERT_H

#include "utils/list.h"
#include "utils/vec.h"

typedef struct ap_alert_t {
    int x, y;
    list texts;
} ap_alert;

void ap_alert_create(ap_alert *a);
void ap_alert_free(ap_alert *a);
void ap_alert_set_pos(ap_alert *a, int x, int y);
void ap_alert_tick(ap_alert *a);
void ap_alert_render(ap_alert *a);
void ap_alert_add(ap_alert *a, const char *msg, vec2i start);

// Format item_name/player_name and display as floating alert text.
void ap_alert_show_item(ap_alert *a, const char *item_name, const char *player_name);

#endif // AP_ALERT_H
