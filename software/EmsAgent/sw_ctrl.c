#include <sys/ioctl.h>		//ioctl().
#include <fcntl.h>			//O_RDWR.
#include <unistd.h>		//close().
#include <stdio.h>			//printf().
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h> 

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "sw_ctrl.h"

#define INTERFACE_UP		1
#define INTERFACE_DOWN		0

SW_PORT_COUNT statistics;
mc_group_struct amcgrp_cache[MARL_LEN_MAX];//search key is group ip address.
drv_pvlan_struct vlan_cache[MAX_VLAN_NUM];
SW_CONFIG swconfig_cache;


int GetSWCounters( unsigned char cardno,  SW_PORT_COUNT *swcount )
{

	int fd , i, j ;
	drv_dma_counters_struct counter;
	memset(&counter, 0 ,sizeof(drv_dma_counters_struct));

	if ( CTRL_CARDNO  != cardno )
		return -1;

	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	if ( 0 != ioctl(fd, IOCTL_GET_DMACOUNTERS, &counter) )
	{
		logger(LOG_ERR, "IOCTL ERR GET COUNTERS ERR BY DMA.\n");
		return ERROR;
	}
	else
	{
		logger(LOG_DEBUG, "IOCTL SUCCESS GET COUNTERS.  \n");
		j = 0;
		for ( i = 0; i < 1664 ; i = i + 64 )
		{
			/*if ( (1600 == i) || ( 1536 == i ) )
			{
				swcount->recvbytecount[j] =htonl( counter.counterdata[i + 8]);
				swcount->recvpktcount[j] = htonl(counter.counterdata[ i + 9 ] & 0xFFFFFF);
				swcount->recvMCpktcount[j] = htonl(counter.counterdata[ i + 10 ] & 0xFFFFFF);
				swcount->recvBCpktcount[j] = htonl(counter.counterdata[ i + 11 ] & 0xFFFFFF);

				swcount->tranbytecount[j] = htonl(counter.counterdata[i + 24 ]);
				swcount->tranpktcount[j] = htonl(counter.counterdata[ i + 25 ] & 0xFFFFFF);
				swcount->tranMCpktcount[j] = htonl(counter.counterdata[ i + 26 ] & 0xFFFFFF);
				swcount->tranBCpktcount[j] = htonl(counter.counterdata[ i + 27 ] & 0xFFFFFF);
			}
			else
			{
				swcount->recvbytecount[j] = htonl(counter.counterdata[i + 8]);
				swcount->recvpktcount[j] = htonl(counter.counterdata[ i + 9 ] & 0xFFFFF);
				swcount->recvMCpktcount[j] = htonl(counter.counterdata[ i + 10 ] & 0xFFFFF);
				swcount->recvBCpktcount[j] = htonl(counter.counterdata[ i + 11 ] & 0xFFFFF);

				swcount->tranbytecount[j] = htonl(counter.counterdata[i + 24 ]);
				swcount->tranpktcount[j] = htonl(counter.counterdata[ i + 25 ] & 0xFFFFF);
				swcount->tranMCpktcount[j] = htonl(counter.counterdata[ i + 26 ] & 0xFFFFF);
				swcount->tranBCpktcount[j] = htonl(counter.counterdata[ i + 27 ] & 0xFFFFF);
			}*/
			if ( (1600 == i) || ( 1536 == i ) )
			{
				swcount->recvbytecount[j] =htonl( counter.counterdata[i + 7]);
				swcount->recvpktcount[j] = htonl(counter.counterdata[ i + 8] );
				swcount->recvMCpktcount[j] = htonl(counter.counterdata[ i + 10 ] );
				swcount->recvBCpktcount[j] = htonl(counter.counterdata[ i + 11 ] );

				swcount->tranbytecount[j] = htonl(counter.counterdata[i + 24 ]);
				swcount->tranpktcount[j] = htonl(counter.counterdata[ i + 25 ] );
				swcount->tranMCpktcount[j] = htonl(counter.counterdata[ i + 26 ] );
				swcount->tranBCpktcount[j] = htonl(counter.counterdata[ i + 27 ] );
			}
			else
			{
				swcount->recvbytecount[j] = htonl(counter.counterdata[i + 7]);
				swcount->recvpktcount[j] = htonl(counter.counterdata[ i + 8 ]);
				swcount->recvMCpktcount[j] = htonl(counter.counterdata[ i + 10 ] );
				swcount->recvBCpktcount[j] = htonl(counter.counterdata[ i + 11 ] );

				swcount->tranbytecount[j] = htonl(counter.counterdata[i + 24 ]);
				swcount->tranpktcount[j] = htonl(counter.counterdata[ i + 25 ] );
				swcount->tranMCpktcount[j] = htonl(counter.counterdata[ i + 26 ]);
				swcount->tranBCpktcount[j] = htonl(counter.counterdata[ i + 27 ] );
			}

			
			logger(LOG_DEBUG, "GET COUNTERS recvbyte %u, recvpkt %u, recvmcpkt %u, recvbcpkt %u .\n", swcount->recvbytecount[j], swcount->recvpktcount[j],swcount->recvMCpktcount[j] , swcount->recvBCpktcount[j]);
			logger(LOG_DEBUG, "GET COUNTERS tranbyte %u, tranpkt %u, tranmcpkt %u, tranbcpkt %u .\n", swcount->tranbytecount[j], swcount->tranpktcount[j],swcount->tranMCpktcount[j] , swcount->tranBCpktcount[j]);
			j++;
		}

	}

	return 0;
	
	/*int  fd, i, j;
	drv_ram counter;

	if ( CTRL_CARDNO  != cardno )
		return -1;

	counter.len = 1;
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	for ( i = 0; i < 5; i++ )
	{
		if ( ( 3 == i ) || ( 4 == i ) )
		{
			counter.addr = GRBYT( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->recvbytecount[21 + i] = counter.data[0];
				statistics.recvbytecount[21 + i] += swcount->recvbytecount[21 + i];
			}

			counter.addr = GRPKT( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->recvpktcount[21 + i] = counter.data[0] & 0xFFFFFF;
				statistics.recvpktcount[21 + i] += swcount->recvpktcount[21 + i];
			}


			counter.addr = GRMCA( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->recvMCpktcount[21 + i] = counter.data[0] & 0xFFFFFF;
				statistics.recvMCpktcount[21 + i] += swcount->recvMCpktcount[21 + i];
			}

			counter.addr = GRBCA( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->recvBCpktcount[21 + i] = counter.data[0] & 0xFFFFFF ;
				statistics.recvBCpktcount[21 + i] += swcount->recvBCpktcount[21 + i];
			}

			counter.addr = GTBYT( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->tranbytecount[21 + i] = counter.data[0];
				statistics.tranbytecount[21 + i] += swcount->tranbytecount[21 + i];
			}

			counter.addr = GTPKT( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->tranpktcount[21 + i] = counter.data[0] & 0xFFFFFF ;
				statistics.tranpktcount[21 + i] += swcount->tranpktcount[21 + i];
			}

			counter.addr = GTMCA( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->tranMCpktcount[21 + i] = counter.data[0] & 0xFFFFFF ;
				statistics.tranMCpktcount[21 + i] += swcount->tranMCpktcount[21 + i];
			}

			counter.addr = GTBCA( i );
			if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
			{
				logger(LOG_ERR, "IOCTL ERR, read counters ge%d\n", i - 3);
				return -1;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, read counters ge%d, len %d, addr %x,  value %d\n", i - 3, counter.len,counter.addr, counter.data[0]);
				swcount->tranBCpktcount[21 + i] = counter.data[0] & 0xFFFFFF ;
				statistics.tranBCpktcount[21 + i] += swcount->tranBCpktcount[21 + i];
			}
			
		}
		else
		{
			for( j = 0; j < 8; j++ )
			{
				counter.addr = RBYT( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->recvbytecount[ i*8 + j ] = counter.data[0];
					statistics.recvbytecount[ i*8 + j ] += swcount->recvbytecount[ i*8 + j ];
				}

				counter.addr = RPKT( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->recvpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.recvpktcount[ i*8 + j ] += swcount->recvpktcount[ i*8 + j ] ;
				}

				counter.addr = RMCA( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->recvMCpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.recvMCpktcount[ i*8 + j ] += swcount->recvMCpktcount[ i*8 + j ] ;
				}

				counter.addr = RBCA( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->recvBCpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.recvBCpktcount[ i*8 + j ] += swcount->recvBCpktcount[ i*8 + j ] ;
				}


				counter.addr = TBYT( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->tranbytecount[ i*8 + j ] = counter.data[0];
					statistics.tranbytecount[ i*8 + j ] += swcount->tranbytecount[ i*8 + j ];
				}

				counter.addr = TPKT( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->tranpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.tranpktcount[ i*8 + j ] += swcount->tranpktcount[ i*8 + j ] ;
				}

				counter.addr = TMCA( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->tranMCpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.tranMCpktcount[ i*8 + j ] += swcount->tranMCpktcount[ i*8 + j ] ;
				}

				counter.addr = TBCA( i, j);
				if ( 0 != ioctl(fd, IOCTL_READ_RAM, &counter ) )
				{
					logger(LOG_ERR, "IOCTL ERR, read counters fe%d\n", i*8 + j + 1);
					return -1;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, read counters fe%d, len %d, addr %x,  value %d\n",  i*8 + j + 1, counter.len,counter.addr, counter.data[0]);
					swcount->tranBCpktcount[ i*8 + j ] = counter.data[0] & 0xFFFFF;
					statistics.tranBCpktcount[ i*8 + j ] += swcount->tranBCpktcount[ i*8 + j ] ;
				}

			}
		}
		
	}

	return 0;*/

}


