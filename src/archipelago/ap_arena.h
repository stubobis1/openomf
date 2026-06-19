#ifndef AP_ARENA_H
#define AP_ARENA_H

#include "game/protos/scene.h"
#include "game/utils/score.h"

// Call on arena_create; registers foreign-item callback to show score-style text.
// Drains any items queued via ap_arena_queue_item since the last fight.
void ap_arena_attach(scene *s);

// Call on arena_free; unregisters callback and clears state.
void ap_arena_detach(void);

// Queue an item notification to be shown as score text when the next arena fight starts.
// Safe to call from any scene (e.g. mechlab on_item_received).
void ap_arena_queue_item(const char *item_name, const char *player_name);

// chr_score_add variant with word_wrap=true — use for AP notification text.
void ap_chr_score_add(chr_score *score, const char *str, int points, vec2i pos, float position);

// Add a formatted item-received score text to score. Applies abbreviations and word-boundary trimming.
void ap_show_score_item(chr_score *score, const char *item_name, const char *player_name);

#endif // AP_ARENA_H
