#include <stdio.h>
#include "Messages.h"
#include "menu.h"
#include "menus.h"
#include "Bluetooth.h"
#include "LinkAlarm.h"
#include "Display.h"
#include "LcdDisplay.h"
#include "Buttons.h"
#include "Icons.h"
#include "hal_lpm.h"
#include "IdlePageMain.h"
#include "MusicMenu.h"
#include "MenuApp.h"

// From LCD Display. Think the lcd should be responsible for registering it's own config functions??

unsigned char const *RstPinIcon(void)
{
	if ( QueryRstPinEnabled() )
	{
	    return  pRstPinIcon;
	}
	return pNmiPinIcon;
}

unsigned char const * SecondsIcon(void)
{
	if ( GetDisplaySeconds() )
	{
	    return pSecondsOnMenuIcon;
	}
	return  pSecondsOffMenuIcon;

}
unsigned char const * TimeFormatIcon(void)
{
	if(GetTimeFormat() == TWELVE_HOUR)
	{
		return hour24;
	}
	return hour12;
}

unsigned char const * DateFormatIcon(void)
{
	if( GetDateFormat() == DAY_FIRST )
	{
		return NormalDateFormat;
	}
	return pStupidDateFormat;
}


/* OLD
MENU_START(menu1)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_discoverability_icon, bluetooth_toggle_discoverability, 0) // no update as the spp callback does the refresh
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_status_icon, bluetooth_toggle_bluetooth, 0) // no update as the spp callback does the refresh
MENU_END

MENU_START(menu2)
MENU_MSG_BUTTON(SoftwareResetMsg, NO_MSG_OPTIONS, pResetButtonIcon, BUTTON_STATE_PRESSED, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(bluetooth_get_secure_smiple_pairing_icon, bluetooth_toggle_secure_smiple_pairing, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_STATIC_ICON_ACTION(pRstPinIcon, ToggleRstPin, MENU_FLAG_UPDATE)
MENU_END


MENU_START(menu3)
MENU_STATIC_ICON_ACTION(pNormalDisplayMenuIcon,ToggleIdleBufferInvert, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(SecondsIcon, ToggleSecondsHandler, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_DYNAMIC_ICON_ACTION(TimeFormatIcon, ToggleTimeFormat, MENU_FLAG_UPDATE)
MENU_END
*/

MENU_START(bluetooth)
MENU_DYNAMIC_ICON_ACTION(BluetoothDiscoverabilityIcon, BluetoothToggleDiscoverability, 0) // no update as the spp callback does the refresh
MENU_DYNAMIC_ICON_ACTION(BluetoothSecureSmiplePairingIcon, BluetoothToggleSecureSmiplePairing, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(BluetoothSniffModeIcon, BluetoothSniffModeToggle, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(BluetoothStatusIcon, BluetoothToggle, 0) // no update as the spp callback does the refresh
MENU_MSG_BUTTON(ListPairedDevicesMsg, NO_MSG_OPTIONS, pBluetoothListIcon,  MENU_FLAG_UPDATE)
MENU_END

MENU_START(clock)
MENU_DYNAMIC_ICON_ACTION(SecondsIcon, ToggleSecondsHandler, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(DateFormatIcon, ToggleDateFormat, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_DYNAMIC_ICON_ACTION(TimeFormatIcon, ToggleTimeFormat, MENU_FLAG_UPDATE)
MENU_BLANK()
MENU_END

MENU_DEF(dev2)

MENU_START(dev)
MENU_STATIC_ICON_ACTION(pNormalDisplayMenuIcon,ToggleIdleBufferInvert, MENU_FLAG_UPDATE)
MENU_MSG_BUTTON(SoftwareResetMsg, NO_MSG_OPTIONS, pResetButtonIcon, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_NEXT(dev2, MENU_FLAG_UPDATE)
MENU_END

MENU_START(dev2)
MENU_STATIC_ICON_ACTION(pNormalDisplayMenuIcon,ToggleIdleBufferInvert, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pRstPinIcon, ToggleRstPin, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon,MenuAppStart,MENU_FLAG_UPDATE)
MENU_NEXT(dev, MENU_FLAG_UPDATE)
MENU_END


MENU_START(TopLevel)
MENU_SUB(bluetooth, pBluetooth, MENU_FLAG_UPDATE)
MENU_SUB(clock, pClockSettings, MENU_FLAG_UPDATE)
MENU_SUB(MusicMenu, pMusicIcon, MENU_FLAG_UPDATE)
MENU_SUB(dev, pDevSettings, MENU_FLAG_UPDATE)
MENU_MSG_BUTTON(WatchStatusMsg,  NO_MSG_OPTIONS, pWatchStatusPageIcon, MENU_FLAG_UPDATE)
MENU_END



void menus_init(void)
{
	menu_init(&TopLevel);
        printf("Testing output\n");
}

