#include <iostream>
#include <stdio.h>
#include <map>
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

#include "CRoute.h"

using namespace std;


void CRoute::newRoute(stringstream & outss)
{
	CppSQLite3DB db;
	char 	sql[256];

	try {
		db.open(CONF_DB);

		// get the interface name
		sprintf(sql, "select id, ip from L3INTF;");
		CppSQLite3Query q = db.execQuery(sql);

		outss << "<select class='fixsize'  name='intf_id'>";
		
		while (!q.eof())
        	{	
			outss << " <option value='" << q.getIntField(0) << "'>" << q.getStringField(1) << "</option>";
			q.nextRow();
		}
		
		outss << "</select>";		
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();

}


void CRoute::getRouteList(stringstream & outss, stringstream & outss2)
{
	CppSQLite3DB db;
	char 	sql[256];
	int id, dev, default_gw_flag=0;
	char * mask=NULL;

	map <int, string> l3_interface; 
	map <int, string>::iterator it;

	try {
		db.open(CONF_DB);

		// get the interface name
		sprintf(sql, "select id, ip from L3INTF;");
		CppSQLite3Query q1 = db.execQuery(sql);

		while (!q1.eof())
        	{	
			l3_interface[q1.getIntField(0)] = q1.getStringField(1);
			q1.nextRow();
		}

		
		sprintf(sql, "select id, net, mask, dev from StaticRoute;");
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{	

			mask = (char *) q.getStringField(2);
			
			if(mask != NULL) 
			{
				if(strcmp("0", mask)==0)
				{
					//default gateway
					default_gw_flag = 1;
					
					//id
					id = q.getIntField(0);
		 
					outss2 << "<tr>";					
					outss2 << "<td>default</td>";

					//gateway, save as network
					outss2 << "<td>" <<  q.getStringField(1) << "</td>";

					//delete button
					outss2 << "<td><input class='btn1' type='button' name='Delete' value='"<< multilang::getLabel("delete") <<"' onclick='DelRoute(" << id << ");'></td>";

					outss2 << "</tr>\n";
				}
				else
				{
					//route list
					outss << "<tr>";
					
					//id
					id = q.getIntField(0);
		 
					//net
					outss << "<td>" <<  q.getStringField(1) << "</td>";

					//mask
					outss << "<td>" <<  q.getStringField(2) << "</td>";

					//interface
					dev = q.getIntField(3);
					it = l3_interface.find(dev);
					if(it == l3_interface.end())
						outss << "<td> none </td>";
					else
						outss << "<td>" << (*it).second << "</td>";
					
					//delete button
					outss << "<td><input class='btn1' type='button' name='Delete' value='"<< multilang::getLabel("delete") <<"' onclick='DelRoute(" << id << ");'></td>";

					outss << "</tr>\n";
				}
			}
			q.nextRow();
		}
		
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();

	if(default_gw_flag == 0)
	{
		//add default gateway modify button
		outss2 << "<tr>";					
		outss2 << "<td>default</td>";

		//gateway, save as network
		outss2 << "<td><input type='text' name='default' value='' size='16'></td>";

		//modify button
		outss2 << "<td><input class='btn1' type='button' name='Modify' value='"<< multilang::getLabel("modify") <<"' onclick='DefaultGWAdd();'></td>";

		outss2 << "</tr>\n";
	}
}



int CRoute::delRoute()
{
	CppSQLite3DB db;
	char 	sql[256];
	char		cmd_buff[128];
	char * route_net=NULL;
	char * mask=NULL;
	
	std::string db_id = getReqValue("listidx");


	try {
		db.open(CONF_DB);

		sprintf(sql, "select net, mask from StaticRoute where id=%d", std::atoi(db_id.c_str()));
		CppSQLite3Query q = db.execQuery(sql);

		if (!q.eof())
        	{	
			route_net = (char *) q.getStringField(0);
			mask = (char *) q.getStringField(1);
		}		

		if((route_net != NULL) && (mask != NULL))
		{
			if(strcmp("0", mask)== 0)
				sprintf(cmd_buff, "/sbin/route del default");
			else
				sprintf(cmd_buff, "/sbin/route del -net %s netmask %s", route_net, mask);
			system(cmd_buff);
			
			sprintf(sql, "delete from StaticRoute where id=%d", std::atoi(db_id.c_str()));
			CppSQLite3Query q = db.execQuery(sql);
			
		}
	}
	catch (CppSQLite3Exception& e)
	{
		return(ERROR_DATABASE);
	}
	
	db.close();
	sprintf(cmd_buff, "sessionID=%s", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/route.cgi", cmd_buff);
	
	return 0;

}

int CRoute::saveRoute()
{
	CppSQLite3DB db;
	char 	sql[256];
	char		cmd_buff[128];
	int ret;
	
	std::string intf_id = getReqValue("intf_id");
	std::string net = getReqValue("net");
	std::string mask = getReqValue("mask");

	try {
		db.open(CONF_DB);

		if(strcmp("0", mask.c_str())== 0)
			sprintf(cmd_buff, "/sbin/route add default gw %s", net.c_str());
		else
			sprintf(cmd_buff, "/sbin/route add -net %s netmask %s dev eth%d", net.c_str(), mask.c_str(), std::atoi(intf_id.c_str())+2);
		ret = system(cmd_buff);

		if(ret != 0)
		{
			db.close();
			return(ERROR_WRONG_ARGS);
		}
		sprintf(sql, "insert into StaticRoute(net, mask, dev) values ( '%s', '%s', %d)", net.c_str(), mask.c_str(), std::atoi(intf_id.c_str()));
		CppSQLite3Query q = db.execQuery(sql);

	}
	catch (CppSQLite3Exception& e)
	{
		return(ERROR_DATABASE);
	}
	
	db.close();
	sprintf(cmd_buff, "sessionID=%s", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/route.cgi", cmd_buff);
	
	return 0;
}

void CRoute::output()
{
	std::string result;
	CCGISession* cs;	
	stringstream outss, outss2;
	int ret;
	
	std::string action = getReqValue("action");
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 
	
	if (action == "new")
	{
		newRoute(outss);
		addUserVariable("intf_idlist", outss.str());
		if( outputHTMLFile("route.html", true) != 0)
		{
			outputError("Cann't open route.html\n");
		}
	}
	else if (action == "del")
	{
		//delete a static route 
		ret =  delRoute();
		if ( ret != ERROR_NO )
			outputError(multilang::getError(ret));
		
	} 
	else if (action == "save")
	{
		//save a static route  
		ret =  saveRoute();
		if ( ret != ERROR_NO )
			outputError(multilang::getError(ret));
	} 
	else if (action == "list") 
	{
		//get static route list
		getRouteList(outss, outss2);
		addUserVariable("route_items", outss.str());
		addUserVariable("default_gw", outss2.str());

		//output with the HTML template
		if( outputHTMLFile("route_table.html", true) != 0)
		{
			outputError("Cann't open route_table.html\n");
		}			
	} 
	else
	{
		//get the main l3config page
		if(outputHTMLFile("l3config.html", true) != 0)
			outputError("Cann't open l3config.html\n");
	}

	free(cs);

}
