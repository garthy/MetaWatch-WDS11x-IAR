#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Messages.h"
#include "BufferPool.h"
#include "Buttons.h"
#include "hal_lpm.h"
#include "MessageQueues.h"
#include "Bluetooth.h"
#include "LinkAlarm.h"
#include "Display.h"
#include "Icons.h"
#include "Menu.h"
//#include "Menus.h"

#include "hal_board_type.h"
//#include "hal_rtc.h"
//#include "hal_battery.h"
//#include "hal_lcd.h"

/* Also used as indexs into menu struct */
#define MENU_BUTTON_F 0x0
#define MENU_BUTTON_E 0x1
#define MENU_BUTTON_D 0x2
#define MENU_BUTTON_A 0x3

/****************************/
#define MENU_BUTTON_B 0x4
#define MENU_BUTTON_NEXT MENU_BUTTON_B
#define MENU_BUTTON_C 0x5
#define MENU_BUTTON_EXIT MENU_BUTTON_C
/****************************/


#define MENU_MAX_DEPTH 5

struct menu const *menustack[MENU_MAX_DEPTH] = {0};
signed char pos = -1;

int istop()
{
	return (pos == 0);
}

void menu_pop()
{
	if(pos > 0)
	{
		--pos;
	}
}

void menu_push(struct menu const *m)
{
	if(pos < (MENU_MAX_DEPTH-1))
	{
		menustack[++pos] = m;
	}
}

void menu_next(struct menu const *m)
{
   menustack[pos] = m;
}

struct menu const *  menu_current(void)
{
	return menustack[pos];
}

unsigned char const * menu_get_icon(struct menu_item const * item)
{
  if(item->type == menu_menu)
  {
	  return item->u.imenu.pIcon;
  } else if(item->type == menu_msg)
  {
	  return item->u.imsg.pIcon;
  } else if(item->type ==  menu_action)
  {
	  return item->u.iaction.pIcon;
  } else if(item->type == menu_icon_action)
  {
	  return item->u.iiconaction.geticon();
  }
  return 0;
}

int menu_button_handler(unsigned char MsgOptions)
{
	char refresh = 0;
	const struct menu_item *i = &(menu_current()->items[MsgOptions]);
	if(MsgOptions < 4)
	{
		switch(i->type)
		{
		case menu_msg:
			// Nothing this should be handled by the framework
			break;
		case menu_menu:
			menu_push(i->u.imenu.menuptr);
			break;
		case menu_action:
			if(i->u.iaction.action)
			{
				i->u.iaction.action();
			}
			break;
		case menu_icon_action:
			if(i->u.iiconaction.action)
			{
				i->u.iiconaction.action();
			}
			break;
		default:
			break;
		}
	}
	else if(MsgOptions == MENU_BUTTON_NEXT)
	{
		struct menu const *m = menu_current();
		if(m->next)
		{
			menu_next(m->next);
            refresh = 1;
		}
	} else if(MsgOptions == MENU_BUTTON_EXIT)
	{
		if(istop())
		{
			tHostMsg* pOutgoingMsg;
			/* Only save stuff if it's been changed */
			/* save all of the non-volatile items */
			// FIXME SAVE STUFF!!

			//BPL_AllocMessageBuffer(&pOutgoingMsg);
			//pOutgoingMsg->Type = PariringControlMsg;
			//pOutgoingMsg->Options = PAIRING_CONTROL_OPTION_SAVE_SPP;
			//RouteMsg(&pOutgoingMsg);
			//SaveLinkAlarmEnable();
			//SaveRstNmiConfiguration();
			//SaveIdleBufferInvert();
			//SaveDisplaySeconds();
			//SaveTimeFormat();

			/* go back to the normal idle screen */
			BPL_AllocMessageBuffer(&pOutgoingMsg);
			pOutgoingMsg->Type = IdleUpdate;
			RouteMsg(&pOutgoingMsg);
		}
		else
		{
			menu_pop();
			refresh = 1;
		}
	}
	if((i->flags & MENU_FLAG_UPDATE) == MENU_FLAG_UPDATE)
	{
		refresh = 1;
	}
	return refresh;
}

unsigned char index2buttonindex[MENU_COUNT] = {SW_F_INDEX, SW_E_INDEX, SW_D_INDEX, SW_A_INDEX};
unsigned char index2buttonmsgoption[MENU_COUNT] = {MENU_BUTTON_F, MENU_BUTTON_E, MENU_BUTTON_D, MENU_BUTTON_A };

void menu_config_buttons(void)
{
	struct menu const *m = menu_current();
	int i;
	for(i = 0; i < MENU_COUNT; ++i)
	{
		 EnableButtonAction(IDLE_MODE,
				 index2buttonindex[i],
				 m->items[i].ButtonPressType,
				 m->items[i].type == menu_msg ? m->items[i].u.imsg.msg : MenuButtonMsg,
				 m->items[i].type == menu_msg ? m->items[i].u.imsg.Options : index2buttonmsgoption[i]);
	}
    EnableButtonAction(IDLE_MODE,
                       SW_C_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       MenuButtonMsg,
                       MENU_BUTTON_EXIT);

    EnableButtonAction(IDLE_MODE,
                       SW_B_INDEX,
                       BUTTON_STATE_IMMEDIATE,
                       MenuButtonMsg,
                       MENU_BUTTON_NEXT);
}
