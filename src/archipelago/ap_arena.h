#ifndef AP_ARENA_H
#define AP_ARENA_H

#include "game/protos/scene.h"

void ap_arena_attach(scene *s);
void ap_arena_detach(void);
void ap_arena_tick(void);
void ap_arena_render(void);

// Queue an item notification for when the next arena fight starts.
void ap_arena_queue_item(const char *item_name, const char *player_name);

#endif // AP_ARENA_H
