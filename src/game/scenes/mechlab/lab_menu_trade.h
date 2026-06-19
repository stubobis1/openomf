#ifndef LAB_MENU_TRADE_H
#define LAB_MENU_TRADE_H

#include <stdbool.h>
#include "game/gui/component.h"
#include "game/protos/scene.h"

component *lab_menu_trade_create(scene *s);

// Confirm/cancel callbacks and done handler — used by ap_do_trade in ap_mechlab.c.
bool confirm_trade(component *c, void *userdata);
bool cancel_trade(component *c, void *userdata);
void lab_menu_trade_done(component *menu, component *submenu);

#endif // LAB_MENU_TRADE_H
