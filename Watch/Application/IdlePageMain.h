#ifndef METAWATCH_IDLE_PAGE_MAIN_H
#define METAWATCH_IDLE_PAGE_MAIN_H

void InitializeDisplaySeconds(void);
int GetDisplaySeconds(void);
void ToggleSecondsHandler(void);

 unsigned char const* GetSpritePointerForChar(unsigned char CharIn);
 unsigned char const* GetSpritePointerForDigit(unsigned char Digit);

extern const unsigned char pTimeColonR[1][19*1];
extern const unsigned char pTimeColonL[1][19*1];
extern const unsigned char pTimeDigit[10*2][19*2];

#endif /* METAWATCH_IDLE_PAGE_MAIN_H */
