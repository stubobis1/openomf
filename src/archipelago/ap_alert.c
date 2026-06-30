#include "ap_alert.h"

#include <ctype.h>
#include <string.h>

#include "game/gui/text/text.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/vec.h"

#define AP_ALERT_COLOR    0xE7
#define AP_ALERT_SHADOW   0xF8
#define AP_ALERT_HANG     25
#define AP_ALERT_DIST     50

typedef struct {
    text  *obj;
    float  position;
    vec2i  start;
    int    age;
} alert_text;

static text *make_text(const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, 155, 18);
    text_set_color(t, AP_ALERT_COLOR);
    text_set_shadow_color(t, AP_ALERT_SHADOW);
    text_set_shadow_style(t, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    text_set_word_wrap(t, true);
    text_set_from_c(t, str);
    text_generate_layout(t);
    return t;
}

static vec2i lerp2i(vec2i a, vec2i b, float t) {
    return vec2i_create(a.x + (int)((b.x - a.x) * t), a.y + (int)((b.y - a.y) * t));
}

void ap_alert_create(ap_alert *a) {
    a->x = 0;
    a->y = 0;
    list_create(&a->texts);
}

void ap_alert_free(ap_alert *a) {
    iterator it;
    alert_text *t;
    list_iter_begin(&a->texts, &it);
    foreach(it, t) {
        text_free(&t->obj);
    }
    list_free(&a->texts);
}

void ap_alert_set_pos(ap_alert *a, int x, int y) {
    a->x = x;
    a->y = y;
}

void ap_alert_tick(ap_alert *a) {
    iterator it;
    alert_text *t;
    int lastage = -1;
    list_iter_begin(&a->texts, &it);
    foreach(it, t) {
        if(lastage > 0 && (lastage - t->age) < AP_ALERT_DIST) break;
        if(t->age > AP_ALERT_HANG) t->position -= 0.01f;
        lastage = t->age++;
        if(t->position < 0.0f) {
            text_free(&t->obj);
            list_delete(&a->texts, &it);
        }
    }
}

void ap_alert_render(ap_alert *a) {
    iterator it;
    alert_text *t;
    int lastage = -1;
    list_iter_begin(&a->texts, &it);
    foreach(it, t) {
        if(lastage > 0 && (lastage - t->age) < AP_ALERT_DIST) break;
        vec2i home = vec2i_create(a->x, a->y);
        vec2i pos = lerp2i(home, t->start, t->position);
        pos.x -= text_get_layout_width(t->obj) / 2;
        text_draw(t->obj, pos.x, pos.y);
        lastage = t->age;
    }
}

void ap_alert_add(ap_alert *a, const char *msg, vec2i start) {
    alert_text t;
    t.obj = make_text(msg);
    t.start = start;
    t.position = 1.0f;
    t.age = 0;
    list_append(&a->texts, &t, sizeof(alert_text));
}

static void format_item_text(const char *item_name, const char *player_name, char *out, size_t out_size) {
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "%s from %s", item_name, player_name);

    for(int i = 0; tmp[i]; i++) tmp[i] = (char)tolower((unsigned char)tmp[i]);

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

    while(si > 0 && buf[si - 1] == ' ') buf[--si] = '\0';

    snprintf(out, out_size, "%s", buf);
}

void ap_alert_show_item(ap_alert *a, const char *item_name, const char *player_name) {
    char buf[70];
    format_item_text(item_name, player_name, buf, sizeof(buf));
    ap_alert_add(a, buf, vec2i_create(160, 10));
}
