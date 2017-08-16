#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string.h>

#include "CgiRequest.h"
#include "Tools.h"
#include "Lang.h"
#include "CGISession.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"

#include "CReboot.h"

using namespace std;

int CReboot::reboot()
{
	//CCGISession *cs;
	//cs = new CCGISession();
	
	int i = system("/sbin/reboot");
	//if ( i >= 0)
	//{
		//outputHTMLFile("/relogin.html", true);	
		//free(cs);
		//return 0;
	//}else return ERROR_SYSTEM_GENERAL;
}



void CReboot::output()
{

	std::string result;
	CCGISession *cs;
	std::string action = getReqValue("act");
	
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	if ( action == "reboot") {
		if(outputHTMLFile("reboot.html", true) != 0)
		{  
				outputError("Cann't open reboot.html\n");
		}	
		int ret = reboot();
	} else if( outputHTMLFile("systemreboot.html", true) != 0) 
		outputError("Cann't open systemreboot.html\n");		

	free(cs);

}
