#ifndef METAWATCH_MENU_H
#define METAWATCH_MENU_H

#define NEWMENU

#include "Messages.h"

struct menu;

struct menu_item_msg
{
    const eMessageType msg;
    const unsigned char Options;
    unsigned char const * const pIcon;
};

struct menu_item_menu
{
    struct menu const * const menuptr;
    unsigned char const * const pIcon;
};

typedef unsigned char const *  (* const menu_icon_func)(void);
typedef void ( * const  menu_action_func)(void);
typedef void ( * menu_action_func_item)(int id);

struct menu_item_icon_action
{
    unsigned char const * const pIcon;
    const menu_action_func action;
};

struct menu_item_txt_action
{
    char *text;
    menu_action_func_item action;
    char id;
};


struct menu_item_geticon_action
{
	const menu_icon_func geticon;
	const menu_action_func action;
};

typedef enum {menu_null, menu_msg, menu_menu, menu_action, menu_icon_action, menu_text_action} menu_enum;

#define MENU_FLAG_UPDATE 0x1
#define MENU_ITEM_MENU_PUSH 0x2
#define MENU_ITEM_MENU_NEXT 0x4

struct menu_item {
	menu_enum type;
	unsigned char ButtonPressType;
	unsigned char flags;
	union {
		struct menu_item_msg imsg;
		struct menu_item_menu imenu;
		struct menu_item_icon_action iaction;
		struct menu_item_geticon_action iiconaction;
		struct menu_item_txt_action itext;
	}u;
};

#define MENU_ITEMS 5

struct menu{
	struct menu_item items[MENU_ITEMS];
};

#define MENU_DEF(name) extern const struct menu name;

#define MENU_BLANK() { .type = menu_null },

#define MENU_START(name) const struct menu name = {	.items = {
/*#define MENU_START_WITH_NEXT(name, nextmenu) const struct menu name = {	.next = &nextmenu, .items = {*/

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

#define MENU_TEXT_ACTION(picon, actionfunc, menuflags) \
    { .type = menu_text,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags, \
	.u.itext.text =  picon,\
	.u.itext.action = actionfunc,\
    },

#define MENU_MSG_BUTTON(msgid, option, picon, menuflags) \
    { .type = menu_msg,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags, \
	.u.imsg.msg = msgid,\
	.u.imsg.Options = option,\
	.u.imsg.pIcon =  picon,\
    },

#define MENU_SUB(menu, picon, menuflags) \
    { .type = menu_menu,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags | MENU_ITEM_MENU_PUSH, \
	.u.imenu.menuptr = &menu,\
	.u.imenu.pIcon = picon,\
    },

#define MENU_NEXT(menu, menuflags) \
    { .type = menu_menu,\
	.ButtonPressType = BUTTON_STATE_IMMEDIATE, \
    .flags = menuflags | MENU_ITEM_MENU_NEXT, \
	.u.imenu.menuptr = &menu,\
	.u.imenu.pIcon = pNextIcon,\
    },


#define MENU_END }};


void menu_init(struct menu const *m);
void menu_config_buttons(void);
void DrawMenu();
int menu_button_handler(unsigned char MsgOptions);
void menu_set_app(struct menu const *m);
#endif /* METAWATCH_MENU_H */
