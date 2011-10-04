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
#include "LcdBuffer.h"


#include "hal_board_type.h"


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

static void menu_push(struct menu const *m);
void menu_init(struct menu const *m)
{
  menu_push(m);
}

static int istop()
{
	return (pos == 0);
}

struct menu const *  menu_current(void)
{
	return menustack[pos];
}

static void menu_pop()
{
	if(pos > 0)
	{
		--pos;
	}
        menu_config_buttons();
}

static void menu_push(struct menu const *m)
{
	if(pos < (MENU_MAX_DEPTH-1))
	{
		menustack[++pos] = m;
	}
        menu_config_buttons();
}

void menu_next(struct menu const *m)
{
   menustack[pos] = m;
   menu_config_buttons();
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

int menu_handle_button(const struct menu_item *i)
{
  	switch(i->type)
	{
	case menu_msg:
		// Nothing this should be handled by the framework
                // In fact we should never get here!
		break;
	case menu_menu:
		if((i->flags & MENU_ITEM_MENU_PUSH) == MENU_ITEM_MENU_PUSH)
		{
			menu_push(i->u.imenu.menuptr);
		} else if((i->flags & MENU_ITEM_MENU_NEXT) == MENU_ITEM_MENU_NEXT)
		{
			menu_next(i->u.imenu.menuptr);
		}
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
        if((i->flags & MENU_FLAG_UPDATE) == MENU_FLAG_UPDATE)
	{
		return 1;
	}
        return 0;
}

int menu_button_handler(unsigned char MsgOptions)
{
	char refresh = 0;
	if(MsgOptions < MENU_ITEMS)
	{
            refresh = menu_handle_button(&(menu_current()->items[MsgOptions]));
	}
	else if(MsgOptions == MENU_BUTTON_EXIT)
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
	return refresh;
}

unsigned char index2buttonindex[MENU_ITEMS] = {SW_F_INDEX, SW_E_INDEX, SW_D_INDEX, SW_A_INDEX, SW_B_INDEX};
unsigned char index2buttonmsgoption[MENU_ITEMS] = {MENU_BUTTON_F, MENU_BUTTON_E, MENU_BUTTON_D, MENU_BUTTON_A, MENU_BUTTON_B };

void menu_config_buttons(void)
{
	struct menu const *m = menu_current();
	int i;
	for(i = 0; i < MENU_ITEMS; ++i)
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
}

static void DrawCommonMenuIcons(void)
{
	/*
  CopyColumnsIntoMyBuffer(pNextIcon,
                          BUTTON_ICON_B_E_ROW,
                          BUTTON_ICON_SIZE_IN_ROWS,
                          RIGHT_BUTTON_COLUMN,
                          BUTTON_ICON_SIZE_IN_COLUMNS);
                          */

  CopyColumnsIntoMyBuffer(pLedIcon,
                          BUTTON_ICON_C_D_ROW,
                          BUTTON_ICON_SIZE_IN_ROWS,
                          LEFT_BUTTON_COLUMN,
                          BUTTON_ICON_SIZE_IN_COLUMNS);

  CopyColumnsIntoMyBuffer(pExitIcon,
                          BUTTON_ICON_C_D_ROW,
                          BUTTON_ICON_SIZE_IN_ROWS,
                          RIGHT_BUTTON_COLUMN,
                          BUTTON_ICON_SIZE_IN_COLUMNS);
}

void DrawMenu()
{
  const struct menu * const menu = menu_current();
  DrawCommonMenuIcons();
  CopyColumnsIntoMyBuffer(menu_get_icon(&(menu->items[0])),
                          BUTTON_ICON_A_F_ROW,
                          BUTTON_ICON_SIZE_IN_ROWS,
                          LEFT_BUTTON_COLUMN,
                          BUTTON_ICON_SIZE_IN_COLUMNS);

  CopyColumnsIntoMyBuffer(menu_get_icon(&(menu->items[1])),
                            BUTTON_ICON_B_E_ROW,
                            BUTTON_ICON_SIZE_IN_ROWS,
                            LEFT_BUTTON_COLUMN,
                            BUTTON_ICON_SIZE_IN_COLUMNS);

  CopyColumnsIntoMyBuffer(menu_get_icon(&(menu->items[2])),
                           BUTTON_ICON_C_D_ROW,
                           BUTTON_ICON_SIZE_IN_ROWS,
                           LEFT_BUTTON_COLUMN,
                           BUTTON_ICON_SIZE_IN_COLUMNS);


  CopyColumnsIntoMyBuffer(menu_get_icon(&(menu->items[3])),
                           BUTTON_ICON_A_F_ROW,
                           BUTTON_ICON_SIZE_IN_ROWS,
                           RIGHT_BUTTON_COLUMN,
                           BUTTON_ICON_SIZE_IN_COLUMNS);


  CopyColumnsIntoMyBuffer(menu_get_icon(&(menu->items[4])),
                          BUTTON_ICON_B_E_ROW,
                          BUTTON_ICON_SIZE_IN_ROWS,
                          RIGHT_BUTTON_COLUMN,
                          BUTTON_ICON_SIZE_IN_COLUMNS);
}


