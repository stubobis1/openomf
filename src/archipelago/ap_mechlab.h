#ifndef AP_MECHLAB_H
#define AP_MECHLAB_H

#include <stdint.h>
#include "game/protos/scene.h"
#include "utils/vector.h"

typedef struct component component; // forward-declare; avoids pulling in gui headers

// Mechlab lifecycle: call attach on mechlab_create, detach on mechlab_free.
void ap_mechlab_attach(scene *s);
void ap_mechlab_detach(void);

// --- Training (lab_menu_training.c) ---

// Price table lookup — use instead of a local prices[] in training UI.
int32_t ap_train_price(int level);

// Register the two label components from lab_menu_training_create.
// Cleared automatically on ap_mechlab_detach.
void ap_register_train_labels(component *stat_label, component *price_label);

// Update training labels after a buy or on focus.
// next_level: the level now being offered (current buy_level for focus, buy_level+1 after buy).
void ap_update_train_labels(int stat_lang_id, int next_level);

// Send a training check and pre-scout the following location.
void ap_do_train(scene *s, int stat);
// Scout the current-level training location for the hint text.
void ap_focus_train(scene *s, int stat);

// --- HAR upgrades (lab_menu_customize.c) ---

// ap_buy_price is also used inline by focus handlers for price display.
int32_t ap_buy_price(int32_t vanilla_price, int current_level);

// Register the two label components from lab_menu_customize_create.
// Cleared automatically on ap_mechlab_detach.
void ap_register_buy_labels(component *header_label, component *details_label);

// Update HAR buy labels after a buy or on focus.
// next_buy_level: the level now being offered (APChecks level for focus, APChecks+1 after buy).
void ap_update_buy_har_labels(sd_pilot *pilot, int stat, int next_buy_level);

void ap_do_buy_har(scene *s, int har_id, int stat);
void ap_focus_buy_har(scene *s, int har_id, int stat);

// --- Other ---

// Apply AP progressive HAR/pilot stats to a pilot (e.g. after a HAR trade).
void ap_apply_har_stats(sd_pilot *pilot);

// --- Trade menu (lab_menu_trade.c) ---

// Build the display slice for the trade menu: eligible HARs sorted by ID,
// excluding current_har_id, showing up to 6 at a time and cycling on each call.
// display_out must hold at least 6 ints. Returns the number of HARs written.
int ap_trade_page(int current_har_id, uint16_t har_trades, int *display_out);

// Match / tournament check sends (arena.c, newsroom.c).
void ap_on_match_win(int trn_index);
void ap_on_tournament_win(void);

// Filter trnlist to only include tournaments accessible by the player's
// Progressive Tournament Access count. Call after trnlist_init in AP mode.
void ap_filter_trnlist(vector *trnlist);

#endif // AP_MECHLAB_H
