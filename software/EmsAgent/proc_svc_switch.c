#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../common/const.h"
#include "../common/msg.h"
//#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "../common/iniFile.h"
#include "message_proc.h"

/****************************************************************

	Process Video Sevice Switch command from UDP 

******************************************************************/
#define MAX_UDP_RECV_SIZE           10240
#define DEFAULT_SVC_PORT		2001

/* response error code */
//#define  ERROR_COMM_485		0x15	
//#define  ERROR_COMM_PARSE		0x17	
//#define  ERROR_WRONG_ARGS	0x19	
#define	ERROR_NO_INPUTVIDEO	0x20
//#define  ERROR_I2C				0x21
//#define  ERROR_HDMI			0x22

typedef struct SVCSW_CONFIG{
	unsigned int 	port; 
	unsigned int 	multicast;
	char  		ipaddr[16];  /*used for Multicast IP */
}SVCSW_CONFIG;

extern CARD_INFO	cardData[1 + MAX_CARD_NUM_UTN];
extern int svclog;

int getSvcSwitchCfg(SVCSW_CONFIG *param)
{
	char file[64];
	struct stat conf_stat;
	const char *str;

	snprintf(file, 64, "%s", SSW_PARAM_CONFIG);

	/* check config file exist first */
	if(stat(file, &conf_stat) !=0)
		return -1;
	
	IniFilePtr pIni = iniFileOpen(file);
	if(NULL == pIni)
		return -1;

	if(NULL == param)
	{
		iniFileFree(pIni);
		return -1;
	}

	memset(param, 0, sizeof(SVCSW_CONFIG));
	param->port = iniGetInt(pIni, "SVCSW", "PORT");
	str = iniGetStr(pIni, "SVCSW", "IP");
	if(str !=0)
	{
		strncpy(param->ipaddr, str, 15);
		param->multicast = 1;
	} 
	else
		param->multicast = 0;

	iniFileFree(pIni);
	return 0;
}


int OpenSvcPort()
{
	SVCSW_CONFIG config;
	int sock_fd;
    	struct sockaddr_in my_addr;         
	struct ip_mreq mcast_req;           
	unsigned long inaddr = 0, lTTL = 255;
    	int value;

	if(getSvcSwitchCfg(&config) !=0)
	{
		config.port = DEFAULT_SVC_PORT;
	}
	if(config.port <= 0)
		config.port = DEFAULT_SVC_PORT;

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sock_fd < 0) 
	{
		logger(LOG_ERR, "OpenSvcPort(), socket() - error\n");
		return(-1);
	}

	value = 1;
	ioctl(sock_fd, FIONBIO, &value);
    
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(config.port);
	
 	value = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) < 0) 
	{
		logger(LOG_ERR, "OpenSvcPort(), setsockopt() - 1 error\n");
		return(-1);
	}
    
	value = MAX_UDP_RECV_SIZE;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(value)) == -1)
	{
		logger(LOG_ERR, "OpenSvcPort(), setsockopt() - 2error\n");
		return(-1);
	}

	if(config.multicast) 
	{
		struct sockaddr_in their_addr;      
		struct hostent *he;               
		int fFlag = 0;
		
		if ((inaddr = inet_addr(config.ipaddr)) != INADDR_NONE) // it's dotted-decimal
		{
			memcpy((char *)&their_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
		}
		else
		{
			if ((he = gethostbyname(config.ipaddr)) == NULL)
			{
				logger(LOG_ERR, "OpenSvcPort(), gethostbyname(%s) - error\n", config.ipaddr);
				return(-1);
			}
	    
			memcpy((char *)&their_addr.sin_addr, he->h_addr, he->h_length);
		}
        
		memcpy(&mcast_req.imr_multiaddr, &their_addr.sin_addr, sizeof(struct in_addr));
		mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

		if(setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mcast_req, sizeof(mcast_req)) < 0) 
		{
			logger(LOG_ERR, "OpenSvcPort(), set multicast address (%s) - error\n", config.ipaddr);
			return(-1);
		}

		if(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL)) == -1)
		{
			logger(LOG_ERR, "OpenSvcPort(), set multicast ttl () - error\n");
			return(-1);
		}
        
		if(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag)) == -1)
		{
			logger(LOG_ERR, "OpenSvcPort(), set multicast loop () - error\n");
			return(-1);
		}
	}

	if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) 
	{
		logger(LOG_ERR, "OpenSvcPort(), bind() - error\n");
		return(-1);
	}
	
	if(config.multicast)
		logger(LOG_INFO, "Video Switch Service listen at Multicast:%s port:%d\n", config.ipaddr, config.port);
	else
		logger(LOG_INFO, "Video Switch Service listen at UDP port:%d\n", config.port);
    
	return sock_fd;
	
}