int ReseSWCounters(unsigned char  card_no )
{
	int fd , i, j ;
	drv_dma_counters_struct counter;
	memset(&counter, 0 ,sizeof(drv_dma_counters_struct));

	if ( CTRL_CARDNO  != card_no )
		return -1;

	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	if ( 0 != ioctl(fd, IOCTL_CLEAR_DMACOUNTERS, &counter) )
	{
		logger(LOG_ERR, "IOCTL ERR GET COUNTERS ERR BY DMA.\n");
		return ERROR;
	}
	else
	{
		logger(LOG_DEBUG, " IOCTL_CLEAR_DMACOUNTERS  success!\n ");
	}
		
	return 0;

}

int GetAllVlan(unsigned char card_no )
{
	//SW_VLAN vlangrp[MAX_VLAN_NUM];
	drv_pvlan_struct tempvlan;
	int fd ,i ;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];
	
	if ( CTRL_CARDNO  != card_no )
		return -1;


	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str, "delete from VLAN");
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK)
	{
		logger ( LOG_ERR , "Database delete vlan error: %s\n", sqlite3_errmsg(db));
	 }
	
	for ( i = 0; i < MAX_VLAN_NUM; i++ )
	{
		tempvlan.vlan_id = i;
		if ( 0 != ioctl(fd, IOCTL_GET_PVLAN, &tempvlan) )
		{
			logger(LOG_ERR, "IOCTL ERR, read vlan id %d\n", i );

		}
		else
		{
			logger(LOG_DEBUG, "IOCTL SUCCESS, read vlan id %d, read value vlan id %d , bitmap %x, untag bitmap %x \n",  i, tempvlan.vlan_id, tempvlan.port_bitmap, tempvlan.ut_port_bitmap);
			// update database.
			
			sprintf(sql_str, "insert into VLAN(VlanID, utportbitmap, portbitmap) values (%d, %d, %d)",  tempvlan.vlan_id, tempvlan.ut_port_bitmap, tempvlan.port_bitmap);
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			if(rc != SQLITE_OK)
			{
				logger (LOG_ERR,  "Database insert vlan error: %s\n", sqlite3_errmsg(db));
			}
			//update cache.
			vlan_cache[i].port_bitmap = tempvlan.port_bitmap;
			vlan_cache[i].ut_port_bitmap = tempvlan.ut_port_bitmap;
			vlan_cache[i].vlan_id = tempvlan.vlan_id ;
		}
		
	}
	
	
	sqlite3_close(db);
	return 0;
}

