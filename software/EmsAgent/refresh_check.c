#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "refresh_check.h"

typedef struct CLIENT_ENTRY {
	unsigned long			sessionID;
	time_t				last_req_time;
	unsigned short		change_flag; /*bit0: tree, bit1:frameview*/
	unsigned short		topo_flag;	/*bit0: card1, bit1:card2, ....*/
	struct CLIENT_ENTRY *next;
} CLIENT_ENTRY;

CLIENT_ENTRY	*RefreshClient=NULL;

int getRefreshState(unsigned long sessionID, unsigned char type, unsigned char cardno)
{
	CLIENT_ENTRY 	*client;
	unsigned short 	mask;
	int ret=0;

	client = RefreshClient;
	while(client != NULL)
	{
		if(sessionID == client->sessionID)
		{
			client->last_req_time = time(NULL);
			switch(type)
			{
				case REFRESH_TYPE_TREE:
					if(client->change_flag & CHANGE_FLAG_TREEV)
						ret = 1;
					client->change_flag &= ~CHANGE_FLAG_TREEV; 
					break;
				case REFRESH_TYPE_FRAME:
					if(client->change_flag & CHANGE_FLAG_FRAMEV)
						ret = 1;
					client->change_flag &= ~CHANGE_FLAG_FRAMEV; 
					break;
				case REFRESH_TYPE_TOPO:
logger(LOG_DEBUG, "Topo flag=0x%x, sessionid=%d\n", client->topo_flag, client->sessionID);
				mask = 1<<cardno;
					if(client->topo_flag & mask)
						ret = 1;
					client->topo_flag &= ~mask; 
					break;
			}
			return(ret);
		}
		client = client->next;
	}

	/* if the client not found, it is the new request */
	client = (CLIENT_ENTRY *)malloc(sizeof(CLIENT_ENTRY));
	if(client != NULL)
	{
		client->last_req_time = time(NULL);
		client->sessionID = sessionID;
		client->change_flag = 0;
		client->topo_flag = 0;

		//add the record the head of linked list
		client->next = RefreshClient;
		RefreshClient = client;
		logger(LOG_DEBUG, "Add new refresh req client. sessionid=%d\n", client->sessionID);
	}
	else
		logger (LOG_ERR,  "Malloc memory error in getRefreshState !");

	return 0;
}

void setRefreshState(unsigned short flag)
{
	CLIENT_ENTRY 	*client;

	client = RefreshClient;
	while(client != NULL)
	{
		client->change_flag |= flag;
		client = client->next;
	}
}

void setRefreshState_Topo(unsigned char card)
{
	CLIENT_ENTRY 	*client;
	unsigned short 	mask;

	mask = 1<< card;
	
	client = RefreshClient;	
	while(client != NULL)
	{
		client->topo_flag |= mask;
		client = client->next;
	}
}

void clearRefreshClient()
{
	CLIENT_ENTRY 	*client, *previous;
	time_t 			now;

	now = time(NULL);
	previous = NULL;
	client = RefreshClient;
	while(client != NULL)
	{
		if((now - client->last_req_time) > REFRESH_IDLE_TIME)
		{
			//free the idle client
			if(previous == NULL)
			{
				//it is linked list header
				logger(LOG_DEBUG, "Remove refresh req client FROM HEADER. sessionid=%d\n", client->sessionID);
				RefreshClient= client->next;
				free(client);
				client = RefreshClient;				
			} else {
				previous->next = client->next;
				logger(LOG_DEBUG, "Remove refresh req client. sessionid=%d\n", client->sessionID);
				free(client);
				client = previous->next;
			}
		} else {
			previous = client;
			client = client->next;
		}
	}

}

