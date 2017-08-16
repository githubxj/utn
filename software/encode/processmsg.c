#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/ioctl.h>

#include "../common/msg.h"
#include "../common/const.h"
#include "../common/alarm_type.h"
#include "../common/logger.h"

#define	VERSION_HW0 	'0'
#define	VERSION_HW1 	'1'
#define	VERSION_FW0 	'0'
#define	VERSION_FW1 	'1'
#define	VERSION_SW0 	'2'
#define	VERSION_SW1 	'1'
#define   SERVER_ADDR   "127.0.0.1"
#define   SERVER_PORT  6000
#define   SERVER_SLAVE_PORT  6002

char 	OSversion[MAX_DISPLAYNAME_LEN];
char 	CardName[MAX_DISPLAYNAME_LEN];
unsigned char 	adminState;
extern unsigned char card_no;
extern int ch_sock_fd, cb_sock_fd;

int GetOSVersion()
{
	int filefd;
	char buffer[256];
	char *ptr;
	int i;

	filefd = open("/proc/version", O_RDONLY);
	if(filefd == -1) 
	{
		strcpy(OSversion, "Unknow");
		return 0;
	}

	memset(buffer, 0, 256);
	read(filefd, buffer, 256);
	buffer[255] = '\0';

	ptr = buffer;
	while(*ptr != '\0' && (*ptr < '0'  || *ptr > '9'))
		ptr++;

	for(i=0; i<15; i++)
	{
		OSversion[i] = ptr[i];
		if(ptr[i] == '\0' || ptr[i] == ' ')
			break;
	}

	OSversion[i] = '\0';
	
	close(filefd);

	return 0;

}


int init_socket (unsigned char channelno)
{
	
	int connect_fd;
	struct sockaddr_in connect_addr;
	int recv_len, ret;
	char login_buff_send[256];
	char login_buff_recv[256];
	char ret_buff[4];
	char ack_buff[4];
	int ack_value;
	memset (ret_buff, 0 , sizeof (ret_buff));
	memset (ack_buff, 0, sizeof (ack_buff));

	connect_fd = socket (AF_INET, SOCK_STREAM, 0);
	if ( -1 == connect_fd )
	{
		logger (LOG_ERR  , "Error: create socket err!\n");
		return -1;

	}
	else 
	{
		logger (LOG_DEBUG, "socket success !the socket fd is %d\n", connect_fd);
	}

	connect_addr.sin_family = AF_INET;
	if(channelno < 2)
		connect_addr.sin_port = htons (SERVER_PORT);
	else
		connect_addr.sin_port = htons (SERVER_SLAVE_PORT);
	
	connect_addr.sin_addr.s_addr = inet_addr ( SERVER_ADDR );
	bzero ( &(connect_addr.sin_zero) , 0);

	if (connect ( connect_fd, (struct sockaddr *) &connect_addr, sizeof (struct sockaddr ) ) ==  -1 )
	{
		logger (LOG_ERR  , "socket connect err\n");
		return -1;
	}
	else {
		logger (LOG_DEBUG, "connect success !\n");
	}

	return connect_fd;
	
}

