#ifndef  _ALARM_PROC_H
#define  _ALARM_PROC_H

void check_cardAlarms();
unsigned char getServerity(unsigned char alarmType);
void genAlarm(sqlite3 *db, unsigned char alarmType, unsigned char serverity,unsigned char card_no, unsigned char port_no, unsigned char node_id, unsigned char  reason );
void genCardAlarm(sqlite3 *db, unsigned char alarmType, unsigned char serverity, unsigned char card_no, unsigned char port_no);
int UpdateEPOnuList(sqlite3 *db, unsigned char card_no);

#endif
