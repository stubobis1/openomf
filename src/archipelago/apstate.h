#pragma once
#include <stdint.h>
#include <stdbool.h>

// HAR stat indices — must match AP world stat order
#define AP_STAT_ARM_POWER    0
#define AP_STAT_LEG_POWER    1
#define AP_STAT_ARM_SPEED    2
#define AP_STAT_LEG_SPEED    3
#define AP_STAT_ARMOR        4
#define AP_STAT_STUN_RESIST  5
#define AP_STAT_COUNT        6

// Pilot stat indices — must match AP world pilot stat order
#define AP_PILOT_POWER      0
#define AP_PILOT_AGILITY    1
#define AP_PILOT_ENDURANCE  2
#define AP_PILOT_STAT_COUNT 3

// Items from AP server — rebuilt from full replay on each connect; do not persist.
typedef struct {
    uint16_t har_unlocked;                     // bitmask: bit i = HAR i unlocked
    uint8_t  har_stats[11][AP_STAT_COUNT];     // [har_id][AP_STAT_*]
    uint8_t  pilot_stats[AP_PILOT_STAT_COUNT]; // [AP_PILOT_*]
    uint8_t  tournament_access_count;          // 0=NAO only; 1=+Katushai; 2=+WAR; 3=all
    uint8_t  extra_har_colors;                 // additional HAR color slots received
} ap_items_t;

// AP money accumulates from item callbacks; drained into pilot->money at mechlab entry.
typedef struct {
    int32_t  pending_money;
} ap_stats_t;

// Seed settings received from slot_data on connect.
typedef struct {
    int  goal_tournament; // 0-3 = specific tournament, 4 = all
    int  starting_har;    // 0-10 HAR index
    int  har_stat_max;    // max upgrade levels per HAR stat
    int  pilot_stat_max;  // max upgrade levels per pilot stat
    bool include_buy;
    int  buy_cost_factor;  // 10-1000; divide by 100 for float multiplier
    int  money_small_value; // base credits for Money - Small (after factor; default AP_MONEY_SMALL_VALUE)
    int  money_large_value; // base credits for Money - Large (after factor; default AP_MONEY_LARGE_VALUE)
} ap_seed_settings_t;

// Save-persistent AP state (written to .APS sidecar file alongside the .CHR save).
typedef struct {
    uint32_t last_applied_item_index; // highest item.index where consumables were applied
    uint8_t  tournaments_won_mask;    // bitmask of which AP tournament indices (0-3) have been won
    int32_t  har_money[11];           // per-HAR money wallets
} ap_save_t;

// Per-stat buy/train check counters — rebuilt from location history on each items_received.
// Tracks how many checks have been *sent* for each stat, which determines the next
// location ID and cost. Separate from APItems which tracks what was *received*.
typedef struct {
    uint8_t pilot_train[AP_PILOT_STAT_COUNT];
    uint8_t har_buy[11][AP_STAT_COUNT];
} ap_checks_t;

// Runtime tournament state — set in mechlab when a tournament is selected.
typedef struct {
    int  tournament_idx;   // 0-3 matching AP world tournament order
    int  match_offset;     // global match index of first match in this tournament
} ap_tournament_state_t;

// Match offsets per tournament (must match Locations.py non-restricted pilot counts)
// NAO=10, Katushai=11, WAR=11, World=28
#define AP_TOURNAMENT_OFFSETS { 0, 10, 21, 32 }

// Globals defined in apconnect.cpp; included by both C and C++ code.
#ifdef __cplusplus
extern "C" {
#endif

extern ap_items_t            APItems;
extern ap_stats_t            APStats;
extern ap_seed_settings_t    APSeedSettings;
extern ap_save_t             APSave;
extern ap_checks_t           APChecks;
extern ap_tournament_state_t APTournament;
extern bool                  ap_mode;

#ifdef __cplusplus
}
#endif
