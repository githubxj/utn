#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>

#include "const.h"
#include "msg.h"

const char cgi_pipes[] = "/tmp/snmp_pipes";
const char cgi_pipec[] = "/tmp/snmp_pipec";

int send_cmd(char* send_buff, char * recv_buff)
{
	fd_set inputfds;
	struct timeval tv;
	int pipefds, pipefdc;
	int retval;
	MSG_HEAD_CMD *msg_header;

	pipefds = open(cgi_pipes, O_RDWR);
       if(-1 == pipefds)
		return -1;

	pipefdc = open(cgi_pipec, O_RDWR);
       if(-1 == pipefdc) {
	   	close(pipefdc);
		return -1;
       }
	msg_header = (MSG_HEAD_CMD *) send_buff;
	retval = write(pipefds, send_buff, msg_header->length + sizeof(MSG_HEAD_CMD));
	if(retval != msg_header->length + sizeof(MSG_HEAD_CMD)) {		
	   	close(pipefds);
	   	close(pipefdc);
		return -1;
	}
	
	while(1) {
		FD_ZERO(&inputfds);
		FD_SET(pipefdc, &inputfds);
		   
		/* Wait the peer response up to 3 seconds. */
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		
		retval = select(pipefdc +1, &inputfds, NULL, NULL, &tv);

		if (retval == -1) {
		   	close(pipefds);
		   	close(pipefdc);
               	return -1;
		} else if (retval) {
           		if(FD_ISSET(pipefdc, &inputfds))
           		{
           			int recv_size;
				MSG_HEAD_CMD_RSP *msgrsp_header;
					
           			recv_size = read(pipefdc, recv_buff, MAX_CMDBUFF_SIZE);
				msgrsp_header = (MSG_HEAD_CMD_RSP *) recv_buff;
				
				if(recv_size >= sizeof(MSG_HEAD_CMD_RSP) 
					&& recv_size == msgrsp_header->length + sizeof(MSG_HEAD_CMD_RSP))
				   	close(pipefds);
	   				close(pipefdc);
					return 0;
			}
			
		} else {
			/* No data within 5 second */
			break;
		}

        }

	close(pipefds);
	close(pipefdc);

	return -1;
}

int getnlen(const char * str, int max)
{
	int i;
	
	for(i=0; i<max; i++)
	{
		if(*(str + i) == '\0')
			break;
	}

	return i;
	
}