#include "archipelago/ap_melee.h"

#include "archipelago/apstate.h"
#include <stdio.h>

void ap_har_info_text(int har_id, char *buf, size_t size) {
    int total = APSeedSettings.har_stat_max * AP_STAT_COUNT;
    int checks = 0, upgrades = 0;
    for(int s = 0; s < AP_STAT_COUNT; s++) {
        checks   += APChecks.har_buy[har_id][s];
        upgrades += APItems.har_stats[har_id][s];
    }
    snprintf(buf, size, "checks:%d/%d upgrades:%d/%d", checks, total, upgrades, total);
}
