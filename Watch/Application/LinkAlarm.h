#ifndef METAWATCH_LINK_ALARM_H
#define METAWATCH_LINK_ALARM_H

void InitializeLinkAlarmEnable(void);
unsigned char QueryLinkAlarmEnable(void);
void ToggleLinkAlarmEnable(void);
void GenerateLinkAlarm(void);
void SaveLinkAlarmEnable(void);
unsigned char const *LinkAlarmIcon(void);

#endif /* METAWATCH_LINK_ALARM_H */