int SaveAndDelVlan(unsigned char  card_no,SW_VLAN *vlan, unsigned char  msg_code )
{
	drv_pvlan_struct tempvlan;
	int fd;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	if ( CTRL_CARDNO != card_no )
		return -1;

	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	//open database.
	rc = sqlite3_open (CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		return ERROR;
	}
		
	if ( C_SW_DELVLAN == msg_code )
	{
		tempvlan.vlan_id = vlan->vlan_id;
		tempvlan.ut_port_bitmap = 0;
		tempvlan.port_bitmap = 0;
		tempvlan.valid = -1;

		if ( 0 != ioctl(fd, IOCTL_SET_PVLAN, &tempvlan )  )
		{
			logger(LOG_ERR, "IOCTL ERR, delete vlan id %d\n", vlan->vlan_id );
			sqlite3_close(db);
			return ERROR;

		}
		else
		{
			logger(LOG_DEBUG, "IOCTL SUCCESS ,delete vlan id success vlan id %d\n", vlan->vlan_id );
			//update database
			sprintf(sql_str ,"delete from VLAN where VlanID=%d", vlan->vlan_id );
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			if ( rc != SQLITE_OK )
			{
				logger (LOG_ERR,  "Database delete vlan error vlan id is %d: %s\n",vlan->vlan_id , sqlite3_errmsg(db));
			}

		}
		
	}
	else if ( C_SW_SAVEVLAN == msg_code )
	{
		//vlan id invalid.
		if ( ( vlan->vlan_id > MAX_VLAN_NUM ) || ( vlan->vlan_id < 1 ))
		{
			sqlite3_close(db);
			return -1;
		}

		if ( vlan->setFlag & MD_SW_VLAN_ID )
		{
			tempvlan.vlan_id = vlan->vlan_id;
		}
		else
		{
			tempvlan.vlan_id = vlan->vlan_id;//fix?
		}

		if ( vlan->setFlag & MD_SW_VLAN_UPBITMAP )
		{
			tempvlan.ut_port_bitmap = vlan->ut_port_bitmap;
		}
		else
		{
			tempvlan.ut_port_bitmap = vlan_cache[vlan->vlan_id].ut_port_bitmap ;//fix?
		}
		if ( vlan->setFlag & MD_SW_VLAN_PBITMAP )
		{
			tempvlan.port_bitmap = vlan->port_bitmap;
		}
		else
		{
			tempvlan.port_bitmap = vlan_cache[vlan->vlan_id].port_bitmap ;//fix?
		}

		if( vlan->setFlag == 0xFF )
		{
			tempvlan.vlan_id = vlan->vlan_id;
			tempvlan.port_bitmap = vlan->port_bitmap;
			tempvlan.ut_port_bitmap = vlan->ut_port_bitmap;
		}
		tempvlan.valid = -1;

		/* add cpu port in the vlan */
		tempvlan.port_bitmap |= 0x1 << 27;
		tempvlan.ut_port_bitmap |= 0x1 << 27;
		
		/*tempvlan.vlan_id = vlan->vlan_id;
		tempvlan.valid = -1;
		tempvlan.ut_port_bitmap = vlan->ut_port_bitmap;
		tempvlan.port_bitmap = vlan->port_bitmap;*/

		if ( 0 != ioctl(fd, IOCTL_SET_PVLAN, &tempvlan )  )
		{
			logger(LOG_ERR, "IOCTL ERR, save vlan id %d, portbitmap is %x, untag portbitmap is %x\n", vlan->vlan_id, vlan->port_bitmap, vlan->ut_port_bitmap );
			sqlite3_close(db);
			return ERROR;

		}
		else
		{
			logger(LOG_DEBUG, "IOCTL SUCCESS ,save vlan id success vlan id %d\n", vlan->vlan_id );
			//update database
			sprintf(sql_str ,"delete from VLAN where VlanID=%d", vlan->vlan_id );
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			if ( rc != SQLITE_OK )
			{
				logger (LOG_ERR,  "Database delete vlan error vlan id is %d: %s\n", sqlite3_errmsg(db), vlan->vlan_id );
			}

			sprintf(sql_str, "insert into VLAN(VlanID, utportbitmap, portbitmap) values (%d, %d, %d)", vlan->vlan_id, vlan->ut_port_bitmap, vlan->port_bitmap );
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg );
			if ( rc != SQLITE_OK )
			{
				logger (LOG_ERR,  "Database insert vlan error vlan id is %d: %s\n", sqlite3_errmsg(db), vlan->vlan_id );
			}

		}
	}
	
	sqlite3_close(db);
	return 0;
}


