#ifndef  _REFRESH_CHECK_H
#define  _REFRESH_CHECK_H

#define	CHANGE_FLAG_TREEV		0x01
#define	CHANGE_FLAG_FRAMEV		0x02

#define 	REFRESH_IDLE_TIME			60


void clearRefreshClient();
void setRefreshState_Topo(unsigned char card);
void setRefreshState(unsigned short flag);
int getRefreshState(unsigned long sessionID, unsigned char type, unsigned char cardno);

#endif
