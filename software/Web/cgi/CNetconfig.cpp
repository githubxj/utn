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

#include "CNetconfig.h"

using namespace std;

int CNetconfig::ChgNetConfig()
{


	/*this code will change the ipconfig file.*/
	FILE *fp;
	char src[MAX_LEN][MAX_LEN];
	char des[MAX_LEN][MAX_LEN];
	char tmp[MAX_LEN][MAX_LEN];
	CTools t;
	int i = 0, line = 0;
	
	ip = getReqValue("ip").c_str();
	mask = getReqValue("mask").c_str();
	gate = getReqValue("gateway").c_str();
	
	
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
			sprintf (des[i], "%s%s\n", IPADDR_SECTION, ip);

		}

		else if(strstr (src[i], NET_MASK))
		{


			memset (des[i], 0, sizeof(des[i]));
			t.str_replace(src[i], MAX_LEN, "\n", "");
			t.str_replace(src[i], MAX_LEN, " ", "");
			sprintf (des[i], "%s%s\n", NET_MASK, mask);

		}
		else if( strstr (src[i], GATE_WAY))
		{


			memset (des[i], 0, sizeof(des[i]));
			t.str_replace(src[i], MAX_LEN, "\n", "");
			t.str_replace(src[i], MAX_LEN, " ", "");
			sprintf (des[i], "%s%s\n", GATE_WAY, gate);

		}

		else {
			
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


int CNetconfig::getNetConfig()
{

	std::ifstream config_file(IP_CONFIG, std::ios::in);
	if ( !config_file)
		return ERROR_SYSTEM_GENERAL;

	while ( config_file && !config_file.eof())
	{
		char buffer[2048];
		config_file.getline( buffer, sizeof (buffer));
		
		std::string content = std::string(buffer);
		std::string::size_type start;
		start = content.find( "IPADDR=" , 0);
		
		if ( start != std::string::npos)
		{
			content.replace ( start, strlen("IPADDR="), "");
			content.erase(0, content.find_first_not_of(" \t\n\r") );
			content.erase( content.find_last_not_of(" \t\n\r")+1 );
			addUserVariable("ip", content);
		}

		start = content.find( "NETMASK=" , 0);
		if ( start != std::string::npos)
		{
			content.replace ( start, strlen("NETMASK="), "");
			content.erase(0, content.find_first_not_of(" \t\n\r") );
			content.erase( content.find_last_not_of(" \t\n\r")+1 );
			addUserVariable("mask", content);
		}

		start = content.find( "GATEWAY=" , 0);
		if ( start != std::string::npos)
		{
			content.replace ( start, strlen("GATEWAY="), "");
			content.erase(0, content.find_first_not_of(" \t\n\r") );
			content.erase( content.find_last_not_of(" \t\n\r")+1 );
			addUserVariable("gateway", content);
		}
		
		
	}
	config_file.close();

}




void CNetconfig::output()
{
	std::string result;
	CCGISession* cs;	
	
	std::string action = getReqValue("action");
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
			ret = ChgNetConfig();
			result = multilang::getError(ret);
			if(ret == ERROR_NO)
			{
				result += "<br><p>";
				result += multilang::getLabel("reboot_inform");
				result += "</p>";
			}
			outputError(result);
		
	}
	
	else{
			
			ret = getNetConfig();
			if(ret == 0) 
			{
				if(outputHTMLFile("netconfig.html", true) != 0)
				{
					outputError("Cann't open netconfig.html\n");
				}	
			} 
			else 
				outputError(multilang::getError(ret));

	}
	




	free(cs);

}
