#include <iostream>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"

#include "CLogin.h"

using namespace std;

int CLogin::checkLogin(const char * user, const char * pwd)
{
	FILE* fp;
    	char src[MAX_LEN];
	CTools t;

	if ((fp = fopen(WEB_CONF_FILE, "r+")) == NULL)
		return ERROR_SYSTEM_GENERAL;
	
    	if(strcmp(user, DEF_USERNAME))
		return ERROR_LOGIN_FAIL;

	while (fgets(src, sizeof(src), fp))
	{
		t.str_replace(src, MAX_LEN, "\n", "");
		t.str_replace(src, MAX_LEN, " ", "");
        
		if(strstr(src, PASSWD_SECTION))  
		{
            		if(strcmp(pwd, rindex(src, '=')+1) == 0) 
			{
				fclose(fp);
				return 0; 
			} else
				break;
        	}
    	}

	fclose(fp);
	return ERROR_LOGIN_FAIL; 
}

void CLogin::output()
{
	std::string login_name, login_passwd;
	CCGISession* cs;
	bool login_success = false;

	login_name = getReqValue("user");
	login_passwd = getReqValue("pwd");

	if(login_name != "") 
	{
		int ret = checkLogin(login_name.c_str(), login_passwd.c_str());
		if(ret == 0) {
			cs = new CCGISession(getRemoteAddr().c_str());  
			if(cs->IsExistIPForSession(getRemoteAddr().c_str()) == IPEXIST)
				login_success = true;
			else if(!(ret=cs->CreateSession()))
			{	
				login_success = true;
			} else {
				addUserVariable("error", multilang::getError(ERROR_SYSTEM_GENERAL));
			}
		} else {
			addUserVariable("error", multilang::getError(ret));
		}

	}

	if(login_success) {
		addUserVariable("sessionID", cs->GetSessionID());

		//login success, show the main html file
 		if( outputHTMLFile("main.html", true) != 0)
			outputError("Cann't open main.html\n");
		
	}
	else if( outputHTMLFile("login.html", true) != 0)
	{
		outputError("Cann't open login.html\n");
	}	
    

}

