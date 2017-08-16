#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "const.h"
#include "msg.h"

namespace comm {

typedef union semun
{
  int val;
  struct semid_ds *st;
  ushort * array;
}semun_t;

const char cgi_pipes[] = "/tmp/cgi_pipes";
const char cgi_pipec[] = "/tmp/cgi_pipec";

int send_cmd(char* send_buff, char * recv_buff)
{
	fd_set inputfds;
	struct timeval tv;
	int pipefds, pipefdc;
	int retval, ret=-1;
	MSG_HEAD_CMD *msg_header;
	int semid, sem_timeout;
	struct sembuf op;

	pipefds = open(cgi_pipes, O_RDWR);
       if(-1 == pipefds)
		return -1;

	pipefdc = open(cgi_pipec, O_RDWR);
       if(-1 == pipefdc) {
	   	close(pipefds);
		return -1;
       }

	/* get the semaphore and proceed ahead */
	semid = semget((key_t)SEM_PIPE_KEY, 1, 0666);
	if(semid==-1)
	{
	   	close(pipefds);
	   	close(pipefdc);
		return -1;
	}

	op.sem_num = 0; //signifies 0th semaphore
	op.sem_op = -1; 
  	op.sem_flg = IPC_NOWAIT; 	
 	sem_timeout = 50;		//current PPC405 kernel does not support semtimedop()
	while(sem_timeout >0)
	{	
		if( semop(semid,&op,1) == 0)
			break;
		if(errno!=EAGAIN) {
			sem_timeout = 0;
			break;
		}
		usleep(100000);
		sem_timeout--;
	}

	if(sem_timeout<=0)
	{
	   	close(pipefds);
	   	close(pipefdc);
		return -1;
	}
	
	msg_header = (MSG_HEAD_CMD *) send_buff;
	retval = write(pipefds, send_buff, msg_header->length + sizeof(MSG_HEAD_CMD));
	if(retval == msg_header->length + sizeof(MSG_HEAD_CMD)) 
	{	
	   while(1) {
		FD_ZERO(&inputfds);
		FD_SET(pipefdc, &inputfds);
		   
		/* Wait the peer response up to 5 seconds. */
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		
		retval = select(pipefdc +1, &inputfds, NULL, NULL, &tv);

		if (retval == -1) {
			break;
		} else if (retval) {
           		if(FD_ISSET(pipefdc, &inputfds))
           		{
           			int recv_size;
				MSG_HEAD_CMD_RSP *msgrsp_header;
					
           			recv_size = read(pipefdc, recv_buff, MAX_CMDBUFF_SIZE);
				msgrsp_header = (MSG_HEAD_CMD_RSP *) recv_buff;
				
				if(recv_size >= sizeof(MSG_HEAD_CMD_RSP) 
					&& recv_size == msgrsp_header->length + sizeof(MSG_HEAD_CMD_RSP))
				{   	
					ret = 0;
					break;
				}
			}
			
		} else {
			/* No data within 5 second */
			break;
		}
	   }
	}

	close(pipefds);
	close(pipefdc);
	
	//free the semaphore
	op.sem_op = 1; 
	semop(semid, &op, 1);
	
	return(ret);
}
}
