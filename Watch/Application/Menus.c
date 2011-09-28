#include <stdio.h>
#include "Messages.h"
#include "menus.h"
#include "Bluetooth.h"
#include "LinkAlarm.h"
#include "Display.h"
#include "Buttons.h"
#include "Icons.h"
#include "hal_lpm.h"




// From LCD Display. Think the lcd should be responsible for registering it's own config functions
void ToggleSecondsHandler(void);
void ToggleIdleBufferInvert(void);
unsigned char const * SecondsIcon(void);
unsigned char const * TimeFormatIcon(void);


MENU_START(menu1)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_discoverability_icon, bluetooth_toggle_discoverability, 0) // no update as the spp callback does the refresh
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, NULL, 0)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_status_icon, bluetooth_toggle_bluetooth, 0) // no update as the spp callback does the refresh
MENU_END

//MENU_STATIC_ICON_ACTION(picon, actionfunc)
//MENU_MSG(msgid, option, picon)

MENU_START(menu2)
MENU_MSG_BUTTON(SoftwareResetMsg, NO_MSG_OPTIONS, pResetButtonIcon, BUTTON_STATE_PRESSED, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_secure_smiple_pairing_icon, bluetooth_toggle_secure_smiple_pairing, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, NULL, 0)
MENU_STATIC_ICON_ACTION(pRstPinIcon, ToggleRstPin, MENU_FLAG_UPDATE)
MENU_END


MENU_START(menu3)
MENU_STATIC_ICON_ACTION(pNormalDisplayMenuIcon,ToggleIdleBufferInvert, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(SecondsIcon, ToggleSecondsHandler, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, NULL, 0)
MENU_DYNAMIC_ICON_ACTION(TimeFormatIcon, ToggleTimeFormat, MENU_FLAG_UPDATE)
MENU_END

void menu_init(void)
{
	menu_push(&menu1);
}

