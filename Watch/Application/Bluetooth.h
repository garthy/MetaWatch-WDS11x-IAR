#ifndef METWATCH_BLUETOOTH_WRAPPERS_H
#define METWATCH_BLUETOOTH_WRAPPERS_H

#include "SerialProfile.h"

void BluetoothToggle(void);
unsigned char const * BluetoothStatusIcon();
void BluetoothToggleDiscoverability(void);
unsigned char const * BluetoothDiscoverabilityIcon(void);
void BluetoothToggleSecureSmiplePairing(void);
unsigned char const * BluetoothSecureSmiplePairingIcon(void);

#endif /* METWATCH_BLUETOOTH_WRAPPERS_H */
