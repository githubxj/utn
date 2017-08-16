#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>

#include <sys/socket.h>

// ���������
#define ERR_SOCKET_CONNECTFAILED                            -100    // ����ʧ��
#define ERR_SOCKET_DISCONNECTED                              -101    // �쳣�Ͽ�
#define ERR_SOCKET_OVERTIIME                                    -102    // ���ӳ�ʱ
#define ERR_SOCKET_IPADDRESS_NULL                           -103    // ip��ַΪ��
#define ERR_SOCKET_IPADDRESS                                     -104    // ip��ַ����
#define ERR_SOCKET_PORT_INVALID                               -105    // �˿ںŲ���ȷ
#define ERR_SOCKET_IPMASK                                          -106    // ip�������
#define ERR_SOCKET_IPGATEWAY                                   -107    // ip���ش���
#define ERR_SOCKET_UNLOGIN				              -108    // ��δ��½
#define ERR_SOCKET_RCV_DEV_RESPONSE_RET              -109    // �����豸Ӧ�����
#define ERR_SOCKET_SND_DEV                                        -110    // ���豸������Ϣʧ��
#define ERR_SOCKET_TYPE                                              -111   //ip���ʹ���(�������ಥ)
#define ERR_SOCKET_SETOPT                                          -112     //sockopt����ʧ��
#define ERR_SOCKET_BIND                                              -113     //��ʧ��
#define ERR_SOCKET_CLOSE                                            -114      //socket �رմ���


int Socket( int type ); // 0 is tcp, 1 is udp

void SetSockaddr( struct sockaddr_in *addr, const char *ip, const unsigned short port);
int Sendto( int sock, char *buf, int size, struct sockaddr_in *rm_addr);

int acceptor_open( unsigned short port);
int acceptor_get( int acceptor_handler);
int acceptor_close( int acceptor_handler);

int opentcpclient(uint16_t port, const char *ip, int blk);
int connector_open(char *des_ip, unsigned short port);
int connector_close( int connector_handler);

int recvmsg_upper( int socket, char *msg, int len);
int sendmsg_upper( int socket, char *msg, int len);

void * Malloc(size_t size);
void   Free( void *ptr);

int Close( int fd);

#endif
