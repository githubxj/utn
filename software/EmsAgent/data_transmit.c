#include "msg.h"
#include "const.h"

extern  CARD_INFO   cardData[1 + MAX_CARD_NUM_UTN];
extern int fd485;

int check_data_transmit(int fd)
{
	int             i, j;
	char            send_buf[MAX_485MSG_FULLLEN];
	char            recv_buf[MAX_485MSG_FULLLEN];
	MSG_HEAD_485    *head_req, *head_rsp;
	
	memset(send_buf, 0, sizeof(send_buf));
	head_req = (MSG_HEAD_485*)send_buf;
	head_rsp = (MSG_HEAD_485*)recv_buf;
	head_req->code = C_MTT_GET_DATA;
	head_req->src = CTRL_CARDNO;
	head_req->length = 1;

	
    for(i=1; i <= MAX_CARD_NUM_UTN; i++)
    {
        if((cardData[i].card_no == 0) || (cardData[i].card_no == CTRL_CARDNO))
            continue;

        head_req->dst = cardData[i].card_no;
        if(!msg485_sendwaitrsp(fd485, send_buf, recv_buf, TIMEOUT_485MSG))
        {	
            if(head_rsp->rsp != 0)
            {
                logger (LOG_ERR , "Error: Get data error response !,  error:%d.\n", 
                head_rsp->rsp);
                continue;
            }
        }
        
        // 通过fd上传
        int snd_size = head_rsp->length + sizeof(MSG_HEAD_485);
        if(write(fd, send_buf, snd_size) != snd_size)
        {
            logger (LOG_ERR , "Error: Get alarm error response !,  error:%d.\n", 
        }
    }

	return 0;
}

