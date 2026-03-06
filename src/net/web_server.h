#pragma once

#include <cstdint>

class Session;

namespace net {

void webServerInit(volatile uint16_t* distancePtr, Session* session);
void webServerHandle();

}  // namespace net
