#include "LinkAlarm.h"

#include "Messages.h"
#include "NvIds.h"
#include "OSAL_Nv.h"
#include "BufferPool.h"
#include "FreeRTOS.h"
#include "Queue.h"
#include "MessageQueues.h"
#include "Icons.h"

static unsigned char nvLinkAlarmEnable;

unsigned char QueryLinkAlarmEnable(void)
{
  return nvLinkAlarmEnable;
}

void ToggleLinkAlarmEnable(void)
{
	  if ( nvLinkAlarmEnable == 1 )
	  {
	    nvLinkAlarmEnable = 0;
	  }
	  else
	  {
	    nvLinkAlarmEnable = 1;
	  }
}

void InitializeLinkAlarmEnable(void)
{
	  nvLinkAlarmEnable = 1;
	  OsalNvItemInit(NVID_LINK_ALARM_ENABLE,
	                 sizeof(nvLinkAlarmEnable),
	                 &nvLinkAlarmEnable);

}

void SaveLinkAlarmEnable(void)
{
	  osal_nv_write(NVID_LINK_ALARM_ENABLE,
		                NV_ZERO_OFFSET,
		                sizeof(nvLinkAlarmEnable),
		                &nvLinkAlarmEnable);
}


/* send a vibration to the wearer */
void GenerateLinkAlarm(void)
{
  tHostMsg* pMsg;

  BPL_AllocMessageBuffer(&pMsg);
  pMsg->Type = SetVibrateMode;

  tSetVibrateModePayload* pMsgData;
  pMsgData = (tSetVibrateModePayload*) pMsg->pPayload;

  pMsgData->Enable = 1;
  pMsgData->OnDurationLsb = 0x00;
  pMsgData->OnDurationMsb = 0x01;
  pMsgData->OffDurationLsb = 0x00;
  pMsgData->OffDurationMsb = 0x01;
  pMsgData->NumberOfCycles = 5;

  RouteMsg(&pMsg);
}

unsigned char const *LinkAlarmIcon(void)
{
	  if ( QueryLinkAlarmEnable() )
	  {
	    return pLinkAlarmOnIcon;
	  }
	  else
	  {
	    return pLinkAlarmOffIcon;
	  }
}

