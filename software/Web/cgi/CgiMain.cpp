#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "const.h"
#include "msg.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"

#include "CSidemenu.h"
#include "CFrameView.h"
#include "CTopoView.h"
#include "CGetCard.h"
#include "CGetOnu.h"
#include "CLogin.h"
#include "CPasswordChg.h"
#include "CGetGe8.h"

#include "CCardChg.h"
#include "CPerf.h"
#include "COnuChg.h"
#include "CAlarm.h"
#include "CClearAlarm.h"
#include "CNetconfig.h"
#include "CReboot.h"
#include "CSnmpconfig.h"
#include "CSysdate.h"
#include "CGetSfp.h"
#include "CRefreshCheck.h"
#include "CCount.h"
#include "CVlan.h"
#include "CMulticast.h"
#include "CONUIDmgmt.h"
#include "CL3Config.h"
#include "CRoute.h"
#include "CFsaCard.h"
#include "CNttPerfView.h"
#include "CEpnView.h"
#include "CGetEPOnu.h"
#include "CEPOnuChg.h"
#include "CHDCard.h"
#include "CUpgrade.h"

int main(int argc, char *argv[])
{
	char cginame[256];
	int i, index;
	
	CgiRequest * myCGI = NULL;

	/* get the base file name of execution*/ 
	index = 0;
	for(i=0; i<255; i++)
	{
		if( argv[0][i] == '\0' || argv[0][i] == '.' )
			break;
		
		cginame[index] = argv[0][i];
		if(cginame[index] == '/')
			index = 0;
		else 
			index++;
	}
	
	cginame[index] = '\0';

	if(strcmp(cginame, "sidemenu") == 0)
		myCGI = (CgiRequest *) new CSidemenu();
	else if(strcmp(cginame, "frameview") == 0)
		myCGI = (CgiRequest *) new CFrameView();
	else if(strcmp(cginame, "topoview") == 0)
		myCGI = (CgiRequest *) new CTopoView();
	else if(strcmp(cginame, "show_card") == 0)
		myCGI = (CgiRequest *) new CGetCard();
	else if(strcmp(cginame, "show_ge8") == 0)
		myCGI = (CgiRequest *) new CGetGE8();
	//else if(strcmp(cginame, "chgge8") == 0)
		//myCGI = (CgiRequest *) new CGE8Chg();
	else if(strcmp(cginame, "show_onu") == 0)
		myCGI = (CgiRequest *) new CGetOnu();
	else if(strcmp(cginame, "login") == 0)
		myCGI = (CgiRequest *) new CLogin();
	else if(strcmp(cginame, "chgpwd") == 0)
		myCGI = (CgiRequest *) new CPasswordChg();

	else if(strcmp(cginame, "chgcard") == 0)	
		myCGI = (CgiRequest *) new CCardChg();
	else if(strcmp(cginame,"perfview")==0)
		myCGI = (CgiRequest *) new CGetPerf();
	else if(strcmp(cginame,"nttperfview")==0)
		myCGI = (CgiRequest *) new CNttPerf();
	else if(strcmp(cginame,"chgonu")==0)
		myCGI = (CgiRequest *) new COnuChg();
	else if(strcmp(cginame,"getalarm")==0)
		myCGI = (CgiRequest *) new CAlarm();
	else if(strcmp(cginame,"clearalarm")==0)
		myCGI = (CgiRequest *) new CClearAlarm();
	else if(strcmp(cginame,"netconfig")==0)
		myCGI = (CgiRequest *) new CNetconfig();
	else if (strcmp(cginame, "reboot")==0)
		myCGI = (CgiRequest *)new CReboot();
	else if (strcmp(cginame, "snmpconfig")==0)
		myCGI = (CgiRequest *)new CSnmpconfig();
	else if (strcmp(cginame, "sysdate") == 0)
		myCGI = (CgiRequest *) new CSysdate();
	else if (strcmp(cginame, "sfpinfo" ) == 0)
		myCGI = (CgiRequest *)new CGetSfp();
	else if (strcmp(cginame, "refrchk" ) == 0)
		myCGI = (CgiRequest *)new CRefreshCheck();
	else if (strcmp(cginame, "countview" ) == 0)
		myCGI = (CgiRequest *)new CGetCount();
	else if (strcmp(cginame, "vlan" ) == 0)
		myCGI = (CgiRequest *)new CVlan();
	else if (strcmp(cginame, "multicast" ) == 0)
		myCGI = (CgiRequest *)new CMulticast();
	else if (strcmp(cginame, "onuid" ) == 0)
		myCGI = (CgiRequest *)new CONUIDmgmt();
	else if (strcmp(cginame, "l3config" ) == 0)
		myCGI = (CgiRequest *)new CL3Config();
	else if (strcmp(cginame, "route" ) == 0)
		myCGI = (CgiRequest *)new CRoute();
	else if (strcmp(cginame, "fsacard" ) == 0)
		myCGI = (CgiRequest *)new CFsaCard();
	else if (strcmp(cginame, "show_eponu" ) == 0)
		myCGI = (CgiRequest *)new CGetEPOnu();
	else if (strcmp(cginame, "epnview" ) == 0)
		myCGI = (CgiRequest *)new CEpnView();
	else if(strcmp(cginame,"chgeponu")==0)
		myCGI = (CgiRequest *) new CEPOnuChg();
	else if(strcmp(cginame,"hdcard")==0)
		myCGI = (CgiRequest *) new CHDCard();
	else if(strcmp(cginame,"upgrade")==0)
		myCGI = (CgiRequest *) new CUpgrade();

	multilang::init();	

	if(myCGI != NULL )
		myCGI->output();
	else
	{
		myCGI = new CgiRequest();
		char buffer[256];

		sprintf(buffer, "ERROR!!! Unknown CGI: %s!", cginame);
		myCGI->outputError(buffer);
	}
	
	free(myCGI);
	
	return 0;
}

