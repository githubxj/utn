#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include "syscall_wrapper.h"

int Socket( int type ) // 0 is tcp, 1 is udp
{
    	type = (type == 0) ? SOCK_STREAM: SOCK_DGRAM;

    	return socket( PF_INET, type, 0);
}

void SetSockaddr( struct sockaddr_in *addr, const char *ip, const unsigned short port)
{
    	addr->sin_family = PF_INET;
    	addr->sin_addr.s_addr = inet_addr( ip );
    	addr->sin_port = htons( port );
}

int Sendto( int sock, char *buf, int size, struct sockaddr_in  *prm_addr)
{
    	return sendto( sock, buf, size, 0, (struct sockaddr *)prm_addr, sizeof(struct sockaddr_in));
}

int acceptor_open( unsigned short port)
{
    	int sock_listen;
    	int reuse = 1;
        int keepAlive = 1;
        int keepIdle = 30;
        int keepInterval = 5;
        int keepCount = 3;
    	struct sockaddr_in addr;
    	sock_listen = socket(PF_INET, SOCK_STREAM, 0);
    	if( sock_listen < 0)
    	{ 
        	return ERR_SOCKET_CONNECTFAILED;
    	}

    	setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &reuse,sizeof(reuse));
        setsockopt(sock_listen,SOL_SOCKET,SO_KEEPALIVE,&keepAlive,sizeof(keepAlive));
        setsockopt(sock_listen,SOL_TCP,TCP_KEEPIDLE,&keepIdle,sizeof(keepIdle));
        setsockopt(sock_listen,SOL_TCP,TCP_KEEPINTVL, &keepInterval,  sizeof(keepInterval));
        setsockopt(sock_listen,SOL_SOCKET,TCP_KEEPCNT,&keepCount,sizeof(keepCount));

   		memset( &addr, 0 ,sizeof(addr) );   
    	addr.sin_family = PF_INET;
    	addr.sin_port = htons( port);
    	addr.sin_addr.s_addr = htonl(INADDR_ANY);

    	if( bind(sock_listen, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    	{
        	perror( "bind" );
		return ERR_SOCKET_BIND;
    	}

    	listen( sock_listen, 10);
    	return sock_listen;
}

int acceptor_get( int acceptor_handler)
{
    	return accept( acceptor_handler, NULL, NULL);
}

int acceptor_close( int acceptor_handler)
{
    	return close( acceptor_handler );
}

/*
 * 函数：opentcpclient
 * 功能：打开一个tcp连接(client操作)
 * 参数：port 服务端端口号
 *       ip 服务端ip地址
 *       ifname 本地使用的网口，NULL表示使用默认
 * 返回：-1 失败
 *       其它 连接的socket描述符
 */
int opentcpclient(uint16_t port, const char *ip, int blk)
{
  if(NULL == ip)
    return -1;


  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == sockfd)
  {
    perror("opentcpclient - create tcp socket");
    return ERR_SOCKET_CONNECTFAILED;
  }

  int on = blk?0:1;
  ioctl(sockfd, FIONBIO, &on);//非0为允许非阻塞，0为禁止


  int keepAlive = 1;
  int keepIdle = 30;
  int keepInterval = 5;
  int keepCount = 3;

  setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,  &keepAlive,    sizeof(keepAlive));
  setsockopt(sockfd, SOL_SOCKET, TCP_KEEPCNT,   &keepCount,    sizeof(keepCount));
  setsockopt(sockfd, SOL_TCP,    TCP_KEEPIDLE,  &keepIdle,     sizeof(keepIdle));
  setsockopt(sockfd, SOL_TCP,    TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval));

  struct sockaddr_in sa;
  memset(&sa, 0 ,sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port   = htons(port);
  inet_aton(ip, &sa.sin_addr);

  socklen_t slen = sizeof(sa);

  if(-1 == connect(sockfd, (struct sockaddr *)&sa, slen))
  {
    if(blk)
    {
      perror("opentcpclient - connect socket");
      close(sockfd);
      return ERR_SOCKET_CLOSE;
    }
  }

  return sockfd;
}

int connector_open(char *des_ip, unsigned short port)
{

    	int sock;
    	struct sockaddr_in addr;
    	unsigned long non_blocking = 1;
    	unsigned long blocking = 0;
    	struct timeval tv;
    	fd_set writefds;
    	unsigned int len;
    	int error;
        int sndbuffer =10;
    	int keepAlive = 1;
        int keepIdle = 30;
        int keepInterval = 5;
        int keepCount = 3;
        unsigned int len2 = sizeof(sndbuffer);
    	sock = socket( PF_INET, SOCK_STREAM, 0);

    	memset( &addr, 0, sizeof(addr));
    	addr.sin_family = PF_INET;
    	addr.sin_port = htons(port);
    	inet_aton( des_ip, &addr.sin_addr);

//	fcntl(sock, F_SETFL,O_NONBLOCK);
    	ioctl(sock,FIONBIO,&non_blocking);
	
	setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,&keepAlive,sizeof(keepAlive));
        setsockopt(sock,SOL_TCP,TCP_KEEPIDLE,&keepIdle,sizeof(keepIdle));
        setsockopt(sock,SOL_TCP,TCP_KEEPINTVL, &keepInterval,  sizeof(keepInterval));
        setsockopt(sock,SOL_SOCKET,TCP_KEEPCNT,&keepCount,sizeof(keepCount));
    	
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
        	tv.tv_sec = 0; // ..
        	tv.tv_usec = 250000; // ..
        	FD_ZERO(&writefds); 
        	FD_SET(sock, &writefds); 
        	if(select(sock+1,NULL,&writefds,NULL,&tv) != 0)
		{ 
            		if(FD_ISSET(sock,&writefds))
			{
                		len=sizeof(error); 
                		if(getsockopt(sock, SOL_SOCKET, SO_ERROR,  (char *)&error, &len) < 0)
                    			goto error_ret;

                                 setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&sndbuffer,len2);
                                 getsockopt(sock,SOL_SOCKET,SO_SNDBUF,&sndbuffer,&len2);

                                 fprintf(stderr,"sendbuffer: %d\n", sndbuffer); 
                		if(error != 0) 
                    			goto error_ret; 
            		}
            		else
                		goto error_ret; //timeout or error happen 
        	}
        	else 
			goto error_ret;
		ioctl(sock, FIONBIO, &blocking);
	}
	else{
		error_ret: 
		close(sock);
		return ERR_SOCKET_CLOSE; 
	}

	return sock;
	
/*    	ret = connect( sock, (struct sockaddr*)&addr, sizeof(addr));

    	printf("socket is %d\n", sock);

    	if(ret < 0)
    	{
        	close( sock );
        	sock = -1;
    	}

    	return sock;*/
}

int recvmsg_upper(int socket, char* msg, int len)
{
	return recv(socket, msg, len,0);
//	recv(socket, msg, len,MSG_DONTWAIT);
}

int sendmsg_upper(int socket, char* msg, int len)
{
	return send(socket, msg, len, MSG_NOSIGNAL);
}

int connector_close( int connector_handler)
{
    	return close( connector_handler);
}

void * Malloc(size_t size)
{
    	void *buf = malloc( size );
//	printf("Malloc +: 0x%x -- 0x%x\n",(unsigned int)buf, (unsigned int)buf+size );
    	return buf;
}

void   Free( void *ptr)
{
//	printf("Free   -: 0x%x\n",(unsigned int)ptr);
  		if( ptr )
    		free( ptr );
}

int Close( int fd)
{
    	printf( "close: %d\n",fd);
    	return close(fd);
}

