#ifndef  _MESSAGE_PROC_H
#define  _MESSAGE_PROC_H

int ProcessMsg(char * buffer, int size, int fds);
int GetCardInfo(CARD_INFO *	card_info);
int GetOSVersion();
int SetCardFE8(unsigned char card_no, CARD_SET_485FE *fe8set);
int  SetFSA600 (unsigned char  card_no, unsigned char node_id, FSA600_SET_485 *data);
int SetOnu100 (unsigned char card_no, unsigned char node_id ,  FSA600_SETRMT *data);
int FSA600resetAndDFT (unsigned char card_no, unsigned char node_id , unsigned char code);
int getAlarm(unsigned char card_no,  GET_ALARMS_RSP *alarm);
int Ms4Switch(int cardno, char input_video);

#endif
