#ifndef _PROCESSMSG_H
#define _PROCESSMSG_H
int ProcessMsg(char * buffer, int size, int fds);
int init_socket (unsigned char channelno);
int encode_socket (char sendbuff[], char recvbuff[], unsigned char channelno);
#endif

