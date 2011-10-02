#ifndef METAWATCH_IDLE_PAGE_GAMEOFLIFE_H
#define METAWATCH_IDLE_PAGE_GAMEOFLIFE_H

void IdlePageGameOfLifeInit(void);
void IdlePageGameOfLifeExit(void);
void IdlePageGameOfLifeConfigButtons(void);
int IdlePageGameOfLifeHandler(int IdleModeTimerId,tLcdLine *screen);

#endif /* METAWATCH_IDLE_PAGE_GAMEOFLIFE_H */