int encode_socket (char sendbuff[], char recvbuff[], unsigned char channelno)
{

	int ret, recv_len;

	//check connection
	if(channelno < 2)
	{
		if ( -1 == ch_sock_fd )
		{
			ch_sock_fd  = init_socket(channelno);
			if ( -1 == ch_sock_fd )
			{
				logger (LOG_ERR  , "init  Channel socket err!\n");
				return -1;
			}
		}
	} else {
		if ( -1 == cb_sock_fd )
		{
			cb_sock_fd  = init_socket(channelno);
			if ( -1 == cb_sock_fd )
			{
				logger (LOG_ERR  , "init  CPU Bridge socket err!\n");
				return -1;
			}
		}

	}

	// send out message
	if(channelno < 2)
		ret = send(ch_sock_fd, sendbuff, strlen(sendbuff), 0 );	
	else
		ret = send(cb_sock_fd, sendbuff, strlen(sendbuff), 0 );	
	
	if ( ret < 0 )
	{
		logger (LOG_ERR  , "send message err!\n");
		if(channelno < 2)
		{
			close ( ch_sock_fd );
			ch_sock_fd = -1;
		} else {
			close ( cb_sock_fd );
			cb_sock_fd = -1;
		}
		
		return ret;
	}
	else 
	{
		logger (LOG_DEBUG, "the sendbuff's context is %s , the send length is %d\n", sendbuff, ret);
	}

	// receive message
	if(channelno < 2)
		recv_len = recv( ch_sock_fd, recvbuff, MAX_CMDBUFF_SIZE, 0);
	else
		recv_len = recv( cb_sock_fd, recvbuff, MAX_CMDBUFF_SIZE, 0);
		
	if ( recv_len == -1 )
	{
		logger (LOG_ERR  , "receive message err!\n");
		return -1;
	}
	else
	{
		/*test the context of the recvbuff*/
		logger (LOG_DEBUG, "the recv_len is %d  recvbuff's context is %s\n", recv_len, recvbuff);
	}

	return 0;

}

int getVideoState(unsigned char channelno, ENCODE_VIDEO_STATE *videostate)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];

	char vid_bit_type[4];
	char vid_bit_rate[9];
	char resolving[8];
	char frm_rate[3]; 
	char gop[4];
	unsigned char ch_no;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);

	if(channelno >= 2)
		ch_no = channelno - 2;
	else
		ch_no = channelno;
	sprintf(send_socket,"GetVideo %d \n\r", ch_no);
//	sprintf ( send_socket, "GetTime\n\r");
//	sprintf ( send_socket, "GetNetwork\n\r");
//	sprintf (send_socket, "GetAudio 0\n\r");
//	sprintf ( send_socket, "Restart\n\r");
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] ", vid_bit_type);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ] " ,vid_bit_rate);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", resolving);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", frm_rate);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", gop);

		memcpy(videostate->vidBitType, vid_bit_type,sizeof(vid_bit_type));
		memcpy(videostate->vidBitRate, vid_bit_rate,sizeof(vid_bit_rate));
		memcpy(videostate->resolving, resolving,sizeof(resolving));
		memcpy(videostate->frameRate, frm_rate,sizeof(frm_rate));
		memcpy(videostate->gop, gop,sizeof(gop));
		//videostate->gop = gop;
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;
	
}

int getNetworkState(unsigned char channelno, ENCODE_NET_STATE *net_state)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char ip_buff[17];
	char mask_buff[17];
	char gateway_buff[17];
	
	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	
	sprintf(send_socket,"GetNetwork\n\r");
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] %*[^=]=%[^ ] %*[^=]=%[^ ] ",ip_buff, mask_buff, gateway_buff);
		logger (LOG_DEBUG, "the ipaddr is %s \n,the mask is %s\n,the gateway is %s\n", ip_buff,mask_buff,gateway_buff);
		memcpy(net_state->ipaddr, ip_buff ,sizeof(ip_buff));
		memcpy(net_state->mask, mask_buff,sizeof(mask_buff));
		memcpy(net_state->gateway, gateway_buff,sizeof(gateway_buff));
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;
}

int getPROTOCOLState(unsigned char channelno, ENCODE_PROTOCOL_STATE *protocol_state)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket[MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char protocol[6];
	char ip[16];
 	char port[8];
	unsigned char ch_no;
	
	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);

	if(channelno >= 2)
		ch_no = channelno - 2;
	else
		ch_no = channelno;
	sprintf(send_socket,"getMulti %d \n\r", ch_no);
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] " ,protocol);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ]", ip);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", port);
		memcpy(protocol_state->protocol, protocol,sizeof(protocol));
		memcpy(protocol_state->ip, ip,sizeof(ip));
		memcpy(protocol_state->port, port,sizeof(port));
		printf("%s\n",protocol_state->protocol);
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;
}