void  send_response(int sock_fd, struct sockaddr * dest, int rsp) 
{
	char send_buff[8];

	send_buff[0] = 0xF6;
	send_buff[1] = 0x88;
	send_buff[2] = (char) rsp;

	sendto(sock_fd, send_buff, 3, MSG_DONTWAIT, dest, sizeof(struct sockaddr)) ;
}

void ProcessSvcSwitchMsg(int sock_fd)
{
    	struct sockaddr_in sender;         
	socklen_t sender_addr_len;
	int rlen,i, start_idx;
	char buffer[MAX_CMDBUFF_SIZE];
	char checksum;
	int cardno, input_video;

	memset((char *)&sender, 0, sizeof(sender));
	sender_addr_len = sizeof(sender);
	rlen = recvfrom(sock_fd, buffer, MAX_CMDBUFF_SIZE, 0, (struct sockaddr *)&sender, &sender_addr_len);
	if(rlen <= 0)
	{
		logger(LOG_ERR, "Video Switch Service UDP receive error\n");
		return;
	}

	if(svclog)
		DumpMsg((unsigned char *)buffer, rlen); 

	/* check message format */
	if(rlen < 7)
	{
		logger(LOG_WARNING, "Switch Service received incomplete msg, len=%d\n", rlen);
		return;
	}

	start_idx = 0;
	while((buffer[start_idx] != 0xF6) && (start_idx < rlen))
		start_idx++;
	
	if((rlen - start_idx) < 7)
	{
		logger(LOG_WARNING, "Switch Service received  msg has no start flag 0xF6 : %d\n", start_idx);
		return;
	}
	
	if(buffer[start_idx + 1] != 0x88)
	{
		logger(LOG_DEBUG, "Skip Switch Service commad: 0x%02x\n", buffer[start_idx + 1]);
		return;
	}

	checksum = buffer[start_idx + 1];
	for(i=2;i<6; i++)
		checksum += buffer[start_idx + i];

	if(buffer[start_idx + 6] != (checksum&0x7f))
	{
		logger(LOG_WARNING, "Switch Service commad checksum error: recv:0x%02x, calc: 0x%02x\n", buffer[start_idx + 6], checksum);
		return;
	}

	input_video = buffer[start_idx + 2]&0x3F;
	input_video <<= 8;
	input_video += buffer[start_idx + 3]&0x7F;
	input_video += buffer[start_idx + 2]&0x80;
	logger(LOG_DEBUG, "Got Switch Service commad, cam:%d\n", input_video);

	cardno = input_video/10 + 1;
#if 0
	if((cardno > MAX_CARD_NUM_UTN) || (cardData[cardno].card_type != CARD_TYPE_MS4V1H))
#else
	if((cardno > MAX_CARD_NUM_UTN)
            ||(        (cardData[cardno].card_type != CARD_TYPE_MS4V1H)
		    && ( cardData[cardno].card_type !=  CARD_TYPE_M4S)
		    && (cardData[cardno].card_type !=  CARD_TYPE_M4H)
                )
            )
#endif
	{
		logger(LOG_WARNING, "Switch Service commad no input video:%d, card-%d, idx-%d, card_type:%d\n", 
                            input_video, cardno, input_video%10, cardData[cardno].card_type);
		send_response(sock_fd, (struct sockaddr *)&sender, ERROR_NO_INPUTVIDEO);
		return;
	}
	
	rlen = Ms4Switch(cardno, input_video%10);
	send_response(sock_fd, (struct sockaddr *)&sender, rlen);

}


#if 1 // 20150625
int OpenMsgSvcPort( )
{
	SVCSW_CONFIG config;
	int sock_fd;
	struct sockaddr_in my_addr;         
	struct ip_mreq mcast_req;           
	unsigned long inaddr = 0, lTTL = 255;
	int value;
    int port = -1;
    
	if(getSvcSwitchCfg(&config) !=0)
	{
		port = DEFAULT_SVC_PORT + 1;
	}
	else
    	port = config.port + 1;
	
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sock_fd < 0) 
	{
		logger(LOG_ERR, "OpenMsgSvcPort(), socket() - error\n");
		return(-1);
	}

	value = 1;
	ioctl(sock_fd, FIONBIO, &value);
    
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(port);
	
 	value = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) < 0) 
	{
		logger(LOG_ERR, "OpenMsgSvcPort(), setsockopt() - 1 error\n");
		return(-1);
	}
    
	value = MAX_UDP_RECV_SIZE;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(value)) == -1)
	{
		logger(LOG_ERR, "OpenSvcPort(), setsockopt() - 2error\n");
		return(-1);
	}



	if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) 
	{
		logger(LOG_ERR, "OpenMsgSvcPort(), bind() - error\n");
		return(-1);
	}
	
    logger(LOG_INFO, "Video Switch Service listen at UDP port:%d\n", port);
    
	return sock_fd;
	
}

#endif
