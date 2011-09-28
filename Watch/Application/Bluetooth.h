#ifndef METWATCH_BLUETOOTH_WRAPPERS_H
#define METWATCH_BLUETOOTH_WRAPPERS_H

void bluetooth_toggle_bluetooth(void);
unsigned char const * bluetooth_get_status_icon();
void bluetooth_toggle_discoverability(void);
unsigned char const * bluetooth_get_discoverability_icon(void);
void bluetooth_toggle_secure_smiple_pairing(void);

#endif /* METWATCH_BLUETOOTH_WRAPPERS_H */
