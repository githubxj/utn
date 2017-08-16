#ifndef  _SERIAL_COMM_H
#define  _SERIAL_COMM_H

int OpenDev(const char *Dev);
void set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);
void closeDev(int fd);
int msg485_sendwaitrsp(int fdCom, const char * send_buff, const char * recv_buff , int timeout);
int msg485_receive(int fdCom, const char * recv_buff,  int timeout);
int msg485_receive_1msg(int fdCom, char * recv_buff,  int timeout);
int msg485_send(int fdCom, const char * send_buff, int length);

#endif
