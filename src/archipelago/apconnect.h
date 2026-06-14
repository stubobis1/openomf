#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    APCONN_NOT_CONNECTED = 0,
    APCONN_CONNECTING,
    APCONN_READY,
    APCONN_FATAL_ERROR,
} ap_connection_status_t;

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
void Archipelago_Connect(const char *uri, const char *slot, const char *password);
void Archipelago_Poll(void);
void Archipelago_Disconnect(void);
ap_connection_status_t Archipelago_ConnectionStatus(void);

// Send a location check to the server.
void Archipelago_SendCheck(int64_t location_id);

// Signal goal completion.
void Archipelago_GoalComplete(void);

// Register a callback invoked when any item is received.
// item_name and player_name are transient — copy if needed beyond the call.
void Archipelago_SetItemReceivedCallback(void (*cb)(const char *item_name, const char *player_name));

// Scout (and hint) a buy location. Response arrives via the buy hint callback.
void Archipelago_ScoutBuyLocation(int64_t location_id);

// Register a callback invoked when scout info arrives for a buy location.
// location_id, item_name, and player_name are transient — copy if needed.
void Archipelago_SetBuyHintCallback(void (*cb)(int64_t location_id, const char *item_name, const char *player_name));

// Register a callback invoked once after each on_items_received batch completes.
// Fires after all per-item callbacks, so APItems and APChecks are fully rebuilt.
void Archipelago_SetItemsDoneCallback(void (*cb)(void));

#ifdef __cplusplus
}
#endif