int getOSDState (unsigned char channelno ,ENCODE_OSD_STATE *osdstate)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket[MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	
	char channel_name[16];
	char showtime[4];
	char location[4],color[4];
	unsigned char ch_no;
	
	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);

	if(channelno >= 2)
		ch_no = channelno - 2;
	else
		ch_no = channelno;
	sprintf(send_socket,"GetOSD %d\n\r", ch_no);
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] ", location);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ]", showtime);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^\"]%*c%[^\"]", channel_name);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", color);
		
		/*printf ("the location is %d \n", location);
		printf ("the systime is %s\n", showtime);
		printf ("the channel name is %s\n", channel_name);
		printf ("the color is %d\n",  color);
		printf ("the vlost is %s\n", showvlost);*/

		if ( !strcmp(showtime, "ON") )
		{
			osdstate->show_time = 1;
		}
		else
		{
			osdstate->show_time = 2;

		}
		memcpy(osdstate->location, location,sizeof(location));
		memcpy(osdstate->channel_name, channel_name,sizeof(channel_name));
		memcpy(osdstate->color, color,sizeof(color));
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;

}


int getPICTUREState(unsigned char channelno, ENCODE_PICTURE_STATE *picture_state)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket[MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char bright[4];
	char contrast[4];
	char chroma[4];
	char saturation[4];
	unsigned char ch_no;
	
	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	
	if(channelno >= 2)
		ch_no = channelno - 2;
	else
		ch_no = channelno;
	sprintf(send_socket,"GetPicture %d\n\r", ch_no);
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
		printf ("the channelno is %d\n", channelno);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] ", bright);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ]", contrast);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", chroma);
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%*[^ ] %*[^=]=%[^ ]", saturation);
		memcpy(picture_state->bright, bright,sizeof(bright));
		memcpy(picture_state->contrast, contrast,sizeof(contrast));
		memcpy(picture_state->chroma, chroma,sizeof(chroma));
		memcpy(picture_state->saturation, saturation,sizeof(saturation));
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}

	return 0;
}


int getSysTime ( ENCODE_SYSTIME *systime )
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char sys_time[20];
	unsigned char channelno = 0;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	
	sprintf(send_socket,"GetTime \n\r" );
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] " ,sys_time);
		printf ("the system time is %s \n", sys_time);
		memcpy(systime->time, sys_time,sizeof(sys_time));	
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;
}


int getNTP ( ENCODE_NTP *sysntp)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char server_ip[17];
	char interval[8];
	unsigned char channelno = 0;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	
	sprintf(send_socket,"GetNTP \n\r" );
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%[^ ] " ,server_ip);
		sscanf ( recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ]", interval );
		printf ("the NTP server ip is %s \n", server_ip);
		printf (" the NTP intervals is %s\n" , interval );
		memcpy(sysntp->server_addr, server_ip ,sizeof(server_ip));
		memcpy(sysntp->interval, interval,sizeof(interval));
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;

}

int getPatten (  unsigned char channelno , ENCODE_MODE_STATE *patten)
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char stream_buff[3];
	unsigned char ch_no;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	memset (stream_buff, 0, sizeof ( stream_buff));

	if(channelno >= 2)
		ch_no = channelno - 2;
	else
		ch_no = channelno;
	sprintf(send_socket,"GetPatten %d \n\r" , ch_no);
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		sscanf (recv_socket, "%*[^=]=%*[^ ] %*[^=]=%[^ ]", stream_buff);
		printf ("the stream type is %s\n",stream_buff);

		if  ( !strcmp (stream_buff, "TS"))
		{
			patten->streamtype = 1;
		}
		else if ( !strcmp(stream_buff, "ES"))
		{
			patten->streamtype = 2;
		}
		else {
			patten->streamtype = 3;
		}

	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
	
	return 0;
}


