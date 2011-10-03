#include <string.h>
#include "hal_lcd.h"
#include "Messages.h" //IDLE_MODE + msgs
#include "Buttons.h"  // BUTTON_STATE
#include "hal_board_type.h"// SW_
#include "LcdDisplay.h"
#include "OneSecondTimers.h"


#define QRTIMER_TIMER   1
#define QRBUTTON_LEFT  2
#define QRBUTTON_RIGHT 3
#define QRBUTTON_EXIT  4
#define QRBUTTON_FIRE  5

const unsigned char pInvader[8*1] ={
0x3 ,
0x7 ,
0x1f ,
0x7f ,
0x7f ,
0x1f ,
0x7 ,
0x3 ,
};

const unsigned char pAlien[8*1] ={
0x6 ,
0x39 ,
0x6d ,
0xfc ,
0xfc ,
0x6d ,
0x39 ,
0x6 ,
};

static signed char timerid = -1;
static char ypos=50;

int IdlePageQrCodeInit()
{
	StopAllDisplayTimers();
	if(timerid == -1)
	{
		timerid = AllocateOneSecondTimer();
		if (timerid < 0)
		{
			// How do we report errors
		}
		SetupOneSecondTimer(timerid, 1, REPEAT_FOREVER, BarCode, QRTIMER_TIMER);
		StartOneSecondTimer(timerid);
	}
	FillMyBuffer(STARTING_ROW, NUM_LCD_ROWS, 0x00);


	return 0;
}

int IdlePageQrCodeExit(void)
{
	StopOneSecondTimer(timerid);
	DeallocateOneSecondTimer(timerid);
	timerid = -1;

	tHostMsg* pOutgoingMsg;
    /* go back to the normal idle screen */
    BPL_AllocMessageBuffer(&pOutgoingMsg);
    pOutgoingMsg->Type = IdleUpdate;
    RouteMsg(&pOutgoingMsg);
    return 0;
}

void DrawScreen()
{
	FillMyBuffer(STARTING_ROW, NUM_LCD_ROWS, 0x00);
	CopyColumnsIntoMyBuffer(pInvader,
				                ypos,
		                        8,
		                        0,
								1);
	int i;
	int col = 0;
	for(col = 0; col < 4; ++col)
	{
		for(i = 0; i < 8; ++i)
		{
			CopyColumnsIntoMyBuffer(pAlien,
				                	16+(i*8),
				                	8,
				                	4+col,
				                	1);
		}
	}
}

int IdlePageQrCodeButtonHandler(unsigned char MsgOptions)
{
	switch(MsgOptions)
	{
	case QRTIMER_TIMER:
		// update the world
		DrawScreen();
		break;
	case QRBUTTON_FIRE:
		// Fire a shot
		break;
	case QRBUTTON_LEFT:
		// move us
		++ypos;
		break;
	case QRBUTTON_RIGHT:
		--ypos;
		break;
	case QRBUTTON_EXIT:
		IdlePageQrCodeExit();
		break;
	}
    return 0;
}

void IdlePageQrCodeConfigButtons(void)
{
    EnableButtonAction(IDLE_MODE,
                       SW_F_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       QRBUTTON_EXIT);

    EnableButtonAction(IDLE_MODE,
                       SW_E_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       QRBUTTON_EXIT);

    /* led is already assigned */

    EnableButtonAction(IDLE_MODE,
                       SW_C_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       QRBUTTON_LEFT);

    EnableButtonAction(IDLE_MODE,
    				   SW_B_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       QRBUTTON_FIRE);

    EnableButtonAction(IDLE_MODE,
                       SW_A_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       BarCode,
                       QRBUTTON_RIGHT);
}

