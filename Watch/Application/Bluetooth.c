#include "Messages.h"
#include "BufferPool.h"
#include "FreeRTOS.h"
#include "Queue.h"
#include "MessageQueues.h"
#include "SerialProfile.h"
#include "Bluetooth.h"


void bluetooth_toggle_bluetooth(void)
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

void bluetooth_toggle_discoverability(void)
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

void bluetooth_toggle_secure_smiple_pairing(void)
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
