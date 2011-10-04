#include "Messages.h"
#include "BufferPool.h"
#include "FreeRTOS.h"
#include "Queue.h"
#include "MessageQueues.h"
#include "SerialProfile.h"
#include "Bluetooth.h"
#include "Icons.h"
#include "NvIds.h"
#include "OSAL_Nv.h"

 unsigned char nvSniffModeOption=0;


 void BluetoothSniffModeSave(void)
 {
	 osal_nv_write(NVID_SNIFF_MODE,
                   NV_ZERO_OFFSET,
                   sizeof(nvSniffModeOption),
                   &nvSniffModeOption);
 }

static void BluetoothSniffModeSetup(void)
{
	switch ( nvSniffModeOption )
	{
	case BLUETOOTH_SNIFF_ENABLE:
		EnableSniff();
		break;

	case BLUETOOTH_SNIFF_DISABLE:
		DisableSniff();
		break;

	case BLUETOOTH_SNIFF_DEBUG:
		SetSniffSlotParameter(MaxInterval, MILLISECONDS_TO_BASEBAND_SLOTS(1200));
		SetSniffSlotParameter(MinInterval, MILLISECONDS_TO_BASEBAND_SLOTS(1100));
		SetSniffSlotParameter(Attempt, 100);
		break;
	}
}

 void BluetoothSniffModeInitialize(void)
 {
	 nvSniffModeOption = 0;
	 OsalNvItemInit(NVID_SNIFF_MODE,
			 sizeof(nvSniffModeOption),
			 &nvSniffModeOption);
	 BluetoothSniffModeSetup();

 }

 void BluetoothSniffModeToggle(void)
 {
	 nvSniffModeOption = (nvSniffModeOption+1)%3;
	 BluetoothSniffModeSetup();
 }

 unsigned char const * BluetoothSniffModeIcon(void)
 {
	 switch ( nvSniffModeOption )
	 {
	 case BLUETOOTH_SNIFF_ENABLE:
		 return pSniffDefaultIcon;
		 break;

	 case BLUETOOTH_SNIFF_DISABLE:
		 return pSniffOffIcon;
		 break;

	 case BLUETOOTH_SNIFF_DEBUG:
		 return pSniffDebugIcon;
	     break;
	 }
	 return pSniffDefaultIcon;
 }

void BluetoothToggle(void)
{
	tHostMsg* pOutgoingMsg;
    if ( QueryConnectionState() != Initializing )
    {
      BPL_AllocMessageBuffer(&pOutgoingMsg);

      if ( QueryBluetoothOn() )
      {
        pOutgoingMsg->Type = TurnRadioOffMsg;
      }
      else
      {
        pOutgoingMsg->Type = TurnRadioOnMsg;
      }

      RouteMsg(&pOutgoingMsg);
      }
}
unsigned char const * BluetoothStatusIcon()
{
	if ( QueryConnectionState() == Initializing )
	{
	    return pBluetoothInitIcon;
	}
	else if ( QueryBluetoothOn() )
	{
	    return pBluetoothOnIcon;
	}
	return pBluetoothOffIcon;
}

void BluetoothToggleDiscoverability(void)
{
	tHostMsg* pOutgoingMsg;
	if ( QueryConnectionState() != Initializing )
	{
		BPL_AllocMessageBuffer(&pOutgoingMsg);
	    pOutgoingMsg->Type = PariringControlMsg;

	    if ( QueryDiscoverable() )
	    {
	    	pOutgoingMsg->Options = PAIRING_CONTROL_OPTION_DISABLE_PAIRING;
	    }
	    else
	    {
	    	pOutgoingMsg->Options = PAIRING_CONTROL_OPTION_ENABLE_PAIRING;
	    }
	    RouteMsg(&pOutgoingMsg);
	}
}

unsigned char const * BluetoothDiscoverabilityIcon(void)
{
	if ( QueryConnectionState() == Initializing )
	{
		return pPairableInitIcon;
	}
	else if ( QueryDiscoverable() )
	{
	return pPairableIcon;
	}
	return pUnpairableIcon;
}

void BluetoothToggleSecureSmiplePairing(void)
{
	tHostMsg* pOutgoingMsg;
    if ( QueryConnectionState() != Initializing )
    {
      BPL_AllocMessageBuffer(&pOutgoingMsg);
      pOutgoingMsg->Type = PariringControlMsg;
      pOutgoingMsg->Options = PAIRING_CONTROL_OPTION_TOGGLE_SSP;
      RouteMsg(&pOutgoingMsg);
    }
}

unsigned char const * BluetoothSecureSmiplePairingIcon(void)
{
	if ( QueryConnectionState() == Initializing )
	{
		return pSspInitIcon;
	}
	else if ( QuerySecureSimplePairingEnabled() )
	{
	return pSspEnabledIcon;
	}
	return pSspDisabledIcon;
}
