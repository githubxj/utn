#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CSysdate.h"
using namespace std;

int CSysdate::getDate()
{
	time_t 	now;
	struct tm *now_tm;
	char buffer[32];

	time(&now);
	
       now_tm = localtime(&now);
 	if(now_tm != NULL)
 	{
 		sprintf(buffer, "%d", now_tm->tm_year+1900);		
		addUserVariable("cur_year", buffer);
 		sprintf(buffer, "%d", now_tm->tm_mon + 1);		
		addUserVariable("cur_mon", buffer);
 		sprintf(buffer, "%d", now_tm->tm_mday);		
		addUserVariable("cur_day", buffer);
		
 		sprintf(buffer, "%02d", now_tm->tm_hour);		
		addUserVariable("cur_hour", buffer);
 		sprintf(buffer, "%02d", now_tm->tm_min);		
		addUserVariable("cur_min", buffer);
 		sprintf(buffer, "%02d", now_tm->tm_sec);		
		addUserVariable("cur_sec", buffer);

		return 0;
 	}
	else
		return -1;

}

int CSysdate::setDate()
{
	int year, mon, day, hour, min, sec;
	char buffer[256];
	
	year = std::atoi(getReqValue( "year").c_str());
	if(year < 1970 || year > 2038)
		return ERROR_DATETIME_FORMAT;
	mon = std::atoi(getReqValue( "mon").c_str());
	if(mon < 1 || mon > 12)
		return ERROR_DATETIME_FORMAT;
	day = std::atoi(getReqValue( "day").c_str());
	if(day < 1 || day > 31)
		return ERROR_DATETIME_FORMAT;
	hour = std::atoi(getReqValue( "hour").c_str());
	if(hour < 0 || hour > 23)
		return ERROR_DATETIME_FORMAT;
	min = std::atoi(getReqValue( "min").c_str());
	if(min < 0 || min > 59)
		return ERROR_DATETIME_FORMAT;
	sec = std::atoi(getReqValue( "sec").c_str());
	if(sec < 0 || sec > 59)
		return ERROR_DATETIME_FORMAT;

	outputError(multilang::getError(ERROR_NO));
	fclose(stdout);

	sprintf(buffer, "/mnt/utn/bin/rtc_time -s %d-%d-%d %d:%d:%d", year, mon, day, hour, min, sec);
	system(buffer);
//	system("/sbin/hwclock --systohc");

//		m_debug << std::string(buffer) ;
	
	return 0;	
}

void CSysdate::output()
{
	CCGISession* cs;	
	std::string action = getReqValue("action");
	std::string result;
	int ret;
	
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 
	
	if ( action == "set")
	{
		ret = setDate();
		result = multilang::getError(ret);
		outputError(result);
	}
	else{	

		ret = getDate();
		if(ret == 0) 
		{
				if(outputHTMLFile("sysdate.html", true) != 0)
				{
					outputError("Cann't open sysdate.html\n");
				}	
			} 
			else 
				outputError(multilang::getError(ret));

	}
	

	free(cs);

}

