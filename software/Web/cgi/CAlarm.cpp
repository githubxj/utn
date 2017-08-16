#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "const.h"
#include "alarm_type.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"

#include "CAlarm.h"
#define  PAGE_SIZE 20
using namespace std;


void CAlarm::getAlarmRecs(stringstream & outss)
{
	CppSQLite3DB db;
	char 	sql[256];
	char		buff[32];
	int rec_count = 0, pages_total=0;
	int severity, alarm_type, cardno, nodeid, port;

	try {
		db.open(DYNAMIC_DB);

		//get the card type cache		
		CppSQLite3Query q_card = db.execQuery("select CardNo, CardType from Card;");

		while (!q_card.eof()){
			cardno = q_card.getIntField(0);
			if(cardno >0 && cardno <= MAX_CARD_NUM_UTN)
					card_type[cardno] = q_card.getIntField(1);

			q_card.nextRow();
		}

		//get the total alarm count;
		CppSQLite3Query q_count = db.execQuery("select count(*) from Alarm");

		if(!q_count.eof())
			rec_count = q_count.getIntField(0);

		int req_page = atoi(getReqValue("page").c_str());
		pages_total = (rec_count + PAGE_SIZE -1)/PAGE_SIZE;

		sprintf(sql, "select Severity, AlarmTime, AlarmType, CardNo, NodeId, Port, Reason from Alarm order by AlarmTime DESC limit %d, %d;", req_page * PAGE_SIZE, PAGE_SIZE);
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{
        		severity = q.getIntField(0);
			//std::string level = getseverity(severity);
			outss << "<tr class='s" << severity<< "'>";

			//alarm time
//			Convert.ToDateTime(ds.Tables[0].Rows[0]["datetime"]).ToString("yyMMdd");

			outss << "<td>" <<  q.getStringField(1) << "</td>";

			//alarm type
			alarm_type = q.getIntField(2);
			outss << "<td>" <<  multilang::getAlarmName(alarm_type, card_type[cardno]) << "</td>";
			
 			//alarm severity
			outss << "<td>" <<  multilang::getSeverity(severity) << "</td>";

 			//device type
			cardno = q.getIntField(3);
			nodeid = q.getIntField(4);
			port = q.getIntField(5);
			
 			//std::string devicename = getDevice(alarm_type, cardno, nodeid);
			outss << "<td>" <<  multilang::getDevice(alarm_type, card_type[cardno], nodeid) << "</td>";

			//card no
			if(cardno == 0)
				outss << "<td align='center'> - </td>";
			else
				outss << "<td align='center'>" <<  cardno << "</td>";

			//nodeid
			if(nodeid == 0)
				outss << "<td align='center'> - </td>";
			else
				outss << "<td align='center'>" <<  nodeid << "</td>";
			
			//port
			if(port == 0)
				outss << "<td align='center'> - </td>";
			else
				outss << "<td align='center'>" <<  port << "</td>";
			
			//other info
			outss << "<td>" <<  q.fieldValue(6) << "</td>";

			outss << "</tr>\n";
			q.nextRow();
		}
		
		//generate the navigate info
		if(rec_count > PAGE_SIZE) 
		{
			stringstream nav_outs;
				
			sprintf(buff, "%d/%d", req_page + 1, pages_total);
			addUserVariable("page_info", buff);

			if(req_page != 0)
			{	
				nav_outs <<"<a class='linkalarm' href='/cgi-bin/getalarm.cgi?sessionID="<< getReqValue("sessionID") << "&page=" << req_page-1 << "'> " << multilang::getLabel("prev") << " </a>&nbsp;";			
			}

			if((req_page + 1) != pages_total)
				nav_outs <<"<a class='linkalarm' href='/cgi-bin/getalarm.cgi?sessionID="<< getReqValue("sessionID") << "&page=" << req_page + 1 << "'> " << multilang::getLabel("next") << " </a>&nbsp;";			

			addUserVariable("navigate", nav_outs.str());
			
		}
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();
}

void CAlarm::output()
{
	CCGISession* cs;
	stringstream outss;
	std::string sid;

	// check login session
	sid = getReqValue("sessionID");
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(sid.c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	// generate the alarm records
	getAlarmRecs(outss);
	
	addUserVariable("alarm_recs", outss.str());

	//output with the HTML template
	if( outputHTMLFile("alarms.html", true) != 0)
	{
		outputError("Cann't open alarms.html\n");
	}	

	free(cs);
   
}

