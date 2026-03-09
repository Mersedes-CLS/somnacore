#pragma once

#include <cstdint>

namespace net {

struct CalibCommand {
    bool    hasCommand;
    char    command[16];
    int     position;
};

void calibPushDistance(uint16_t distMm);
CalibCommand calibPollCommand();
void calibPostResult(int position, int weightKg, int distMm, int dmin, int dmax, int jitter);

// Load calibration table from server. Returns number of points loaded.
// table must have at least maxPoints entries.
struct CalibPoint {
    int position;
    int weight_kg;
    int distance_mm;
    bool valid;
};
int calibLoadTable(CalibPoint* table, int maxPoints);

} // namespace net
