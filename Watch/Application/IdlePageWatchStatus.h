#ifndef METAWATCH_IDLE_PAGE_WATCH_STATUS_H
#define METAWATCH_IDLE_PAGE_WATCH_STATUS_H

void IdlePageWatchStatusInit(void);
void IdlePageWatchStatusExit(void);
void IdlePageWatchStatusConfigButtons(void);
void IdlePageWatchStatusHandler(int IdleModeTimerId);

#endif /* METAWATCH_IDLE_PAGE_WATCH_STATUS_H */