int setEncode ( unsigned char channelno, char commond[] )
{
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	char ack_buff[4];
	int ack_value;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	memset (ret_buff, 0 , sizeof (ret_buff));
	memset (ack_buff, 0, sizeof (ack_buff));
	
	if ( encode_socket( commond, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	
	return 0;

}

char * getONOFF ( unsigned char type )
{

	if ( 1 == type )
	{
		return "ON";
	}
	else {
		return "OFF";
	}

}


char *getCompresstype ( unsigned char type)
{
	switch (type)
	{
		case 1:
			return "MPEG2";
		case 2:
			return "MPEG4";
		case 3:
			return "H264";
		case 4:
			return "AVS";
		default :
			return "UNKOWN";
	}

}


char *getStreamtype ( unsigned char type )
{

	switch (type)
	{
		case 1:
			return "TS";
		case 2:
			return "ES";

		default :
			return "UNKOWN";
	}

}

int getCardInfo( CARD_INFO *card_info)
{	
	card_info->card_no = card_no;
	card_info->admin_state = adminState;
	memcpy(card_info->card_name, CardName, MAX_DISPLAYNAME_LEN);
	card_info->card_type =CARD_TYPE_ENCODE;
	card_info->version_HW[0] = VERSION_HW0;
	card_info->version_HW[1] = VERSION_HW1;
	card_info->version_FW[0] = VERSION_FW0;
	card_info->version_FW[1] = VERSION_FW1;
	card_info->version_SW[0] = VERSION_SW0;
	card_info->version_SW[1] = VERSION_SW1;
	memcpy(card_info->version_OS, OSversion, MAX_DISPLAYNAME_LEN);
	return 0;	

}

int getAlarm( GET_ALARMS_RSP *alarm_info)
{
	alarm_info->alarmNum = 0;
	alarm_info->rsp_type = 0;
	alarm_info->data.alarms[0].alarm_type = ATYPE_PORTDOWAN;
	alarm_info->data.alarms[0].port_no = 1;

	return 0;
}


int ProcessMsg(char * buffer, int size, int fds)
{
	char rspbuff[MAX_CMDBUFF_SIZE];
	char commond_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_485 *msgheader;
	MSG_HEAD_485 *msgrspheader;
	int send_size, ret;

	ENCODE_VIDEO_STATE *video_state;
	ENCODE_NET_STATE *net_state;
	ENCODE_PROTOCOL_STATE *protocol_state;
	ENCODE_OSD_STATE *osd_state;
	ENCODE_PICTURE_STATE *picture_state;
	ENCODE_SYSTIME *systime;
	ENCODE_NTP *sysntp;
	ENCODE_CTRL *sys_ctrl;
	ENCODE_MODE_STATE *patten;
	CARD_INFO *cardinfo;
	GET_ALARMS_RSP *alarmrsp;
	unsigned char ch_no = 0;
	
	msgheader = (MSG_HEAD_485 *)buffer;
	msgrspheader = (MSG_HEAD_485 *)rspbuff;

	memset (commond_buff, 0,sizeof (commond_buff));
	memset ( msgrspheader, 0 , sizeof(MSG_HEAD_485) );
	msgrspheader->code = msgheader->code;
	msgrspheader->src = msgheader->dst;
	msgrspheader->dst = msgheader->src;
	msgrspheader->onu_nodeid = msgheader->onu_nodeid;
	/*if(msgheader->onu_nodeid >= 2)
		msgheader->onu_nodeid -= 2;*/

	printf ("process message:%d\n", msgheader->code);

	switch (msgheader->code)
	{
		case C_CARD_GET:
			cardinfo = (CARD_INFO *)(rspbuff + sizeof(MSG_HEAD_485));
			ret = getCardInfo(cardinfo);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (CARD_INFO);
			}
			else {

				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_VIDEO_GET:
			video_state = (ENCODE_VIDEO_STATE *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getVideoState(msgheader->onu_nodeid, video_state);
			if ( 0 == ret)
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof(ENCODE_VIDEO_STATE);
			}
			else {
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;

			}
			break;
		case C_ENCODE_NET_GET:
			net_state = (ENCODE_NET_STATE *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getNetworkState(msgheader->onu_nodeid, net_state);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length =  sizeof (ENCODE_NET_STATE);
			}
			else{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_PROTOCOL_GET:
			protocol_state = (ENCODE_PROTOCOL_STATE *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getPROTOCOLState(msgheader->onu_nodeid, protocol_state);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_PROTOCOL_STATE);
			}
			else {

				msgrspheader->rsp = ret;
				msgrspheader->length = 0;

			}
			break;	
		case C_ENCODE_OSD_GET:
			osd_state = (ENCODE_OSD_STATE *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getOSDState(msgheader->onu_nodeid, osd_state);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_OSD_STATE);
			}
			else {

				msgrspheader->rsp = ret;
				msgrspheader->length = 0;

			}
			break;
		case C_ENCODE_PICTURE_GET:
			picture_state = (ENCODE_PICTURE_STATE *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getPICTUREState(msgheader->onu_nodeid, picture_state);
			if ( 0 == ret )
			{
				printf("here\n here\n here\n here\n here\n");
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_PICTURE_STATE);
			}
			else {

				msgrspheader->rsp = ret;
				msgrspheader->length = 0;

			}
			break;
		case C_ENCODE_SYSTIME_GET:
			systime = ( ENCODE_SYSTIME *)(rspbuff + sizeof (MSG_HEAD_485));
			ret = getSysTime ( systime );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_SYSTIME);
			}
			else {

				msgrspheader->rsp =  ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_NTP_GET:
			sysntp = (ENCODE_NTP * )(rspbuff + sizeof (MSG_HEAD_485));
			ret = getNTP ( sysntp);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_NTP);
			}
			else
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_MODE_GET:
			patten = (ENCODE_MODE_STATE* )(rspbuff + sizeof (MSG_HEAD_485));
			ret = getPatten ( msgheader->onu_nodeid ,patten);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = sizeof (ENCODE_MODE_STATE);
			}
			else
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_RESTART:
			sprintf(commond_buff,"Restart \n\r" );
			ret = setEncode ( 0,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_NTP_SET:
			if ( msgheader->length != sizeof (ENCODE_NTP))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			sysntp = (ENCODE_NTP *) ( buffer + sizeof (MSG_HEAD_485));
			
			sprintf(commond_buff,"SetNTP NTPSERVER=%s INTERVAL=%s \n\r", sysntp->server_addr, sysntp->interval);
			ret = setEncode ( 0,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_NET_SET:
			if ( msgheader->length != sizeof (ENCODE_NET_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			net_state = (ENCODE_NET_STATE *) ( buffer + sizeof (MSG_HEAD_485));
			sprintf (commond_buff, "SetNetwork IP=%s  MASK=%s GATEWAY=%s\n\r", net_state->ipaddr, net_state->mask, net_state->gateway);
			ret = setEncode ( net_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_SYSTIME_SET:
			if ( msgheader->length != sizeof (ENCODE_SYSTIME))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			systime = (ENCODE_SYSTIME *) ( buffer + sizeof (MSG_HEAD_485));
			sprintf ( commond_buff, "SetTime Time=%s \n\r", systime->time);
			ret = setEncode ( 0,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_PROTOCOL_SET:
			if ( msgheader->length != sizeof (ENCODE_PROTOCOL_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			protocol_state = (ENCODE_PROTOCOL_STATE*) ( buffer + sizeof (MSG_HEAD_485));
			if(protocol_state->channelno >= 2)
				ch_no = protocol_state->channelno - 2;
			else
				ch_no = protocol_state->channelno;
			sprintf(commond_buff, "SetProtocol %d Protocol=%s \n\r", ch_no, protocol_state->protocol);
			printf("%s\n",commond_buff);
			ret = setEncode ( protocol_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			
			memset(commond_buff, 0, sizeof(commond_buff));
			sprintf(commond_buff, "Enstart %s %s %d \n\r", protocol_state->ip, protocol_state->port, ch_no);
			ret = setEncode ( protocol_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_VIDEO_SET:
			if ( msgheader->length != sizeof (ENCODE_VIDEO_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			video_state = (ENCODE_VIDEO_STATE*) ( buffer + sizeof (MSG_HEAD_485));
			if(video_state->channelno >= 2)
				ch_no = video_state->channelno - 2;
			else
				ch_no = video_state->channelno;
			sprintf(commond_buff, "SetVideo %d VidBitType=%s VidBitRate=%s Resolving=%s FrmRate=%s GOP=%s \n\r", ch_no, video_state->vidBitType, video_state->vidBitRate, video_state->resolving, video_state->frameRate, video_state->gop);
			ret = setEncode ( video_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_OSD_SET:
			if ( msgheader->length != sizeof (ENCODE_OSD_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			osd_state = (ENCODE_OSD_STATE*) ( buffer + sizeof (MSG_HEAD_485));
			if(osd_state->channelno >= 2)
				ch_no = osd_state->channelno - 2;
			else
				ch_no = osd_state->channelno;
			sprintf ( commond_buff, "SetOSD %d Loc=%s Time=%s chname=%s color=%s\n\r", ch_no, osd_state->location , getONOFF( osd_state->show_time ), osd_state->channel_name, osd_state->color);
			ret = setEncode ( osd_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_PICTURE_SET:
			if ( msgheader->length != sizeof (ENCODE_PICTURE_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			picture_state = (ENCODE_PICTURE_STATE*) ( buffer + sizeof (MSG_HEAD_485));
			if(picture_state->channelno >= 2)
				ch_no = picture_state->channelno - 2;
			else
				ch_no = picture_state->channelno;
			sprintf ( commond_buff, "SetPicture %d Bright=%s Contrast=%s Chroma=%s Saturation=%s\n\r", ch_no, picture_state->bright, picture_state->contrast, picture_state->chroma, picture_state->saturation);
			ret = setEncode ( picture_state->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		case C_ENCODE_MODE_SET:
			if ( msgheader->length != sizeof (ENCODE_MODE_STATE))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			patten = (ENCODE_MODE_STATE*) ( buffer + sizeof (MSG_HEAD_485));
			if(patten->channelno >= 2)
				ch_no = patten->channelno - 2;
			else
				ch_no = patten->channelno;
			sprintf ( commond_buff, "SetPatten %d STREAMTYPE=%s\n\r", ch_no, getStreamtype(patten->streamtype));
			ret = setEncode ( patten->channelno,commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		/*case C_ENCODE_CTRL:
			if ( msgheader->length != sizeof (ENCODE_CTRL))
			{
				msgrspheader->rsp= ERROR_WRONG_ARGS;
				msgrspheader->length = 0;
				break;
			}
			sys_ctrl = (ENCODE_CTRL *) ( buffer + sizeof (MSG_HEAD_485));
			if ( 1 == sys_ctrl->flag )
			{
				sprintf ( commond_buff, "Enstart %s %d %d\n\r", sys_ctrl->ipaddr, sys_ctrl->portno, sys_ctrl->channelno);
			}
			else
			{
				sprintf ( commond_buff, "Enstop %d\n\r", sys_ctrl->channelno);
			}
			ret = setEncode ( commond_buff );
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = 0;
			}
			else 
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;*/
		case GET_ALARMS:
			alarmrsp = (GET_ALARMS_RSP *)(rspbuff + sizeof(MSG_HEAD_485));
			ret = getAlarm( alarmrsp);
			if ( 0 == ret )
			{
				msgrspheader->rsp = 0;
				msgrspheader->length = (alarmrsp->alarmNum)*sizeof (ALARM_INFO) +2 ;
			}
			else
			{
				msgrspheader->rsp = ret;
				msgrspheader->length = 0;
			}
			break;
		default:
			msgrspheader->rsp = 1;
			msgrspheader->length = 0;
			
			

	}

	send_size = msgrspheader->length + sizeof(MSG_HEAD_485);
	if (msg485_send(fds, rspbuff, send_size)  != send_size)
	{
		logger (LOG_ERR  , "message response send err!\n");
		return -1;
	}

	return 0;

}
