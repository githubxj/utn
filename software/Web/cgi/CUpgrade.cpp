#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

#include "CGISession.h"
#include "Tools.h"
#include "const.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "Comm.h"

#include "CUpgrade.h"

#define  UPGRADE_TMPPATH	"/tmp/upgrade"
#define  VERSION_INFO		"/tmp/upgrade/version.info"
#define  PACKAGE_NAME		"/tmp/upgrade/upgrade.tgz"
#define  CHECK_SUM			"/tmp/upgrade/check.sum"
#define  CHECK_SUM_TMP		"/tmp/upgrade/check.sum.tmp"
#define  UPLOAD_FILENAME 	"newversion"


/***************************************
upgrade return value for Javascript:
0 - OK
-1 <error info>

prog return value for Javascript:
0~100 progress in percentage
101 upgrade complete
102 upgrade interrupted
103 upgrade failed with check sum error
104 upgrade failed with other reason
***************************************/

using namespace std;

int CUpgrade::do_update(const char* filename)
{    
	char cmd[1024] = {0};  
	
	snprintf(cmd, sizeof(cmd),
		"cd /updtmp && rm -fr upgrade && tar xzf %s%s && cd upgrade && chmod +x update.sh && ./update.sh", upload_path.c_str(), filename);    

	return(system(cmd));    	  
}

void CUpgrade::output()
{
	std::string action = getReqValue("act");
	CCGISession* cs;
	
	/* check login */
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		delete(cs);
		return;
	} 
	delete(cs);
	
	if(action == "upgrade") {
		//do software upgrade
		std::string src_file = getReqUploadFilename("newversion");
		
		/*
		int len = getReqUploadFileLen("newversion");
		char buff[256];
		sprintf(buff, "File:%s, Len=%d\n", src_file.c_str(), len);
		outputError(buff);
		*/
		
		if(src_file != "")
		{
			if(do_update(src_file.c_str()))
				outputError(multilang::getError(ERROR_UP_ERROR));
			else if(outputHTMLFile("reboot.html", true) != 0)
				outputError("Cann't open reboot.html\n");

			fclose(stdout);
		}
		else
			outputError(multilang::getError(ERROR_UP_FILE));
		return;
	} else {
		//clear upload directory
		system("rm -fr /updtmp/*");
		if( outputHTMLFile("sysinfo.html", true) != 0) {
				outputError("Cann't open sysinfo.html\n");
		}	

	}

}


