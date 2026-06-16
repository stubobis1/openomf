#include <stdio.h>
#include <stdlib.h>

/* AP */ #include "archipelago/ap_mechlab.h"
/* AP */ #include "archipelago/apstate.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_confirm.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/scenes/mechlab/lab_menu_trade.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/allocator.h"
#include "utils/log.h"

static void ap_preview_har(scene *s, int har_id) {
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    pilot->har_id = har_id;
    if(ap_mode) {
        ap_apply_har_stats(pilot);
        if(har_id >= 0 && har_id < 11)
            pilot->money = APSave.har_money[har_id];
    }
    mechlab_update(s);
}

void lab_menu_trade_done(component *menu, component *submenu) {
    scene *s = trnmenu_get_userdata(submenu);
    log_debug("trade done");
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(p1->pilot != &p1->chr->pilot) {
        omf_free(p1->pilot);
        p1->pilot = &p1->chr->pilot;
        mechlab_update(s);
    }
}

bool confirm_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(!ap_mode) {
        int trade_value = calculate_trade_value(&p1->chr->pilot);
        int har_value = har_price(p1->pilot->har_id);
        p1->chr->pilot.money += trade_value - har_value;
    }
    /* AP */ int old_har = p1->chr->pilot.har_id;
    /* AP */ int new_har = p1->pilot->har_id;
    /* AP */ if(ap_mode && old_har >= 0 && old_har < 11)
    /* AP */     APSave.har_money[old_har] = p1->chr->pilot.money;
    p1->chr->pilot.har_id = p1->pilot->har_id;
    p1->chr->pilot.leg_speed = 0;
    p1->chr->pilot.arm_speed = 0;
    p1->chr->pilot.leg_power = 0;
    p1->chr->pilot.arm_power = 0;
    p1->chr->pilot.armor = 0;
    p1->chr->pilot.stun_resistance = 0;
    /* AP */ if(ap_mode && new_har >= 0 && new_har < 11)
    /* AP */     p1->chr->pilot.money = APSave.har_money[new_har];
    omf_free(p1->pilot);
    p1->pilot = &p1->chr->pilot;
    /* AP */ if(ap_mode) ap_apply_har_stats(p1->pilot);
    mechlab_update(s);
    trnmenu_finish(c->parent);
    return true;
}

bool cancel_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    omf_free(p1->pilot);
    p1->pilot = &p1->chr->pilot;
    mechlab_update(s);
    trnmenu_finish(c->parent);
    return true;
}

void lab_menu_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    char tmp[100] = "";
    if(ap_mode) {
        // In AP mode all unlocked HARs trade for free
        snprintf(tmp, sizeof(tmp), lang_get(520), lang_get(31 + p1->chr->pilot.har_id),
                 lang_get(31 + p1->pilot->har_id));
    } else {
        int trade_value = calculate_trade_value(&p1->chr->pilot);
        int har_value = har_price(p1->pilot->har_id);
        if(trade_value == har_value) {
            snprintf(tmp, sizeof(tmp), lang_get(520), lang_get(31 + p1->chr->pilot.har_id),
                     lang_get(31 + p1->pilot->har_id));
        } else if(trade_value > har_value) {
            char price[15];
            snprintf(price, sizeof(price), "$ %dK", trade_value - har_value);
            snprintf(tmp, sizeof(tmp), lang_get(518), lang_get(31 + p1->chr->pilot.har_id),
                     lang_get(31 + p1->pilot->har_id), price);
        } else if(trade_value + p1->pilot->money > har_value) {
            char price[15];
            snprintf(price, sizeof(price), "$ %dK", har_value - trade_value);
            snprintf(tmp, sizeof(tmp), lang_get(519), lang_get(31 + p1->chr->pilot.har_id), price,
                     lang_get(31 + p1->pilot->har_id));
        } else {
            log_debug("trade: can't afford HAR %d (need %d, have trade=%d money=%d)",
                      p1->pilot->har_id, har_value, trade_value, p1->pilot->money);
            return;
        }
    }

    component *menu = lab_menu_confirm_create(s, confirm_trade, s, cancel_trade, s, tmp);
    trnmenu_set_userdata(menu, s);
    trnmenu_set_submenu_done_cb(menu, lab_menu_trade_done);
    trnmenu_finish(c->parent);
    trnmenu_set_submenu(c->parent->parent, menu);
}

