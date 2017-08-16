#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "iniFile.h"

#define LOG_CONF_FILE		"/var/agent_log.conf"
//#define LOG_CONF_FILE		"/mnt/mtd/etc/agent_log.conf"

/*********************************************************
log config file format: 
[log]
level=7
msg_485=1
msg_fifo=1
debug_fpga=1
alarm=1
rstcp=1
svc=1
***********************************************************/

time_t	conf_file_mtime=-1;

int dump485;
int dumpFifo;
int debugFpga;
int alarmlog;
int rstcplog;
int svclog;

/*LOG_ERR: 3, LOG_WARNING: 4, LOG_NOTICE: 5, LOG_INFO: 6, LOG_DEBUG: 7 */
int log_level=LOG_NOTICE;

void logger(int level, const char *fmt, ...) 
{
        va_list args;

	 if(level >  log_level)
	 	return;

	 va_start(args, fmt);
	 vsyslog(level, fmt, args);
        va_end(args);
}

void DumpMsg(unsigned char *buffer, int len)
{	
	char tmp[8];
	char hexmsg[50];
	char text[17];
	int i,index;

//	if(log_level < LOG_DEBUG)
//		return; 
	
	for(i =0; i< len; i++)
	{
		sprintf(tmp,"%02x", buffer[i]);
		
		index = i%16;		
		if( buffer[i] >= 0x20 && buffer[i] < 0x7f)
			text[index] = buffer[i];
		else 
			text[index] = '.';

		index = index * 3;
		hexmsg[index] = tmp[0];
		hexmsg[index + 1] = tmp[1];
		hexmsg[index + 2] = ' ';
		
		if((i+1) % 16 == 0) 
		{
			text[16] = '\0';
			hexmsg[48] = '\0';
			syslog(LOG_DEBUG, "%s %s\n", hexmsg, text);	
		}
	}

	if( i%16 != 0)
	{	
		text[i%16] = '\0';

		for(;i%16 != 0; i++)
		{
			index = (i%16) * 3;		
			hexmsg[index] = ' ';
			hexmsg[index + 1] = ' ';
			hexmsg[index + 2] = ' ';
		}
			
		hexmsg[48] = '\0';
		syslog(LOG_DEBUG, "%s %s\n", hexmsg, text);	
	}

}

void logger_reload()
{
	struct stat conf_stat;
	IniFilePtr confFile;
	int varInt;

	/* check config modified first */
	if(stat(LOG_CONF_FILE, &conf_stat) !=0)
	{
		fprintf(stderr, "log configure file does not exist, use default.\n");
		dumpFifo = 0;
		dump485 = 0;
		debugFpga = 0;
		alarmlog=0;
		rstcplog=0;
		svclog = 0;
		log_level=LOG_NOTICE;
		return;
	}

	if(conf_file_mtime == conf_stat.st_mtime)
		return;

	conf_file_mtime = conf_stat.st_mtime;

	fprintf(stderr, "Log Configure file reload.\n");

	confFile = iniFileOpen(LOG_CONF_FILE);
	if(confFile == 0)
	{
		fprintf(stderr, "Log configure file error\n");
		return;
	}

	varInt = iniGetInt(confFile, "log", "level");
	if(varInt != 0 )
		log_level = varInt;

	varInt = iniGetInt(confFile, "log", "msg_fifo");
	if(varInt == 0 )
		dumpFifo = 0;
	else 
		dumpFifo = varInt;
	
	varInt = iniGetInt(confFile, "log", "debug_fpga");
	if(varInt == 0 )
		debugFpga = 0;
	else 
		debugFpga = 1;
	
	varInt = iniGetInt(confFile, "log", "msg_485");
	if(varInt == 0 )
		dump485 = 0;
	else 
		dump485 = 1;
	
	varInt = iniGetInt(confFile, "log", "alarm");
	if(varInt == 0 )
		alarmlog = 0;
	else 
		alarmlog = 1;
	
	varInt = iniGetInt(confFile, "log", "rstcp");
	if(varInt == 0 )
		rstcplog = 0;
	else 
		rstcplog = 1;
		
	varInt = iniGetInt(confFile, "log", "svc");
	if(varInt == 0 )
		svclog = 0;
	else 
		svclog = 1;
	iniFileFree(confFile);
	
}




