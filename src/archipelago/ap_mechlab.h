#ifndef AP_MECHLAB_H
#define AP_MECHLAB_H

#include <stdint.h>
#include "game/protos/scene.h"

// Mechlab lifecycle: call attach on mechlab_create, detach on mechlab_free.
void ap_mechlab_attach(scene *s);
void ap_mechlab_detach(void);

// Pilot training buy/focus callbacks (lab_menu_training.c).
void ap_do_train(scene *s, int stat);
void ap_focus_train(scene *s, int stat);

// HAR stat buy/focus callbacks (lab_menu_customize.c).
// ap_buy_price is also used inline by focus handlers for price display.
int32_t ap_buy_price(int32_t vanilla_price, int current_level);
void    ap_do_buy_har(scene *s, int har_id, int stat);
void    ap_focus_buy_har(scene *s, int har_id, int stat);

// Match / tournament check sends (arena.c, newsroom.c).
void ap_on_match_win(int trn_index);
void ap_on_tournament_win(void);

#endif // AP_MECHLAB_H
