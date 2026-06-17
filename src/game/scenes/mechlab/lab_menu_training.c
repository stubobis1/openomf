#include <stdio.h>

/* AP */ #include "archipelago/ap_mechlab.h"
/* AP */ #include "archipelago/apstate.h"
#include "game/gui/label.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_training.h"
#include "game/utils/formatting.h"
#include "resources/bk.h"
#include "resources/languages.h"

// I don't care anymore, sorry
static component *label1;
static component *label2;

static void lab_menu_focus_power(component *c, bool focused, void *userdata);
static void lab_menu_focus_agility(component *c, bool focused, void *userdata);
static void lab_menu_focus_endurance(component *c, bool focused, void *userdata);

void lab_menu_training_power(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_POWER] : pilot->power;
    int32_t price = ap_train_price(level);
    pilot->money -= price;
    /* AP */ if(ap_mode) {
        ap_do_train(s, AP_PILOT_POWER);
        ap_update_train_labels(512, level + 1);
    } else {
        pilot->power++;
        lab_menu_focus_power(c, true, userdata);
    }
    mechlab_update(s);
}

void lab_menu_training_check_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int max_level = ap_mode ? APSeedSettings.pilot_stat_max : 23;
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_POWER] : pilot->power;
    if(level >= max_level) {
        component_disable(c, 1);
        return;
    }
    int32_t price = ap_train_price(level);
    component_disable(c, price > pilot->money);
}

void lab_menu_training_agility(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_AGILITY] : pilot->agility;
    int32_t price = ap_train_price(level);
    pilot->money -= price;
    /* AP */ if(ap_mode) {
        ap_do_train(s, AP_PILOT_AGILITY);
        ap_update_train_labels(513, level + 1);
    } else {
        pilot->agility++;
        lab_menu_focus_agility(c, true, userdata);
    }
    mechlab_update(s);
}

void lab_menu_training_check_agility_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int max_level = ap_mode ? APSeedSettings.pilot_stat_max : 23;
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_AGILITY] : pilot->agility;
    if(level >= max_level) {
        component_disable(c, 1);
        return;
    }
    int32_t price = ap_train_price(level);
    component_disable(c, price > pilot->money);
}

void lab_menu_training_endurance(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_ENDURANCE] : pilot->endurance;
    int32_t price = ap_train_price(level);
    pilot->money -= price;
    /* AP */ if(ap_mode) {
        ap_do_train(s, AP_PILOT_ENDURANCE);
        ap_update_train_labels(514, level + 1);
    } else {
        pilot->endurance++;
        lab_menu_focus_endurance(c, true, userdata);
    }
    mechlab_update(s);
}

void lab_menu_training_check_endurance_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    int max_level = ap_mode ? APSeedSettings.pilot_stat_max : 23;
    int level = ap_mode ? APChecks.pilot_train[AP_PILOT_ENDURANCE] : pilot->endurance;
    if(level >= max_level) {
        component_disable(c, 1);
        return;
    }
    int32_t price = ap_train_price(level);
    component_disable(c, price > pilot->money);
}

void lab_menu_training_done(component *c, void *userdata) {
    trnmenu_finish(c->parent);
}

// clang-format off
static const button_details details_list[] = {
    {lab_menu_training_power,     "POWER",   TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2, 0}, false},
    {lab_menu_training_agility,   "AGILITY", TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2, 0}, false},
    {lab_menu_training_endurance, "ENDUR.",  TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2, 0}, false},
    {lab_menu_training_done,      "DONE",    TEXT_ROW_VERTICAL,   TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {1, 0, 0, 0}, false},
};
// clang-format on

static void lab_menu_focus_power(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        int level = ap_mode ? APChecks.pilot_train[AP_PILOT_POWER] : pilot->power;
        /* AP */ if(ap_mode) {
            ap_update_train_labels(512, level);
            ap_focus_train(s, AP_PILOT_POWER);
        } else {
            label_set_text(label1, lang_get(512));
            if(level >= 23) {
                label_set_text(label2, "UNAVAILABLE");
            } else {
                char tmp[32]; char price_str[16];
                score_format(ap_train_price(level), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "$ %sK", price_str);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint_wrapped(s, lang_get(533));
        }
    }
}

static void lab_menu_focus_agility(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        int level = ap_mode ? APChecks.pilot_train[AP_PILOT_AGILITY] : pilot->agility;
        /* AP */ if(ap_mode) {
            ap_update_train_labels(513, level);
            ap_focus_train(s, AP_PILOT_AGILITY);
        } else {
            label_set_text(label1, lang_get(513));
            if(level >= 23) {
                label_set_text(label2, "UNAVAILABLE");
            } else {
                char tmp[32]; char price_str[16];
                score_format(ap_train_price(level), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "$ %sK", price_str);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint_wrapped(s, lang_get(534));
        }
    }
}

static void lab_menu_focus_endurance(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        int level = ap_mode ? APChecks.pilot_train[AP_PILOT_ENDURANCE] : pilot->endurance;
        /* AP */ if(ap_mode) {
            ap_update_train_labels(514, level);
            ap_focus_train(s, AP_PILOT_ENDURANCE);
        } else {
            label_set_text(label1, lang_get(514));
            if(level >= 23) {
                label_set_text(label2, "UNAVAILABLE");
            } else {
                char tmp[32]; char price_str[16];
                score_format(ap_train_price(level), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "$ %sK", price_str);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint_wrapped(s, lang_get(535));
        }
    }
}

void lab_menu_focus_training_done(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        label_set_text(label1, "");
        label_set_text(label2, "");
        mechlab_set_hint_wrapped(s, lang_get(536));
    }
}

static const spritebutton_focus_cb focus_cbs[] = {lab_menu_focus_power, lab_menu_focus_agility,
                                                  lab_menu_focus_endurance, lab_menu_focus_training_done};

component *lab_menu_training_create(scene *s) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 9)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 1);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, false);

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        sprite *button_sprite = animation_get_sprite(main_buttons, i);
        component *button = sprite_button_from_details(&details_list[i], NULL, button_sprite->data, s);
        spritebutton_set_font(button, FONT_SMALL);
        spritebutton_set_text_color(button, TEXT_TRN_BLUE);
        component_set_pos_hints(button, button_sprite->pos.x, button_sprite->pos.y);

        if(i == 0) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_power_price);
        } else if(i == 1) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_agility_price);
        } else if(i == 2) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_endurance_price);
        }
        component_tick(button);
        spritebutton_set_focus_cb(button, focus_cbs[i]);

        trnmenu_attach(menu, button);
    }

    label1 = label_create("");
    label_set_text_color(label1, 0xA5);
    label_set_font(label1, FONT_SMALL);
    component_set_size_hints(label1, 90, 110);
    component_set_pos_hints(label1, 200, 148);
    trnmenu_attach(menu, label1);

    label2 = label_create("");
    label_set_text_color(label2, 0xA7);
    label_set_font(label2, FONT_SMALL);
    component_set_size_hints(label2, 90, 110);
    component_set_pos_hints(label2, 200, 186);
    trnmenu_attach(menu, label2);

    /* AP */ ap_register_train_labels(label1, label2);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
