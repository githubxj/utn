#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CSnmpconfig.h"
#include "iniFile.h"
using namespace std;

int CSnmpconfig::getConfigFile()
{
	struct stat conf_stat;
	IniFilePtr confFile;
	const char *var;
	int i;
	char keyname[16];

	confFile = iniFileOpen(SNMP_AGENT_CONFIG);
	if (confFile == 0)
		return ERROR_SYSTEM_GENERAL;

	var = iniGetStr (confFile, "sys", "sysContact");
	if ( var != NULL )
	{

		addUserVariable("sys_contact" ,  var);
	}

	var = iniGetStr ( confFile, "sys", "sysName");
	if ( var != NULL )
	{
		addUserVariable( "sys_name", var);
	}

	var = iniGetStr ( confFile, "sys", "sysLocation");
	if ( var != NULL )
	{
		addUserVariable( "sys_location", var);
	}

	/*var = iniGetStr ( confFile, "trap", "community");
	if ( var != NULL )
	{
		addUserVariable( "snmp_community", var);
	}*/
	
	
	char fieldname[32];
	char buf[32];
	std::string s = "0";
	for ( i = 1; i < 5; i++)
	{
		sprintf ( keyname, "trap%d", i);
		var = iniGetStr(confFile, "trap", keyname);
		sprintf (fieldname, "trap_host%d", i);
		if (var == NULL || var[0] == '\0' ||var[0] == '\r' || var[0] == '\n')
				{ addUserVariable(fieldname , s); }
		else
		{
			sprintf ( buf, "%s", var);
			addUserVariable( fieldname, buf);
		}
		
	}

	
	iniFileFree( confFile);

	std::ifstream config_file(SNMPD_CONFIG, std::ios::in);
	if ( !config_file)
		return ERROR_SYSTEM_GENERAL;

	while ( config_file && !config_file.eof())
	{
		char buffer[2048];
		config_file.getline( buffer, sizeof (buffer));
		if ( buffer[0] == '#')
			continue;
		std::string content = std::string(buffer);
		std::string::size_type start;
		start = content.find( "rocommunity" , 0);
		
		if ( start != std::string::npos)
		{
			content.replace ( start, strlen("rocommunity"), "");
			content.erase(0, content.find_first_not_of(" \t\n\r") );
			content.erase( content.find_last_not_of(" \t\n\r")+1 );
			addUserVariable("ro_community", content);
		}

		start = content.find( "rwcommunity" , 0);
		if ( start != std::string::npos)
		{
			content.replace ( start, strlen("rwcommunity"), "");
			content.erase(0, content.find_first_not_of(" \t\n\r") );
			content.erase( content.find_last_not_of(" \t\n\r")+1 );
			addUserVariable("rw_community", content);
		}
		
		
	}
	config_file.close();
	
	m_desthtml = "snmpconfig.html" ;
	return 0;


}

int CSnmpconfig::conf_save(const char * section, const char * keyname, const char * value)
{
	IniFilePtr confFile;
	char *var;
	int ret;

	confFile = iniFileOpen( SNMP_AGENT_CONFIG);
	if ( confFile == 0)
		return ERROR_SYSTEM_GENERAL;
	iniSetStr( confFile, section, keyname, value);
	ret = iniFileSave( confFile);
	iniFileFree( confFile);
	return ret;
	

}
int CSnmpconfig::setConfigFile()
{
	char fieldname[32];
	char buf[32];
	int i;
	
	if ( conf_save("sys", "sysContact",getReqValue( "sys_contact" ).c_str() ))
		return ERROR_SYSTEM_GENERAL;
	if ( conf_save("sys", "sysName", getReqValue( "sys_name" ).c_str() ))
		return ERROR_SYSTEM_GENERAL;
	if ( conf_save("sys", "sysLocation", getReqValue( "sys_location" ).c_str()))
		return ERROR_SYSTEM_GENERAL;
	if ( conf_save("sys", "trap", getReqValue( "snmp_community" ).c_str()))
		return ERROR_SYSTEM_GENERAL;
	
	for (i = 1; i < 5; i++)
	{
		sprintf (fieldname, "trap_host%d", i);
		sprintf ( buf , "trap%d" ,i);
		if ( conf_save("trap", buf, getReqValue( fieldname).c_str()))
		return ERROR_SYSTEM_GENERAL;

	}

	std::ifstream config_file(SNMPD_CONFIG);
	std::ofstream chg_config_file(CHG_SNMPD_CONFIG);
	if ( !config_file)
		return ERROR_SYSTEM_GENERAL;

	while ( config_file && !config_file.eof())
	{
		char buffer[2048];
		config_file.getline( buffer, sizeof (buffer));
		if ( buffer[0] == '#')
		{
			chg_config_file<<buffer<<"\n";
			continue;
		}
		std::string content = std::string(buffer);
		std::string::size_type pos1, pos2, pos3, pos4;
		pos1 = content.find( "rocommunity" , 0);
		pos2 = content.find( "rwcommunity" , 0);
		pos3 = content.find("rouser", 0);
		pos4 = content.find("rwuser",  0);
		if ( pos1 != std::string::npos)
		{
			chg_config_file << "rocommunity  " << getReqValue( "ro_community" ).c_str() <<"\n";
		}
		
				
		else if ( pos2 != std::string::npos)
		{	
			chg_config_file<<"rwcommunity "<<getReqValue( "rw_community" ).c_str() <<"\n";
			
		}
		else if (pos3 != std::string::npos)
		{
			chg_config_file << "rouser  " << getReqValue( "ro_community" ).c_str() <<"\n";
			
		}
		else if (pos4 != std::string::npos)
		{
			chg_config_file<<"rwuser  "<<getReqValue( "rw_community" ).c_str() <<"\n";
			
		}
		else {
			chg_config_file<<buffer<<"\n";
		}
				
		
	}
	config_file.close();
	chg_config_file.close();
	remove(SNMPD_CONFIG);
	rename(CHG_SNMPD_CONFIG,  SNMPD_CONFIG);
	
	return ERROR_NO;

	
}

void CSnmpconfig::output()
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
			ret = setConfigFile();
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

			ret = getConfigFile();
			if(ret == 0) 
			{
				if(outputHTMLFile(m_desthtml, true) != 0)
				{
					outputError("Cann't open snmpconfig.html\n");
				}	
			} 
			else 
				outputError(multilang::getError(ret));

	}
	

	free(cs);

}