int GetSWportstate(unsigned char  card_no, SW_PORT_STATE *swport )
{
	drv_port_cfg_struct portstate;	
	int i, fd;
	
	if ( CTRL_CARDNO != card_no )
		return -1;
	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	for ( i = 0; i < 26; i++  )
	{
		portstate.pid = i + 1;
		if ( 0 != ioctl(fd, IOCTL_GET_PORT_CFG, &portstate) )
		{
			logger(LOG_ERR, "IOCTL ERR, get port state port id is  %d\n", i + 1);
			return ERROR;

		}
		else
		{
			logger(LOG_DEBUG, "IOCTL SUCCESS, get port state port id is  %d, port state is %d, port speed is %d, duplex is %d\n", i + 1, portstate.link, portstate.speed, portstate.duplex);
			swport->port_state[i].port_state = portstate.link;
			if( portstate.speed == 2)
				swport->port_state[i].portWorkingMode = ETH_1000M;
			else if( portstate.speed == 1) 
			{
				if(portstate.duplex == 1)
					swport->port_state[i].portWorkingMode = ETH_100MFULL;
				else
					swport->port_state[i].portWorkingMode = ETH_100MHALF;
			}
			else if( portstate.speed == 0) 
			{
				if(portstate.duplex == 1)
					swport->port_state[i].portWorkingMode = ETH_10MFULL;
				else
					swport->port_state[i].portWorkingMode = ETH_10MHALF;
			}
			else
				swport->port_state[i].portWorkingMode = ETH_NA;
		}
		
	}
	
	
	return 0;
}

int SetFEPort100MFull()
{
	drv_phy_reg  phy_set;	
	int i, fd;
	
	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}


	for ( i = 1; i <= 24; i++  )
	{
		phy_set.data = 0x2100;
		phy_set.addr = i;
		if ( 0 != ioctl(fd, IOCTL_WRITE_PHY_REG, &phy_set) )
		{
			logger(LOG_ERR, "IOCTL ERR, set port: %d PHY autonegotiation failed\n", i);
			return ERROR;
		}
	}	
	
	return 0;
}


