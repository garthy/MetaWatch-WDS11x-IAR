#include "FreeRTOS.h"
#include "Messages.h"
#include "queue.h"
#include "MessageQueues.h"
#include "BufferPool.h"
#include "Task.h"
#include "Utilities.h"
#include "Buttons.h"
#include "menu.h"
#include "Icons.h"

// Copies from MediaControl.java
#define VOLUME_UP     10
#define	VOLUME_DOWN   11
#define NEXT          15
#define	PREVIOUS      16
#define TOGGLE        20

#define MUSIC_MSG_TYPE 0x42

static void MusicMsg(int musicmsgtype)
{
	tHostMsg* pOutgoingMsg;

    BPL_AllocMessageBuffer(&pOutgoingMsg);
    pOutgoingMsg->pPayload[0] = musicmsgtype;

    UTL_BuildHstMsg(pOutgoingMsg,GeneralPurposePhoneMsg,MUSIC_MSG_TYPE,
                    pOutgoingMsg->pPayload,sizeof(char));

    RouteMsg(&pOutgoingMsg);
}

void MusicToggle(void)
{
	MusicMsg(TOGGLE);
}

void MusicVolumeUp(void)
{
	MusicMsg(VOLUME_UP);
}

void MusicVolumeDown(void)
{
	MusicMsg(VOLUME_DOWN);
}

void MusicNext(void)
{
	MusicMsg(NEXT);
}

void MusicBack(void)
{
	MusicMsg(PREVIOUS);
}


MENU_START(MusicMenu)
MENU_STATIC_ICON_ACTION(pMusicToggle,MusicToggle, 0)
MENU_STATIC_ICON_ACTION(pMusicVolUp,MusicVolumeUp, 0)
MENU_STATIC_ICON_ACTION(pMusicVolDown,MusicVolumeDown, 0)
MENU_STATIC_ICON_ACTION(pMusicNext,MusicNext, 0)
MENU_STATIC_ICON_ACTION(pMusicBack,MusicBack, 0)
MENU_END
