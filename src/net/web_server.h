#pragma once

#include <cstdint>

namespace net {

void webServerInit(volatile uint16_t* distancePtr);
void webServerHandle();

}  // namespace net
