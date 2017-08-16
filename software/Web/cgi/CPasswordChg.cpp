#include <iostream>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CPasswordChg.h"

using namespace std;

int CPasswordChg::updatePwd(const char * oldpwd, const char * newpwd)
{
	FILE* fp;
    	char src[MAX_LEN][MAX_LEN];
    	char des[MAX_LEN][MAX_LEN];
	char tmp[MAX_LEN][MAX_LEN];
	CTools t;
 	int i = 0, line = 0;

	if ((fp = fopen(WEB_CONF_FILE, "r+w")) == NULL)
		return ERROR_SYSTEM_GENERAL;
	
	while (fgets(src[i], sizeof(src[i]), fp))
	{
		strncpy(tmp[i], src[i], sizeof(src[i]));
		if (strstr(src[i], PASSWD_SECTION))
		{
			memset(des[i], 0, sizeof(des[i]));

			t.str_replace(src[i], MAX_LEN, "\n", "");
			t.str_replace(src[i], MAX_LEN, " ", "");
			if(strcmp(oldpwd, rindex(src[i], '=')+1))
				return ERROR_WRONG_PWD;
			else
				sprintf(des[i], "%s%s\n", PASSWD_SECTION, newpwd);
		} else {
			memset(des[i], 0, sizeof(des[i]));
			memcpy(des[i], tmp[i], sizeof(tmp[i]));
		}

		line = i++;
	}

	rewind(fp);
	for (i = 0; i <= line; i++)
		fputs(des[i], fp); 

	fclose(fp);
	return ERROR_NO; 
}

void CPasswordChg::output()
{
	std::string oldpwd, newpwd, result;
	CCGISession* cs;	
	std::string action = getReqValue("act");

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	if ( action == "chgpwd") {
		oldpwd = getReqValue("oldpwd");
		newpwd = getReqValue("newpwd");

		int ret = updatePwd(oldpwd.c_str(), newpwd.c_str());

		result = multilang::getError(ret);

		outputError(result);
	} else if( outputHTMLFile("password.html", true) != 0) 
		outputError("Cann't open password.html\n");		

	free(cs);
}


