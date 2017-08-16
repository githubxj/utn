#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"

#include "CClearAlarm.h"

using namespace std;

int CClearAlarm::ClearAlarm()
{

	CppSQLite3DB db;



	db.open(DYNAMIC_DB);

	CppSQLite3Query q = db.execQuery("delete from Alarm;");

		
	
	db.close();
	
	return ERROR_NO;


}




void CClearAlarm::output()
{
	CCGISession* cs;	
	std::string result;
	std::string action = getReqValue("act");

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	if ( action == "clear") 
	{
		int ret = ClearAlarm();
	
		result = multilang::getError(ret);

		outputError(result);
	} else if( outputHTMLFile("clearalarm.html", true) != 0) 
		outputError("Cann't open clearalarm.html\n");
	
	free(cs);


}
