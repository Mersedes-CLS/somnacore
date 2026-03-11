#pragma once

#include <cstdint>

namespace net {

void calibPostResult(int position, int weightKg, int distMm, int dmin, int dmax, int jitter);
void calibPushLive(uint16_t distMm, int weightKg);

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