int GetALLMCgroup( unsigned char card_no, SW_MC_STATE*amcg , int *tnum)
{
	int fd , i,j, t_num , mask;
	mc_group_struct mcgrp[MARL_LEN_MAX];
	unsigned char is_static[MARL_LEN_MAX];

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	memset(mcgrp, 0, sizeof(mc_group_struct)*MARL_LEN_MAX );
 	
	if ( CTRL_CARDNO != card_no )
		return -1;
	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	if ( 0 != ioctl(fd , IOCTL_GET_AMCGRP, mcgrp ) )
	{
		logger (LOG_ERR, "IOCTL  ERR :get all multicast grop err , fd is %d\n", fd );
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "IOCTL SUCCESS:get all multicast group successful\n ");

		t_num = 0x00;
		//open database.
		rc = sqlite3_open (DYNAMIC_DB, &db);
		if (rc)
		{
			logger (LOG_ERR ,  "Can't open dynamic database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}

		sprintf(sql_str, "delete from MULTICAST;");
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
		if(rc != SQLITE_OK)
		{
			logger ( LOG_ERR , "Database delete multicast error: %s\n", sqlite3_errmsg(db));
		 }
		
		//update marl cache first.
		memcpy(amcgrp_cache, mcgrp, MARL_LEN_MAX*sizeof(mc_group_struct));

		for (i = 0; i < MARL_LEN_MAX -1; i++ )
		{
			if ( 0 !=  mcgrp[i].grp_ip)
			{

				mask = 0x01;
				for ( j = 0; j < 28; j++ )
				{
					if ( mask == (mask & mcgrp[i].bitmap) )
					{
						if ( LONGLIVE_TIMEOUT_FOR_PORT == mcgrp[i].ports[j].ttl )
						{
							is_static[t_num] = 1;//static multicast group insert into database.
						} 
						else
							is_static[t_num] = 0;//dynamic multicast group.
						
					}
					
					mask = mask << 0x01;
					
				}
				
				sprintf(sql_str, "insert into MULTICAST(GrpIp, portbitmap, flag ) values(%u,%u, %u )", mcgrp[i].grp_ip , mcgrp[i].bitmap, is_static[t_num]);
				rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
				if(rc != SQLITE_OK)
				{
					logger ( LOG_ERR , "insert into  multicast error: %s\n", sqlite3_errmsg(db));
				 }
				t_num = t_num + 1;
				
			}
			
		}

		(*tnum) = t_num;
		logger(LOG_DEBUG, "total number is %d\n", t_num);
		
	}

	sqlite3_close(db);
	return 0;


}



int SaveAndDelMC(unsigned char  card_no,SW_MC_STATE *mcg, unsigned char  msg_code )
{
	drv_mcgrp_struct mcggrp;
	int i, fd,j , mask, flag = 0;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	if ( CTRL_CARDNO != card_no )
		return -1;
	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	//open database.
	rc = sqlite3_open (CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	
	for ( i = 0; i < MARL_LEN_MAX -1; i++ )
	{
		if ( mcg->grpip == amcgrp_cache[i].grp_ip )
		{
			flag = 1;
			break;
		}
	}
	logger(LOG_DEBUG, "save and del mcg flag %d, group ip %x, bitmap %x\n", flag, mcg->grpip, mcg->bitmap );
	
	if ( (0 == flag)  && ( C_SW_SAVEMCG == msg_code ))//not exist.a new multicast group.
	{
		//update amcg cache and 
		for ( i = 0 ; i < MARL_LEN_MAX - 1; i++ )
		{
			if ( 0 ==  amcgrp_cache[i].grp_ip)
			{
				break;
			}
		}
		if ( i < MARL_LEN_MAX -1 )
		{
			amcgrp_cache[i].grp_ip = mcg->grpip;
			amcgrp_cache[i].bitmap = mcg->bitmap;

			mask = 0x01;
			for ( j = 0; j < 28; j++ )
			{
				if ( mask ==  ( mask & mcg->bitmap) )//join
				{
					mcggrp.gip = mcg->grpip;
					mcggrp.pid = j;
					if ( 0 != ioctl(fd , IOCTL_JOIN_MCGRP, mcggrp ) )
					{
						logger (LOG_ERR, "IOCTL  ERR :join  multicast grop err , fd is %d, port id %d\n", fd , j);
						sqlite3_close(db);
						return ERROR;

					}
					else
					{
						logger(LOG_DEBUG,"IOCTL SUCCESS:join  multicast grop success  port id %d\n", j );
					}
					amcgrp_cache[i].ports[j].ttl = LONGLIVE_TIMEOUT_FOR_PORT;
				}
				mask = mask << 1;
			}
		}


		sprintf(sql_str, "insert into MULTICAST(GrpIp, portbitmap, flag ) values(%u,%u, %u )", mcg->grpip, mcg->bitmap , 1 );
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg );
		if ( rc != SQLITE_OK )
		{
			logger (LOG_ERR,  "Database insert MULTICAST error group id is %u: %s\n", mcg->grpip ,sqlite3_errmsg(db) );
		}
		else
		{
			logger (LOG_DEBUG,  "Database insert MULTICASTsuccessful group id is %u: %s\n", mcg->grpip ,sqlite3_errmsg(db) );
		}

		sqlite3_close(db);
		return 0;
		
	}
	else if ( (0 == flag)  && ( C_SW_DELMCG == msg_code ) )
	{
		sqlite3_close(db);
		return -1;
	}
	
	//compare .exist modify or delete?
	mask = 0x01;
	for ( j = 0; j < 28; j++)
	{
		if ( (mask & mcg->bitmap) != ( mask & amcgrp_cache[i].bitmap ))
		{
			mcggrp.gip = mcg->grpip;
			mcggrp.pid = j;
			if ( C_SW_SAVEMCG == msg_code )
			{
				if ( mask == (mask & mcg->bitmap ))//join
				{
					if ( 0 != ioctl(fd , IOCTL_JOIN_MCGRP, mcggrp ) )
					{
						logger (LOG_ERR, "IOCTL  ERR :join  multicast grop err , fd is %d, port id %d\n", fd , j);
						sqlite3_close(db);
						return ERROR;

					}
					else
					{
						logger(LOG_DEBUG,"IOCTL SUCCESS:join  multicast grop success  port id %d\n",j );
					}

				}
				else//leave.
				{
					if ( 0 != ioctl(fd , IOCTL_LEAVE_MCGRP, mcggrp ) )
					{
						logger (LOG_ERR, "IOCTL  ERR :leave  multicast grop err , fd is %d, port id %d\n", fd , j);
						sqlite3_close(db);
						return ERROR;
					}
					else
					{
						logger(LOG_DEBUG,"IOCTL SUCCESS:leave  multicast grop success  port id %d\n",j );
					}
				}
				
			}
			else if ( C_SW_DELMCG == msg_code )
			{
				if ( 0 != ioctl(fd , IOCTL_LEAVE_MCGRP, mcggrp ) )//leave
					{
						logger (LOG_ERR, "IOCTL  ERR :leave  multicast grop err , fd is %d, port id %d\n", fd , j);
						sqlite3_close(db);
						return ERROR;
					}
					else
					{
						logger(LOG_DEBUG,"IOCTL SUCCESS:leave  multicast grop success  port id %d\n", j );
					}
				
			}
			
			
		}

		mask = mask << 1;
		
	}

	sprintf(sql_str, "delete from MULTICAST where GrpIp=%u", mcg->grpip);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK)
	{
		logger ( LOG_ERR , "Database delete multicast error: %s\n", sqlite3_errmsg(db));
	 }

	if ( C_SW_SAVEMCG == msg_code )
	{
		sprintf(sql_str, "insert into MULTICAST(GrpIp, portbitmap, flag ) values(%u,%u, %u )", mcg->grpip, mcg->bitmap , 1 );
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg );
		if ( rc != SQLITE_OK )
		{
			logger (LOG_ERR,  "Database insert MULTICAST error group id is %u: %s\n", mcg->grpip ,sqlite3_errmsg(db) );
		}
	}
	
	
	//updata cache.
	if ( C_SW_SAVEMCG == msg_code )
	{
		amcgrp_cache[i].grp_ip = mcg->grpip;
		amcgrp_cache[i].bitmap = mcg->bitmap;
	}
	else if ( C_SW_DELMCG == msg_code )
	{
		amcgrp_cache[i].grp_ip = 0;
		amcgrp_cache[i].bitmap = 0;
	}
	
	sqlite3_close(db);
	return 0;

}

