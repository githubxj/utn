#include <iostream>
#include <stdio.h>
#include <sstream> 
#include <dirent.h>
#include <string.h>

#include "CgiRequest.h"
#include "Tools.h"
#include "Lang.h"
#include "CGISession.h"
#include "CppSQLite3.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"

#include "CChgip.h"

using namespace std;

int CChgip::ChgIPAddress()
{

//	std::string old_ip = getReqValue("oldip");
//	std::string new_ip1 = getReqValue("ipnew1");
//	std::string new_ip2 = getReqValue("ipnew2");
//	std::string new_ip3 = getReqValue("ipnew3");
//	std::string new_ip4 = getReqValue("ipnew4");

	/*compare the oldip with the  newip ,*/
	//ifconfig eth0 192.168.1.177
	///etc/sysconfig/network-scripts/ifcfg-eth0 reboot .
	//but how to jump to relogin.html?
	/*char str_mac[256];
	sprintf (str_mac, "/sbin/ifconfig eth0 %d.%d.%d.%d" , atoi(getReqValue("newip1").c_str()), atoi(getReqValue("newip2").c_str()) ,atoi(getReqValue("newip3").c_str()),atoi(getReqValue("newip4").c_str()));
	int i = system (str_mac);
	if (i >= 0)
	{
		return 0;
	}else return ERROR_SYSTEM_GENERAL;*/
	/*this code will change the ipconfig file.*/
	FILE *fp;
	char src[MAX_LEN][MAX_LEN];
	char des[MAX_LEN][MAX_LEN];
	char tmp[MAX_LEN][MAX_LEN];
	CTools t;
	int i = 0, line = 0;
	ip = atoi(getReqValue("newip").c_str());
	
	
	if (NULL == (fp = fopen (IP_CONFIG, "r+w")))
		return ERROR_SYSTEM_GENERAL;
	while( fgets(src[i], sizeof(src[i]), fp))
	{
		strncpy(tmp[i], src[i], sizeof(src[i]));
		if ( strstr (src[i], IPADDR_SECTION))
		{
			memset (des[i], 0, sizeof(des[i]));
			t.str_replace(src[i], MAX_LEN, "\n", "");
			t.str_replace(src[i], MAX_LEN, " ", "");
			sprintf (des[i], "%s%s\n", IPADDR_SECTION, getReqValue("newip"));

		}else {
			
			memset (des[i], 0, sizeof(des[i]) );
			memcpy (des[i], tmp[i], sizeof(tmp[i]) );

		}
		line = i++;
	}

	rewind(fp);
	for ( i = 0; i <= line; i++)
		fputs(des[i], fp);
	fclose (fp);

	
	return ERROR_NO;

}







void CChgip::output()
{
	std::string result;
	CCGISession* cs;	

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	
	if(outputHTMLFile("ip.html", true) != 0)
	{  
			outputError("Cann't open ip.html\n");
	}
	int ret = ChgIPAddress();
	 usleep(50000);
	
	 char str_mac[256];
	sprintf (str_mac, "/sbin/ifconfig eth0 %s" ,ip);
	 system (str_mac);


	// result = multilang::getError(ret);

	//utputError(result);
	free(cs);

}
