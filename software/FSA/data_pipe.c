#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "../common/iniFile.h"
#include "fsalib.h"

#ifdef DAT_CARD
#define MAX_DATA_GROUP		5
#else
#define MAX_DATA_GROUP		2
#endif
#define MAX_CH_CLIENT			4	
#define MAX_IDLETIME			300
#define RCV_SND_TIMEOUT		2000
#define BUFF_SIZE				2048
#define MAX_CLIENT_FDS 			(MAX_ONU_NUM+1)*MAX_DATA_GROUP*MAX_CH_CLIENT


struct pollfd client_fds[MAX_CLIENT_FDS];
time_t client_timeout[MAX_ONU_NUM+1][MAX_DATA_GROUP][MAX_CH_CLIENT];

unsigned long rs_statistic_send[MAX_ONU_NUM+1][MAX_DATA_GROUP];
unsigned long rs_statistic_recv[MAX_ONU_NUM+1][MAX_DATA_GROUP];

pthread_attr_t listenattr;
pthread_t thid_listenattr;

pthread_attr_t clientattr;
pthread_t thid_clientattr;

int init_complete=0;
int rstcp_base_port = 5000;
int rstcp_port_step = 100;

extern int rstcplog;


int getSerialServerParam()
{
	int value;
	
	if(0 != access(IO_CONF_FILE, F_OK))
    		return -1;

	IniFilePtr pIni = iniFileOpen(IO_CONF_FILE);
	if(NULL == pIni)
		return -1;

	
	value = iniGetInt(pIni, "SerialServer", "PORT");
	if(value >0)
		rstcp_base_port = value;
 
	value = iniGetInt(pIni, "SerialServer", "STEP");
	if(value >0)
		rstcp_port_step = value;
 
	iniFileFree(pIni);
	return 0;
}

void get_rs_statistics(FSA_LOWSPD_STATE *fsa_lowspd_state, unsigned char onu_id)
{
	unsigned short con_count;
	int i, k, clientfd_idx;
	
	fsa_lowspd_state->rs_rcv[0] = htonl(rs_statistic_recv[onu_id][0]);
	fsa_lowspd_state->rs_rcv[1] = htonl(rs_statistic_recv[onu_id][1]);
	fsa_lowspd_state->rs_rcv[2] = htonl(rs_statistic_recv[onu_id][2]);
	fsa_lowspd_state->rs_rcv[3] = htonl(rs_statistic_recv[onu_id][3]);
	fsa_lowspd_state->rs_rcv[4] = htonl(rs_statistic_recv[onu_id][4]);
	fsa_lowspd_state->rs_snd[0] = htonl(rs_statistic_send[onu_id][0]);
	fsa_lowspd_state->rs_snd[1] = htonl(rs_statistic_send[onu_id][1]);
	fsa_lowspd_state->rs_snd[2] = htonl(rs_statistic_send[onu_id][2]);
	fsa_lowspd_state->rs_snd[3] = htonl(rs_statistic_send[onu_id][3]);
	fsa_lowspd_state->rs_snd[4] = htonl(rs_statistic_send[onu_id][4]);

	fsa_lowspd_state->rs_port[0] = htons(rstcp_base_port + onu_id);
	fsa_lowspd_state->rs_port[1] = htons(rstcp_base_port + rstcp_port_step + onu_id);
	fsa_lowspd_state->rs_port[2] = htons(rstcp_base_port + rstcp_port_step*2 + onu_id);
	fsa_lowspd_state->rs_port[3] = htons(rstcp_base_port + rstcp_port_step*3 + onu_id);
	fsa_lowspd_state->rs_port[4] = htons(rstcp_base_port + rstcp_port_step*4 + onu_id);

	for(k=0; k<MAX_DATA_GROUP; k++)
	{
		con_count=0;
		for ( i = 0; i < MAX_CH_CLIENT; i++)
		{
		clientfd_idx = ((MAX_ONU_NUM+1)*k + onu_id)*MAX_CH_CLIENT + i;
			if ( client_fds[clientfd_idx].fd > 0)
				con_count++;
		}
		fsa_lowspd_state->rs_connection[k] = htons(con_count);
	}
}

