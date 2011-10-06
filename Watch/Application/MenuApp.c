#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "Messages.h"
#include "queue.h"
#include "Messages.h"
#include "MessageQueues.h"
#include "BufferPool.h"
#include "Task.h"
#include "Utilities.h"
#include "Buttons.h"
#include "menu.h"
#include "Icons.h"
#include "MenuApp.h"
#include "HuffmanEngText.h"
static char payload[] = {30,235, 174, 189, 39, 11, 202, 231, 70, 7, 205, 248, 189, 2, 151, 226, 245, 187, 2,50, 0};
static char buf[(26*2)+1];
static struct menu MenuApp;

void decode(char *s, int count,  char *target);
void decode(char *s, int count, char *target)
{
  char i;
  char index = 0;
  char val;
  char charcount = 0;
  const struct node *n = &root;
  while(index < count)
  {
    val = s[index++];
    for(i = 0 ;i < 8; ++i)
    {
        if(!(0x80 & (val << i)))
        {
	  n = n->right;
	} 
	else
	{
	  n = n->left;
	}
	if(n->c)
	{
	  //printf("%c",n->c);
	  target[charcount++] = n->c;
	  if(charcount >= count ) break;
          n = &root;
	}
    }
    if(charcount >= count ) break;
  } 
  target[charcount] = 0;
}

static void MenuAppActionMsg(int item)
{
	tHostMsg* pOutgoingMsg;

	BPL_AllocMessageBuffer(&pOutgoingMsg);
	pOutgoingMsg->pPayload[0] = item;

	UTL_BuildHstMsg(pOutgoingMsg,GeneralPurposePhoneMsg,MENU_MSG_TYPE,
			pOutgoingMsg->pPayload,sizeof(char));

	RouteMsg(&pOutgoingMsg);
}

static void MenuAppAction(int id)
{
	MenuAppActionMsg(id);
}

void MenuAppStart(void)//unsigned char *paylxoad)
{
	memset(&MenuApp,0,sizeof(MenuApp));

	/* first item is length
	 * Then next is huffman encoded text
	 */
	unsigned char length = payload[0];
	//printf("length = %i\n", (int)length);
	decode((char*)(payload+1), length, buf);

	int menuindex = 0;
	char *ch = buf;
	while ((ch - buf) < length)
	{
        MenuApp.items[menuindex].type = menu_text_action;
        MenuApp.items[menuindex].flags = MENU_FLAG_UPDATE;
        MenuApp.items[menuindex].ButtonPressType = BUTTON_STATE_IMMEDIATE;
		MenuApp.items[menuindex].u.itext.text = ch;
		MenuApp.items[menuindex].u.itext.action = MenuAppAction;
		MenuApp.items[menuindex].u.itext.id = menuindex;
		menuindex++;
		ch = strchr(ch,' ');
                if(ch==NULL) break;
		*ch = 0;
		//printf("item = %s\n", ch);
		ch++;
	}
	menu_set_app(&MenuApp);
	tHostMsg* pOutgoingMsg;
	BPL_AllocMessageBuffer(&pOutgoingMsg);
	pOutgoingMsg->Type = MenuModeMsg;
	pOutgoingMsg->Options = MENU_MODE_OPTION_APP;
	RouteMsg(&pOutgoingMsg);
}








