#include <stddef.h>
#include "IdlePage.h"

static struct IdleInfo Info;

void InitIdlePage(int IdleModeTimerId,
	          tLcdLine *buffer)
{
  Info.buffer = buffer;
  Info.IdleModeTimerId =IdleModeTimerId;
}


void IdlePageStart(struct IdlePage const * Page)
{
	if(Page != NULL && Page->Start != NULL)
	{
		Page->Start(&Info);
	}
}

void IdlePageStop(struct IdlePage const * Page)
{
	if(Page != NULL && Page->Stop != NULL)
	{
		Page->Stop(&Info);
	}
}

void IdlePageHandler(struct IdlePage const * Page)
{
	if(Page != NULL && Page->Handler != NULL)
	{
		Page->Handler(&Info);
	}
}

void IdlePageConfigButtons(struct IdlePage const * Page)
{
	if(Page != NULL && Page->ConfigButtons != NULL)
	{
		Page->ConfigButtons(&Info);
	}
}
