#ifndef METWATCH_BLUETOOTH_WRAPPERS_H
#define METWATCH_BLUETOOTH_WRAPPERS_H

#include "SerialProfile.h"

void BluetoothToggle(void);
unsigned char const * BluetoothStatusIcon();
void BluetoothToggleDiscoverability(void);
unsigned char const * BluetoothDiscoverabilityIcon(void);
void BluetoothToggleSecureSmiplePairing(void);
unsigned char const * BluetoothSecureSmiplePairingIcon(void);

/* Add sniff mode */

#define BLUETOOTH_SNIFF_ENABLE  0x00
#define BLUETOOTH_SNIFF_DISABLE 0x01
#define BLUETOOTH_SNIFF_DEBUG   0x02

void BluetoothSniffModeInitialize(void);
void BluetoothSniffModeSave(void);
unsigned char const *BluetoothSniffModeIcon(void);
void BluetoothSniffModeToggle(void);

#endif /* METWATCH_BLUETOOTH_WRAPPERS_H */