int GetConfigParam(unsigned char card_no, SW_CONFIG *swconfig )
{
	int fd;
	drv_igmp_parameter_struct param;
	memset(&param, 0 , sizeof(param));

	if ( CTRL_CARDNO != card_no )
		return -1;
	
	//open device
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	if ( 0 != ioctl(fd, IOCTL_GET_IGMP_PARAM, &param ) )
	{
		logger(LOG_ERR, "IOCTL ERR GET IGMP Parameter.\n");
		return ERROR;
	}
	else
	{
		logger(LOG_DEBUG, "IOCTL SUCCESS GET IGMP parameter.  %u \n", param.query_time );
		swconfig->qry_time = param.query_time;
		swconfig->qry_time = param.query_time;
		swconfig->igmp_timeout = param.timeout;
		swconfig->igmp_qportmap = param.query_ports;
		swconfig->igmp_rportmap = param.router_ports;
	}

	//update cache.
	swconfig_cache.qry_time = param.query_time;
	swconfig_cache.igmp_timeout = param.timeout;
	swconfig_cache.igmp_qportmap = param.query_ports;
	swconfig_cache.igmp_rportmap = param.router_ports;
	
	return 0;
}

int SetConfigParam(unsigned char card_no, SW_CONFIG *swconfig )
{
	int fd;
	drv_igmp_parameter_struct param;

	if ( CTRL_CARDNO != card_no )
		return -1;
	
	fd = open(BCM5645_NAME, O_RDWR );
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	if ( swconfig->setFlag & MD_SW_IGMPQRYTIME )
		param.query_time = swconfig->qry_time;
	else
		param.query_time = swconfig_cache.qry_time;
	
	if ( swconfig->setFlag & MD_SW_IGMP_TIMOUT)
		param.timeout = swconfig->igmp_timeout;
	else
		param.timeout = swconfig_cache.igmp_timeout;
	
	if ( swconfig->setFlag & MD_SW_IGMP_QPORTMAP)
		param.query_ports = swconfig->igmp_qportmap;
	else
		param.query_ports = swconfig_cache.igmp_qportmap;
	
	if ( swconfig->setFlag & MD_SW_IGMP_RPORTMAP)
		param.router_ports = swconfig->igmp_rportmap;
	else
		param.router_ports = swconfig_cache.igmp_rportmap;
	
	if ( 0 != ioctl( fd, IOCTL_SET_IGMP_PARAM, &param ) )
	{
		logger(LOG_ERR, "IOCTL SET IGMP PARAMETER ERR.\n");
		return ERROR;
	}
	else
	{
		logger(LOG_DEBUG, "IOCTL SET IGMP PARAMETER time %u, set flag %u\n", swconfig->qry_time, swconfig->setFlag );
	}

	saveIGMPParam(swconfig);
	
	return 0;

}

int GetIpAddr(char * cp, char * cp_mask, struct in_addr *ip, struct in_addr *mask)
{
	if(inet_aton(cp, ip) == 0)
		return -1;

	if(inet_aton(cp_mask, mask) == 0)
		return -1;
	
	return 0;
	
}

