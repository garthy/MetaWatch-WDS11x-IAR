#include "Messages.h"
#include "BufferPool.h"
#include "FreeRTOS.h"
#include "Queue.h"
#include "MessageQueues.h"
#include "SerialProfile.h"
#include "Bluetooth.h"
#include "Icons.h"

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
