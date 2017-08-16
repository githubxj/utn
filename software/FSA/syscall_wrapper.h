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

// 网络类错误
#define ERR_SOCKET_CONNECTFAILED                            -100    // 连接失败
#define ERR_SOCKET_DISCONNECTED                              -101    // 异常断开
#define ERR_SOCKET_OVERTIIME                                    -102    // 连接超时
#define ERR_SOCKET_IPADDRESS_NULL                           -103    // ip地址为空
#define ERR_SOCKET_IPADDRESS                                     -104    // ip地址错误
#define ERR_SOCKET_PORT_INVALID                               -105    // 端口号不正确
#define ERR_SOCKET_IPMASK                                          -106    // ip掩码错误
#define ERR_SOCKET_IPGATEWAY                                   -107    // ip网关错误
#define ERR_SOCKET_UNLOGIN				              -108    // 尚未登陆
#define ERR_SOCKET_RCV_DEV_RESPONSE_RET              -109    // 接受设备应答错误
#define ERR_SOCKET_SND_DEV                                        -110    // 给设备发送信息失败
#define ERR_SOCKET_TYPE                                              -111   //ip类型错误(单播，多播)
#define ERR_SOCKET_SETOPT                                          -112     //sockopt设置失败
#define ERR_SOCKET_BIND                                              -113     //绑定失败
#define ERR_SOCKET_CLOSE                                            -114      //socket 关闭错误


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