int ConfigVNet(SW_L3INTF *l3intf, int flag)
{
	struct ifreq ifr;
	int sockfd;			/* socket fd we use to manipulate stuff with */	
	int ret;
	struct in_addr ipaddr, mask, broadcast;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		logger(LOG_ERR, "Socket creat error!\n");
		return(ERROR_SYSTEM_GENERAL);
	}

	ret = ERROR_NO;
	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "eth%d", l3intf->id + VNET_BASE);
	if(flag == INTERFACE_DOWN)
	{
		// ifconfig ethx down
		ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
		if(ret != -1)
		{
			ifr.ifr_flags &= ~IFF_UP;		
			if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1)
				ret = ERROR_SYSTEM_GENERAL;	
		} else
			ret = ERROR_SYSTEM_GENERAL;		
	} 
	else
	{
		 /* configure interface up */
		 if(GetIpAddr(l3intf->ip, l3intf->mask, &ipaddr, &mask) == 0)
		 {
		 	struct sockaddr_in saddr;

			// ifconfig ethx ip, mask and up
			saddr.sin_family = AF_INET;
			saddr.sin_addr = ipaddr;
			memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));

			ret = ERROR_SYSTEM_GENERAL;	
			if(ioctl(sockfd, SIOCSIFADDR, &ifr) != -1) /* ip address */
			{
				saddr.sin_addr = mask;
				memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));
				if(ioctl(sockfd, SIOCSIFNETMASK, &ifr) != -1) /* net mask */
				{
					/* set broadcast addr */
					broadcast.s_addr = ipaddr.s_addr | (~mask.s_addr);
					saddr.sin_addr = broadcast;
					memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));
					ioctl(sockfd, SIOCSIFBRDADDR, &ifr); 
					
					/* bring the interface up*/
					if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) != -1)
					{
						ifr.ifr_flags |= IFF_UP;		
						if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) != -1)
							ret = ERROR_NO;	
					}
				}						
				else
					logger(LOG_ERR, "Set netmask error, %d!\n", errno);
			}
			else
				logger(LOG_ERR, "Set ip addr error, %d!\n", errno);
			
		}
		 else
			ret = ERROR_WRONG_ARGS;		

	}
	close(sockfd);
	return(ret);
}

int UpdateL3Interface(SW_L3INTF *l3intf, unsigned char  msg_code )
{
	drv_sw_l3_interface_struct drv_param;
	int fd, ret;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	//open device
	ret = ERROR_NO;
	
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return -1;
	}

	//open database.
	rc = sqlite3_open (CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		close(fd);
		return ERROR_DATABASE;
	}

	memset(&drv_param, 0, sizeof(drv_sw_l3_interface_struct));
	if ( C_SW_DELL3INTF == msg_code )
	{

		ret =  ConfigVNet(l3intf, INTERFACE_DOWN);
		if(ERROR_NO == ret)
		{
			drv_param.venet_idx = l3intf->id;

			if ( 0 != ioctl(fd, IOCTL_DEL_L3_INTF, &drv_param )  )
			{
				logger(LOG_ERR, "IOCTL ERR, delete l3 interface: %d\n", l3intf->id );
				ret= ERROR_SYSTEM_GENERAL;
			}
			else
			{
				logger(LOG_DEBUG, "IOCTL SUCCESS, delete l3 interface:%d success!\n", l3intf->id );

				//update database
				sprintf(sql_str ,"delete from L3INTF where id=%d;", l3intf->id );
				rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
				if ( rc != SQLITE_OK )
				{
					logger (LOG_ERR,  "Database delete l3 interface:%d error:%s\n", l3intf->id, sqlite3_errmsg(db));
					ret = ERROR_DATABASE;
				}
				
				//delete static route configure on this interface
				sprintf(sql_str ,"delete from StaticRoute where dev=%d;", l3intf->id );
				sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			}
		}		
	}
	else if ( C_SW_SAVEL3INTF == msg_code )
	{
		//vlan id invalid.
		if (l3intf->id > MAX_L3INTERFACE) 
			ret =  ERROR_LIMITATION;
		else 
		{
			ret =  ConfigVNet(l3intf, INTERFACE_UP);
			if(ERROR_NO == ret)
			{
				drv_param.venet_idx = l3intf->id;
				drv_param.vlan_tag = l3intf->vlan;
				if ( 0 != ioctl(fd, IOCTL_ADD_L3_INTF, &drv_param)  )
				{
					logger(LOG_ERR, "IOCTL ERR, save l3 interface:%d, vlan:%d\n", l3intf->id, l3intf->vlan);
					ret =  ERROR_SYSTEM_GENERAL;
				}
				else
				{
					logger(LOG_DEBUG, "IOCTL SUCCESS, save l3 interface:%d, vlan:%d\n", l3intf->id, l3intf->vlan);

					//update database
					sprintf(sql_str ,"delete from L3INTF where id=%d;", l3intf->id );
					rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
					if ( rc == SQLITE_OK )
					{
						sprintf(sql_str, "insert into L3INTF(id, vlan, ip, mask) values (%d, %d, '%s', '%s')", l3intf->id, l3intf->vlan, l3intf->ip, l3intf->mask);
						rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg );
						if ( rc != SQLITE_OK )
						{
							logger (LOG_ERR,  "Database insert l3 interface:%d error : %s\n", l3intf->id, sqlite3_errmsg(db));
							ret = ERROR_DATABASE;
						}
					}
					else
					{
						logger (LOG_ERR,  "Database delete l3 interface:%d error:%s\n", l3intf->id, sqlite3_errmsg(db));
						ret = ERROR_DATABASE;
					}
				}
			}
		}
	}

	sqlite3_close(db);
	close(fd);
	return (ret);
}