void lab_menu_trade_for_jaguar_focus(component *c, bool focused, void *userdata)   { if(focused) ap_preview_har(userdata, 0);  }
void lab_menu_trade_for_shadow_focus(component *c, bool focused, void *userdata)   { if(focused) ap_preview_har(userdata, 1);  }
void lab_menu_trade_for_thorn_focus(component *c, bool focused, void *userdata)    { if(focused) ap_preview_har(userdata, 2);  }
void lab_menu_trade_for_pyros_focus(component *c, bool focused, void *userdata)    { if(focused) ap_preview_har(userdata, 3);  }
void lab_menu_trade_for_electra_focus(component *c, bool focused, void *userdata)  { if(focused) ap_preview_har(userdata, 4);  }
void lab_menu_trade_for_katana_focus(component *c, bool focused, void *userdata)   { if(focused) ap_preview_har(userdata, 5);  }
void lab_menu_trade_for_shredder_focus(component *c, bool focused, void *userdata) { if(focused) ap_preview_har(userdata, 6);  }
void lab_menu_trade_for_flail_focus(component *c, bool focused, void *userdata)    { if(focused) ap_preview_har(userdata, 7);  }
void lab_menu_trade_for_gargoyle_focus(component *c, bool focused, void *userdata) { if(focused) ap_preview_har(userdata, 8);  }
void lab_menu_trade_for_chronos_focus(component *c, bool focused, void *userdata)  { if(focused) ap_preview_har(userdata, 9);  }
void lab_menu_trade_for_nova_focus(component *c, bool focused, void *userdata)     { if(focused) ap_preview_har(userdata, 10); }

static const button_details details_list[] = {
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
    {lab_menu_trade, NULL, TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, {2, 0, 0, 0}, false},
};

static const spritebutton_focus_cb focus_cbs[] = {
    lab_menu_trade_for_jaguar_focus,   lab_menu_trade_for_shadow_focus,  lab_menu_trade_for_thorn_focus,
    lab_menu_trade_for_pyros_focus,    lab_menu_trade_for_electra_focus, lab_menu_trade_for_katana_focus,
    lab_menu_trade_for_shredder_focus, lab_menu_trade_for_flail_focus,   lab_menu_trade_for_gargoyle_focus,
    lab_menu_trade_for_chronos_focus,  lab_menu_trade_for_nova_focus};

component *lab_menu_trade_create(scene *s) {
    // animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 13)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    game_player *p1 = game_state_get_player(s->gs, 0);

    p1->pilot = omf_calloc(1, sizeof(sd_pilot));
    memcpy(p1->pilot, &p1->chr->pilot, sizeof(sd_pilot));
    p1->pilot->leg_speed = 0;
    p1->pilot->arm_speed = 0;
    p1->pilot->leg_power = 0;
    p1->pilot->arm_power = 0;
    p1->pilot->armor = 0;
    p1->pilot->stun_resistance = 0;
    /* AP */ if(ap_mode) {
    /* AP */     ap_apply_har_stats(p1->pilot);
    /* AP */     int har = p1->pilot->har_id;
    /* AP */     if(har >= 0 && har < 11) p1->pilot->money = APSave.har_money[har];
    /* AP */ }

    int x = 24;
    int y = 148;
    // Initialize menu, and set button sheet
    component *menu = trnmenu_create(NULL, x, y, false);

    // Build display slice: AP handles eligible filtering and cyclic paging.
    // Vanilla still needs an affordability check, so it builds its own eligible list.
    int display[6];
    int display_count;
    if(ap_mode) {
        display_count = ap_trade_page(p1->pilot->har_id, p1->pilot->har_trades, display);
    } else {
        int trade_value = calculate_trade_value(p1->pilot);
        display_count = 0;
        for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
            if(i == p1->pilot->har_id) continue;
            if(!((p1->pilot->har_trades >> i) & 1)) continue;
            if(har_price(i) > trade_value + p1->pilot->money) continue;
            display[display_count++] = i;
        }
    }
    log_debug("trade: %d display HARs (har_trades=0x%04x)", display_count, p1->pilot->har_trades);

    // Add buttons for the chosen HARs
    for(int k = 0; k < display_count; k++) {
        int i = display[k];
        sprite *button_sprite = animation_get_sprite(main_buttons, i);
        component *button = sprite_button_from_details(&details_list[i], NULL, button_sprite->data, s);
        spritebutton_set_font(button, FONT_SMALL);
        spritebutton_set_text_color(button, TEXT_TRN_BLUE);
        component_set_pos_hints(button, x + button_sprite->pos.x, y + button_sprite->pos.y);

        x += button_sprite->data->w;

        spritebutton_set_focus_cb(button, focus_cbs[i]);
        spritebutton_set_always_display(button);

        trnmenu_attach(menu, button);
    }

    trnmenu_set_userdata(menu, s);
    trnmenu_set_submenu_done_cb(menu, lab_menu_trade_done);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
