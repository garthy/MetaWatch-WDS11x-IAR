#ifndef METAWATCH_IDLE_PAGE_H
#define METAWATCH_IDLE_PAGE_H

#include "hal_lcd.h"

struct IdleInfo
{
	int IdleModeTimerId;
	tLcdLine *buffer;
};

struct IdlePage
{
	struct IdleInfo *Info;
	void (*Start)(struct IdleInfo *Info);
	void (*Stop)(struct IdleInfo *Info);
	int (*Handler)(struct IdleInfo *Info);
	void (*ConfigButtons)(struct IdleInfo *Info);
};

void IdlePageStart(struct IdlePage const * Page);
void IdlePageStop(struct IdlePage const * Page);
void IdlePageHandler(struct IdlePage const * Page);
void IdlePageConfigButtons(struct IdlePage const * Page);

#endif /* METAWATCH_IDLE_PAGE_H*/
