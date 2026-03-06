#pragma once

namespace hal {

void i2cBusRecovery();
void i2cInit();
int  i2cScan();  // returns number of devices found

}  // namespace hal
