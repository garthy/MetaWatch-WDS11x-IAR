#ifndef METAWATCH_MENU_H
#define METAWATCH_MENU_H

#define NEWMENU

#include "Messages.h"



#define MENU_COUNT 4

struct menu;

struct menu_item_msg
{
    const eMessageType msg;
    const unsigned char Options;
    unsigned char const * const pIcon;
};

struct menu_item_menu
{
	/* Maybe pop or replace flag? */
    struct menu const * const menuptr;
    unsigned char const * const pIcon;
};

typedef unsigned char const *  (* const menu_icon_func)(void);
typedef void ( * const  menu_action_func)(void);

struct menu_item_icon_action
{
    unsigned char const * const pIcon;
    const menu_action_func action;
};


struct menu_item_geticon_action
{
	const menu_icon_func geticon;
	const menu_action_func action;
};

typedef enum {menu_msg, menu_menu, menu_action, menu_icon_action} menu_enum;

#define MENU_FLAG_UPDATE 0x1

struct menu_item {
	menu_enum type;
	unsigned char ButtonPressType;
	unsigned char flags;
	union {
		struct menu_item_msg imsg;
		struct menu_item_menu imenu;
		struct menu_item_icon_action iaction;
		struct menu_item_geticon_action iiconaction;
	}u;
};

#define MENU_ITEMS 4

struct menu{
	const struct menu *next;
	const struct menu_item  items[4];
};

#define MENU_DEF(name) extern const struct menu name;

#define MENU_START(name) const struct menu name = {	.next = NULL, .items = {
#define MENU_START_WITH_NEXT(name, nextmenu) const struct menu name = {	.next = &nextmenu, .items = {

#define MENU_DYNAMIC_ICON_ACTION(iconfunc, actionfunc, menuflags) \
    { .type = menu_icon_action,\
    .ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags, \
	.u.iiconaction.geticon = iconfunc,\
	.u.iiconaction.action = actionfunc,\
    },
#define MENU_STATIC_ICON_ACTION(picon, actionfunc, menuflags) \
    { .type = menu_action,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags, \
	.u.iaction.pIcon =  picon,\
	.u.iaction.action = actionfunc,\
    },

#define MENU_MSG_BUTTON(msgid, option, picon, buttonpresstype, menuflags) \
    { .type = menu_msg,\
	.ButtonPressType = buttonpresstype, \
    .flags = menuflags, \
	.u.imsg.msg = msgid,\
	.u.imsg.Options = option,\
	.u.imsg.pIcon =  picon,\
    },

#define MENU_SUB(menu, picon, menuflags) \
    { .type = menu_menu,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags, \
	.u.imenu.menuptr = &menu,\
	.u.imenu.pIcon = picon,\
    },


#define MENU_END }};


struct menu const * menu_current(void);
void menu_push(struct menu const *m);
void menu_pop();
int menu_button_handler(unsigned char MsgOptions);
void menu_config_buttons(void);
unsigned char const * menu_get_icon(struct menu_item const * item);

#endif /* METAWATCH_MENU_H */
