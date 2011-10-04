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

const unsigned char pBluetooth[32*6] ={
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x20 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x60 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x1 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x3 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x60 , 0x7 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x63 , 0xe , 0x0 , 0x0 , 0x0 ,
0xf , 0x67 , 0x7 , 0x0 , 0x0 , 0x0 ,
0xf , 0xee , 0x3 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xfc , 0x1 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xf8 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x70 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xf8 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xfc , 0x1 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xee , 0x3 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x67 , 0x7 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x63 , 0xe , 0x0 , 0x0 , 0x0 ,
0x0 , 0x60 , 0x7 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x3 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x1 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xe0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x60 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x20 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};

const unsigned char pClockSettings[32*6] ={
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x3e , 0x3 , 0x0 , 0x6 , 0x0 ,
0x0 , 0x7f , 0x3 , 0x0 , 0x6 , 0x0 ,
0x0 , 0x63 , 0x3 , 0x0 , 0x6 , 0x0 ,
0xf , 0x3 , 0xf3 , 0x78 , 0x66 , 0x0 ,
0xf , 0x3 , 0xfb , 0xfd , 0x76 , 0x0 ,
0x0 , 0x3 , 0x9b , 0xcd , 0x3e , 0x0 ,
0x0 , 0x3 , 0x9b , 0xd , 0x1e , 0x0 ,
0x0 , 0x3 , 0x9b , 0xd , 0x1e , 0x0 ,
0x0 , 0x63 , 0x9b , 0xcd , 0x3e , 0x0 ,
0x0 , 0x7f , 0xfb , 0xfd , 0x76 , 0x0 ,
0x0 , 0x3e , 0xf3 , 0x78 , 0x66 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};


const unsigned char pDevSettings[32*6] ={
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x3f , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x7f , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x63 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x63 , 0x9e , 0x31 , 0x0 , 0x0 ,
0x0 , 0x63 , 0xbf , 0x31 , 0x0 , 0xf0 ,
0x0 , 0x63 , 0x33 , 0x1b , 0x0 , 0xf0 ,
0x0 , 0x63 , 0x3f , 0x1b , 0x0 , 0x0 ,
0x0 , 0x63 , 0x3f , 0xe , 0x0 , 0x0 ,
0x0 , 0x63 , 0x3 , 0xe , 0x0 , 0x0 ,
0x0 , 0x7f , 0x3f , 0x4 , 0x0 , 0x0 ,
0x0 , 0x3f , 0x1e , 0x4 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};

const unsigned char pStupidDateFormat[32*6] ={
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x10 , 0x40 , 0x80 , 0x1f , 0x0 ,
0x0 , 0x30 , 0x60 , 0xb0 , 0x3f , 0x0 ,
0x0 , 0x70 , 0x70 , 0xb0 , 0x31 , 0x0 ,
0x0 , 0xf0 , 0x78 , 0x98 , 0x31 , 0xf0 ,
0x0 , 0xf0 , 0x7d , 0x98 , 0x31 , 0xf0 ,
0x0 , 0xb0 , 0x6f , 0x8c , 0x31 , 0x0 ,
0x0 , 0x30 , 0x67 , 0x8c , 0x31 , 0x0 ,
0x0 , 0x30 , 0x62 , 0x86 , 0x31 , 0x0 ,
0x0 , 0x30 , 0x60 , 0x86 , 0x31 , 0x0 ,
0x0 , 0x30 , 0x60 , 0x83 , 0x3f , 0x0 ,
0x0 , 0x30 , 0x60 , 0x83 , 0x1f , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};

const unsigned char NormalDateFormat[32*6] ={
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0xc0 , 0xf , 0x20 , 0x80 , 0x0 ,
0x0 , 0xc0 , 0x1f , 0x6c , 0xc0 , 0x0 ,
0x0 , 0xc0 , 0x18 , 0xec , 0xe0 , 0x0 ,
0x0 , 0xc0 , 0x18 , 0xe6 , 0xf1 , 0x0 ,
0x0 , 0xc0 , 0x18 , 0xe6 , 0xfb , 0x0 ,
0x0 , 0xc0 , 0x18 , 0x63 , 0xdf , 0xf0 ,
0x0 , 0xc0 , 0x18 , 0x63 , 0xce , 0xf0 ,
0x0 , 0xc0 , 0x98 , 0x61 , 0xc4 , 0x0 ,
0x0 , 0xc0 , 0x98 , 0x61 , 0xc0 , 0x0 ,
0x0 , 0xc0 , 0xdf , 0x60 , 0xc0 , 0x0 ,
0x0 , 0xc0 , 0xcf , 0x60 , 0xc0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};



// From LCD Display. Think the lcd should be responsible for registering it's own config functions

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
MENU_MSG_BUTTON(ListPairedDevicesMsg, NO_MSG_OPTIONS, pBluetoothListIcon, BUTTON_STATE_IMMEDIATE, MENU_FLAG_UPDATE)
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
MENU_MSG_BUTTON(SoftwareResetMsg, NO_MSG_OPTIONS, pResetButtonIcon, BUTTON_STATE_PRESSED, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_NEXT(dev2, MENU_FLAG_UPDATE)
MENU_END

MENU_START(dev2)
MENU_STATIC_ICON_ACTION(pNormalDisplayMenuIcon,ToggleIdleBufferInvert, MENU_FLAG_UPDATE)
MENU_DYNAMIC_ICON_ACTION(LinkAlarmIcon, ToggleLinkAlarmEnable, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_STATIC_ICON_ACTION(pRstPinIcon, ToggleRstPin, MENU_FLAG_UPDATE)
MENU_NEXT(dev, MENU_FLAG_UPDATE)
MENU_END


MENU_START(TopLevel)
MENU_SUB(bluetooth, pBluetooth, MENU_FLAG_UPDATE)
MENU_SUB(clock, pClockSettings, MENU_FLAG_UPDATE)
MENU_STATIC_ICON_ACTION(pLedIcon, 0, 0)
MENU_SUB(dev, pDevSettings, MENU_FLAG_UPDATE)
MENU_MSG_BUTTON(WatchStatusMsg,  NO_MSG_OPTIONS, pWatchStatusPageIcon, BUTTON_STATE_IMMEDIATE, MENU_FLAG_UPDATE)
MENU_END



void menus_init(void)
{
	menu_init(&TopLevel);
        printf("Testing output\n");
}