void reset_rs_statistics()
{
	int i,j;

	for(i=0; i<=MAX_ONU_NUM; i++)
	{
		for(j=0; j<MAX_DATA_GROUP; j++)
		{
			rs_statistic_recv[i][j] = 0;
			rs_statistic_send[i][j] = 0;
		}
	}
}

void check_new_connection(struct pollfd* listen_fds)
{
	int pollfd_idx, clientfd_idx, new_clientfd;
	struct sockaddr_in client_addr;
	socklen_t sin_size;
	int i, j, retval;
	int group_no;	

	retval = poll(listen_fds, (MAX_ONU_NUM + 1)*MAX_DATA_GROUP, 1000);
	if (-1 == retval ) 
	{
		logger (LOG_ERR, "Listen socket poll error.\n");
		return;
	}
	else if ( retval )
	{
		//check whether a new connection comes.
		for(group_no=0; group_no<MAX_DATA_GROUP; group_no++)
		{
			for (i =0 ; i <= MAX_ONU_NUM; i++)
			{
				pollfd_idx = (MAX_ONU_NUM+1)*group_no + i;
	       		if(listen_fds[pollfd_idx].revents == POLLIN)
				{
					sin_size = sizeof(struct sockaddr_in);
					new_clientfd = accept(listen_fds[pollfd_idx].fd, (struct sockaddr *)&client_addr, &sin_size);
					if ( new_clientfd <= 0)
					{
						logger (LOG_ERR, "ONU number is %d, thread id %lld, accept new comer err, time:%f sec\n", i, (long long)pthread_self() , time(NULL) );
						continue;
					}

					for ( j = 0; j < MAX_CH_CLIENT; j++)
					{
						clientfd_idx = ((MAX_ONU_NUM+1)*group_no + i)*MAX_CH_CLIENT + j;
						if ( client_fds[clientfd_idx].fd <= 0)
						{
							client_fds[clientfd_idx].fd = new_clientfd;
							client_fds[clientfd_idx].events = POLLIN |POLLHUP;
							client_timeout[i][group_no][j] = time(NULL); //update idle timeout
							break;
						}
					}

					if(j <MAX_CH_CLIENT)
					{
						if(rstcplog)
							logger(LOG_DEBUG,"Group:%d, ONU:%d,  thread id is %lld, new connection client[%d] %s:%d\n", group_no, i, (long long)pthread_self() , new_clientfd,
			                       		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
					}
					else
					{
						logger (LOG_WARNING, "max client arrive , Group:%d, ONU:%d, thread id %lld\n", group_no, i, (long long)pthread_self() );
						close(new_clientfd);
						continue;
					}					
				}
			}
		}

	}

}

void *  listen_handle_thread(void * param )
{
	struct pollfd listen_fds[(MAX_ONU_NUM+1)*MAX_DATA_GROUP];
	struct sockaddr_in server_addr;
	int pollfd_idx, group_no;
	unsigned short port_no;	
	int i, timeout, on;
	int policy;
	struct sched_param sh_param;

	if(pthread_getschedparam(pthread_self(), &policy, &sh_param)==0)
		printf("THD:Socket listen-%d, policy:%d, priority:%d\n", getpid(), policy, sh_param.sched_priority);

	getSerialServerParam();
	//printf("Base port:%d, step=%d\n", rstcp_base_port, rstcp_port_step);

	// create the listent socket for each ONU 
	for(group_no=0; group_no<MAX_DATA_GROUP; group_no++)
	{
		for ( i = 0; i <= MAX_ONU_NUM; i++ )
		{
			pollfd_idx = (MAX_ONU_NUM+1)*group_no + i;
			listen_fds[pollfd_idx].events = POLLIN;	
			listen_fds[pollfd_idx].fd = socket(AF_INET, SOCK_STREAM, 0);
			if (-1 ==  listen_fds[pollfd_idx].fd)
			{
				logger(LOG_ERR, "Create listen sock err. Group-%d, ONU-%d\n", group_no, i);
				continue;
			}

			port_no = rstcp_base_port + group_no*rstcp_port_step+ i;
			//printf("group=%d, onu=%d, port=%d\n", group_no, i, port_no);
			
			//bind API 
			on = 1;
			setsockopt( listen_fds[pollfd_idx].fd , SOL_SOCKET, SO_REUSEADDR,&on, sizeof(on) );
			//set recv and send timeout.
			timeout = RCV_SND_TIMEOUT;
			setsockopt(listen_fds[pollfd_idx].fd, SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(int) );
			setsockopt(listen_fds[pollfd_idx].fd, SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(int));
			
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(port_no);
			server_addr.sin_addr.s_addr = INADDR_ANY;
			memset(server_addr.sin_zero, 0, sizeof (server_addr.sin_zero));

			if (bind(listen_fds[pollfd_idx].fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
			{
				logger(LOG_ERR, "Bind listen sock err. Group-%d, Port-%d\n", group_no, port_no);
				continue;
			}

			if (-1 == listen (listen_fds[pollfd_idx].fd, MAX_CH_CLIENT ))
			{
				logger(LOG_ERR, "Start listen sock err. Group-%d, Port-%d\n", group_no, port_no);
				continue;
			}

			if(rstcplog)
				logger (LOG_DEBUG, "start listening ....port number is %d, process id is %d, thread id is %lld\n",
						port_no, getpid(),  (long long)pthread_self() );

		}
	}
	
	while (1)
	{
		check_new_connection(listen_fds);		 
	} 	
	
}

void *  client_handle_thread(void * param )
{
	int group_no;
	int pollfd_idx;
	int i, j, retval, ret;
	char recv_buffer[BUFF_SIZE];
	int loop_count = 0;
	time_t current_sec;
	int policy;
	unsigned char onu_id;
	struct sched_param sch_param;

	if(pthread_getschedparam(pthread_self(), &policy, &sch_param)==0)
		printf("THD:data_p client-%d, policy:%d, priority:%d\n", getpid(), policy, sch_param.sched_priority);

	while (1)
	{
		loop_count++;

		//check idle timeout client.
		if ((loop_count & 0x0F) == 0)
		{
			if(rstcplog)
				logger (LOG_DEBUG, "Loop=%d, check client timeout.\n", loop_count);

			for(group_no=0; group_no<MAX_DATA_GROUP; group_no++)
			{
				for (i =0 ; i <= MAX_ONU_NUM; i++)
				{
					for(j=0; j<MAX_CH_CLIENT; j++)
					{
						pollfd_idx = ((MAX_ONU_NUM+1)*group_no + i)*MAX_CH_CLIENT + j;
						if(client_fds[pollfd_idx].fd <=0 )
							continue;
						
						current_sec = time(NULL);
						if ( (current_sec - client_timeout[i][group_no][j]) > MAX_IDLETIME )
						{
							close(client_fds[pollfd_idx].fd);
							client_fds[pollfd_idx].fd = -1;
							client_timeout[i][group_no][j] = 0;
							logger (LOG_WARNING, "Client idle timeout:group-%d, ONU-%d, client closed\n", group_no, i);
						}
						
					}
				}
			}
		}
	

		retval = poll(client_fds, MAX_CLIENT_FDS, 1000);
		if (-1 == retval ) 
		{
			logger (LOG_ERR, "Client socket poll error.\n");
			continue;
		}
		else if ( retval )
		{
			//check whether client data comes.
			for(group_no=0; group_no<MAX_DATA_GROUP; group_no++)
			{
				for (i =0 ; i <= MAX_ONU_NUM; i++)
				{
					for(j=0; j<MAX_CH_CLIENT; j++)
					{
						pollfd_idx = ((MAX_ONU_NUM+1)*group_no + i)*MAX_CH_CLIENT + j;

						if(client_fds[pollfd_idx].revents & POLLIN)
						{					
							ret = recv(client_fds[pollfd_idx].fd, recv_buffer, BUFF_SIZE, 0);
							if ( ret <= 0 )					
							{
								logger (LOG_WARNING, "Client Receive error:group-%d, ONU-%d, client closed\n", group_no, i);
								close(client_fds[pollfd_idx].fd);
								client_fds[pollfd_idx].fd = -1;
								client_timeout[i][group_no][j] = 0;
							}
							else
							{
								if ( ret <= BUFF_SIZE)
								{
									if(rstcplog)
									{
										struct timeval checkpoint;

										gettimeofday(&checkpoint, NULL);
										logger (LOG_DEBUG,  "%d-Client Rcv %d bytes,group-%d, ONU-%d\n", checkpoint.tv_usec/1000, ret, group_no, i);
									}

									if(i==0)
										onu_id = 0xFF;
									else
										onu_id = i;
									
									//send to ONU.
									sendRSData(onu_id, group_no, recv_buffer, ret);

									client_timeout[i][group_no][j] = time(NULL); //update idle timeout
									rs_statistic_send[i][group_no] += ret;
								}
							}
						}
						else if(client_fds[pollfd_idx].revents & POLLHUP)
						{	
							// remote socket closed
							logger (LOG_WARNING, "Client :group-%d, ONU-%d remote closed\n", group_no, i);
							close(client_fds[pollfd_idx].fd);
							client_fds[pollfd_idx].fd = -1;
							client_timeout[i][group_no][j] = 0;
						}
					}
				}
			}
		}
//		else
//			printf("TCP no data\n");
	} 		

	return 0;
}

int datapipe_init()
{
	int i , res;
 	struct sched_param param;

	for ( i = 0; i < MAX_CLIENT_FDS; i++ )
		client_fds[i].fd = -1;
	
	pthread_attr_init(&listenattr);
	pthread_attr_setdetachstate(&listenattr, PTHREAD_CREATE_DETACHED);
	//pthread_attr_setschedpolicy(&listenattr, SCHED_RR);
	//param.sched_priority = SCHED_PRIORITY_LOW;
	//pthread_attr_setschedparam(&listenattr, &param);
	res = pthread_create(&thid_listenattr, &listenattr, listen_handle_thread , (void *)&i);
	if ( res < 0)
	{
		logger (LOG_ERR , "unable to start listen thread\n");
		return -1;
	}
		
	pthread_attr_init(&clientattr);
	pthread_attr_setdetachstate(&clientattr, PTHREAD_CREATE_DETACHED);
	//pthread_attr_setschedpolicy(&clientattr, SCHED_RR);
	//param.sched_priority = SCHED_PRIORITY_LOW;
	//pthread_attr_setschedparam(&clientattr, &param);
	res = pthread_create(&thid_clientattr, &clientattr, client_handle_thread , (void *)&i);
	if ( res < 0)
	{
		logger (LOG_ERR , "unable to start client thread\n");
		return -1;
	}

	logger (LOG_DEBUG, "create thread successful.number is %d,  process id is %d \n",i, getpid());

	init_complete = 1;
	
	return 0;
}


int proc_recvRSData(unsigned char onu_id, unsigned char port, char * data, int data_len)
{
	int act_client, client_idx;
	
	if( (onu_id == 0) || (onu_id > MAX_ONU_NUM) || (port >= MAX_DATA_GROUP) ||(init_complete==0))
		return -1;

	//send to all client.
	for ( act_client = 0; act_client < MAX_CH_CLIENT; act_client++ )
	{
		client_idx = ((MAX_ONU_NUM+1)*port + onu_id)*MAX_CH_CLIENT + act_client;
		if ( 0 < client_fds[client_idx].fd )//active client.
		{
			if(rstcplog)
				logger (LOG_DEBUG,  "RS recv: %d bytes, group-%d, ONU-%d, send to%d\n", data_len, port, onu_id, act_client);
			if ( -1 == send(client_fds[client_idx].fd, data, data_len , 0))
				logger ( LOG_ERR, "ERR: send  RS messge to client failed. ONU:%d, client fd:%d.\n", onu_id, client_fds[client_idx].fd);

			client_timeout[onu_id][port][act_client] = time(NULL); //update idle timeout
			rs_statistic_recv[onu_id][port] += data_len;
		}
	}

	return 0;
	
}

void thread_clean()
{
	pthread_attr_destroy(&listenattr);
	pthread_attr_destroy(&clientattr);
}