void InitL3Interface()
{
	int l3cap;
	drv_sw_l3_interface_struct drv_param;
	SW_L3INTF l3intf;
	int fd, ret, value;

	sqlite3 *db;
	char *zErrMsg = 0;
	char **result;
	char sql_str[256], buff[128];
	int rc, nrow, ncol, i, fldIndex ;

	//open device
	ret = ERROR_NO;
	
	fd = open(BCM5645_NAME, O_RDWR);
	if ( -1 == fd )
	{
		logger (LOG_ERR, "open bcm5645 switch err, fd is %d\n", fd);
		return;
	}
	
	if ( 0 != ioctl(fd, IOCTL_GET_L3_CAP, &l3cap ))
		return;

	if(l3cap == 0)
	{
		close(fd);
		return;
	}

	//open database.
	rc = sqlite3_open (CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		close(fd);
		return;
	}

	sprintf(sql_str, "select id, vlan, ip, mask from L3INTF;");
	sqlite3_get_table (db, sql_str, &result, &nrow, &ncol,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger ( LOG_ERR  , "select l3intf err.sql=%s,zerrmsg=%s\n",sql_str, zErrMsg );
		close(fd);
		sqlite3_close(db);
		return;
	}

	logger(LOG_DEBUG, "L3 interface initialize, nrow %d, ncol %d\n ", nrow, ncol );
	memset(&drv_param, 0, sizeof(drv_sw_l3_interface_struct));
	for ( i = 1 ; i <= nrow; i++  )
	{
		fldIndex = i*ncol;
		memset(&l3intf, 0, sizeof(SW_L3INTF));
		if ( NULL != result[fldIndex] )
		{
			strcpy(buff,  result[fldIndex]);
			sscanf(buff, "%d", &value);
			l3intf.id = value;
		}
		if ( NULL != result[fldIndex + 1] )
		{
			memset(buff, 0, sizeof(buff));
			strcpy(buff,  result[fldIndex + 1]);
			sscanf(buff, "%d", &value);
			l3intf.vlan = value;
		}
		if ( NULL != result[fldIndex + 2] )
			strncpy(l3intf.ip, result[fldIndex + 2], 15); 
		if ( NULL != result[fldIndex + 3] )
			strncpy(l3intf.mask, result[fldIndex + 3], 15); 

		if( (l3intf.id > MAX_L3INTERFACE) ||(l3intf.id < 1))
			continue;

		ret =  ConfigVNet(&l3intf, INTERFACE_UP);
		if(ERROR_NO == ret)
		{
			drv_param.venet_idx = l3intf.id;
			drv_param.vlan_tag = l3intf.vlan;
			if ( 0 != ioctl(fd, IOCTL_ADD_L3_INTF, &drv_param)  )
				logger(LOG_ERR, "L3init: interface:%d, vlan:%d, ip:%s\n", l3intf.id, l3intf.vlan, l3intf.ip);
			else
				logger(LOG_DEBUG, "L3init: interface:%d, vlan:%d, ip:%s\n", l3intf.id, l3intf.vlan, l3intf.ip);

		}			
	}

	sqlite3_free_table (result);

	// initial route 
	sprintf(sql_str, "select net, mask, dev from StaticRoute;");
	sqlite3_get_table (db, sql_str, &result, &nrow, &ncol,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger ( LOG_ERR  , "select StaticRoute err.sql=%s,zerrmsg=%s\n",sql_str, zErrMsg );
		close(fd);
		sqlite3_close(db);
		return;
	}

	for ( i = 1 ; i <= nrow; i++  )
	{
		fldIndex = i*ncol;
		if (( NULL != result[fldIndex] ) && ( NULL != result[fldIndex + 1] )  && ( NULL != result[fldIndex + 2] ))
		{
			if(strcmp("0",  result[fldIndex + 1])== 0)
				sprintf(buff, "/sbin/route add default gw %s", result[fldIndex] );
			else
			{
				/*get dev*/
				memset(buff, 0, sizeof(buff));
				strcpy(buff,  result[fldIndex + 2]);
				sscanf(buff, "%d", &value);
				
				sprintf(buff, "/sbin/route add -net %s netmask %s dev eth%d", result[fldIndex], result[fldIndex+1], value+2);
			}
			ret = system(buff);
			if(ret != 0)
				logger(LOG_ERR, "L3init: init route table error:net=%s, mask=%s, dev=%d\n",  result[fldIndex], result[fldIndex+1], value);
		}
		else 
			logger(LOG_ERR, "L3init: init route database error\n");

	}

	sqlite3_free_table (result);

	sqlite3_close(db);
	close(fd);
}


