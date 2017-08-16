#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"

#include "CSidemenu.h"

using namespace std;

//define card display name
const char cardname_fsa[] = "FSA1250";
const char cardname_fsa_covond[] = "MAT1000";
const char cardname_ctl[] = "SW&CTL";
const char cardname_sw_ge[] = "SW-GE";
const char cardname_fe8[] = "FE8";
const char cardname_ge8[] = "GE8";
const char cardname_ntt[] = "NTT";
const char cardname_ms4[] = "MS4V1H";
const char cardname_epn[] = "EPN2000";
const char cardname_dat[] = "DAT1000";
const char cardname_hdc[] = "HD-DEC";
const char cardname_sde[] = "SDI-EC";
const char cardname_msd[] = "MS-DEC";
const char cardname_m4s[] = "M4S";
const char cardname_m4h[] = "M4H";
const char cardname_mtt[] = "MTT";  // 201611
const char cardname_mtt441[] = "MTT441";  // 201708
const char cardname_mtt411[] = "MTT411";  // 201708

const char cardname_encode[] = "H4EC";
const char cardname_decode[] = "H4DC";
const char cardname_fsa600[] = "FSA600";
const char cardname_fsa600_covond[] = "DAT600";
const char cardname_unknown[] = "Unknown";

void CSidemenu::genONUNodesMenu(stringstream & outss, int &card_no, int &menuId, int parent_mId, CppSQLite3DB &db)
{
	char sql[256];
	int node_id, node_state;
	
	sprintf(sql, "select NodeId, NodeState  from ONUList where CardNo=%d order by  NodeId;", card_no);

	try {
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
      		{
       		node_id = q.getIntField(0);
			node_state = q.getIntField(1);
			
			if(node_state == ONU_STATE_OFFLINE) 
			{
				q.nextRow();
				continue;
			}

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
#ifdef COVOND
			outss << "d.add("<<menuId<<", "<<parent_mId<<", 'MAU-"<< node_id <<"', '/cgi-bin/show_onu.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"&onunode="<< node_id<<"&flag=initonu";
#else
			outss << "d.add("<<menuId<<", "<<parent_mId<<", 'ONU-"<< node_id <<"', '/cgi-bin/show_onu.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"&onunode="<< node_id<<"&flag=initonu";
#endif
			if(node_state == ONU_STATE_ONLINEE ) 
				outss <<"','','showframe',d.icon.onu,d.icon.onu);";
			else if (node_state == ONU_STATE_ONLINED )
				outss <<"','','showframe',d.icon.onudis,d.icon.onudis);";
			else 
				outss <<"','','showframe',d.icon.onuoff,d.icon.onuoff);";				

			q.nextRow();
			menuId++;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
}

void CSidemenu::genDAUNodesMenu(stringstream & outss, int &card_no, int &menuId, int parent_mId, CppSQLite3DB &db)
{
	char sql[256];
	int node_id, node_state;
	
	sprintf(sql, "select NodeId, NodeState  from ONUList where CardNo=%d order by  NodeId;", card_no);

	try {
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
      		{
       		node_id = q.getIntField(0);
			node_state = q.getIntField(1);
			
			if(node_state == ONU_STATE_OFFLINE) 
			{
				q.nextRow();
				continue;
			}

			outss << "d.add("<<menuId<<", "<<parent_mId<<", 'DAU-"<< node_id <<"', '/cgi-bin/show_onu.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"&onunode="<< node_id<<"&flag=dau";
			if(node_state == ONU_STATE_ONLINEE ) 
				outss <<"','','showframe',d.icon.onu,d.icon.onu);";
			else if (node_state == ONU_STATE_ONLINED )
				outss <<"','','showframe',d.icon.onudis,d.icon.onudis);";
			else 
				outss <<"','','showframe',d.icon.onuoff,d.icon.onuoff);";				

			q.nextRow();
			menuId++;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
}
void CSidemenu::genONU100Menu(stringstream & outss, int & cardno, int & menuID, int parentmID, CppSQLite3DB &db)
{

	char sql[256];
	int  node_id;
	std::string flag ("remote");

	sprintf(sql, "select NodeId  from ONUList where CardNo=%d order by  NodeId;", cardno);

	try {
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
      		{
       		node_id = q.getIntField(0);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
#ifdef COVOND
			outss << "d.add("<<menuID<<", "<<parentmID<<", 'MAU100-"<< node_id <<"', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<cardno<<"&onuid="<< node_id <<"&flag=" << flag;
#else
			outss << "d.add("<<menuID<<", "<<parentmID<<", 'ONU100-"<< node_id <<"', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<cardno<<"&onuid="<< node_id <<"&flag=" << flag;
#endif
			outss <<"','','showframe',d.icon.onu,d.icon.onu);";			

			q.nextRow();
			menuID++;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
}



void CSidemenu::genENcodeMenu(stringstream &outss, int &cardno, int &memuId, int parentmID)
{

	std::string flag_channel ("channel");
	/*std::string flag_net ("network");
	std::string flag_systime ("systime");
	std::string flag_ntp ("ntp");
	std::string flag_reboot("reboot");*/
	
	// generate the menu item:
	// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-1" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<0;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-2" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<1;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-3" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<2;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-4" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<3;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;
	
	/*outss<< "d.add(" <<memuId <<","<<parentmID<<",'network config" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_net;
	outss <<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;
	
	outss<<"d.add(" <<memuId<<","<<parentmID<<",'system time" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_systime;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;
	
	outss<<"d.add(" <<memuId<<","<<parentmID<<",'NTP" <<"','/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_ntp;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;
	
	outss<< "d.add(" <<memuId <<","<<parentmID<<",'reboot" <<"','/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_reboot;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;*/	
}

void CSidemenu::genDEcodeMenu(stringstream &outss, int &cardno, int &memuId, int parentmID)
{

	std::string flag_channel ("channel");
	
	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-1" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<0;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-2" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<1;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-3" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<2;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;

	outss<<"d.add(" <<memuId <<","<<parentmID<<",'channel-4" <<"','/cgi-bin/show_card.cgi?sessionID=" << getReqValue("sessionID") <<"&cardno="<<cardno<<"&flag="<<flag_channel<<"&ch_no="<<3;
	outss<<"','','showframe',d.icon.page,d.icon.page);";
	memuId++;	
}



void CSidemenu::genNTTMenu(stringstream & outss, int & cardno, int & menuID, int parentmID, CppSQLite3DB &db)
{

	char sql[256];
	int  ntuno;
	std::string flag ("getntuport");

	sprintf(sql, "select NtuId  from NTUList where CardNo=%d order by  NtuId;", cardno);

	try {
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
      		{
       		ntuno = q.getIntField(0);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
#ifdef __MTT__ // 201611			
			outss << "d.add("<<menuID<<", "<<parentmID<<", 'MTU-"<< ntuno <<"', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<cardno<<"&ntuno="<< ntuno <<"&flag=" << flag;
#else
            outss << "d.add("<<menuID<<", "<<parentmID<<", 'NTU-"<< ntuno <<"', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<cardno<<"&ntuno="<< ntuno <<"&flag=" << flag;
#endif
			outss <<"','','showframe',d.icon.onu,d.icon.onu);";			

			q.nextRow();
			menuID++;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
}

#if 1 // 201611
void CSidemenu::genMTTMenu(stringstream & outss, int & cardno, int & menuID, int parentmID, CppSQLite3DB &db)
{

	char sql[256];
	int  mtuno;
	std::string flag ("getmtuport");

	sprintf(sql, "select MtuId  from MTUList where CardNo=%d order by  MtuId;", cardno);

	try {
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
		{
       		mtuno = q.getIntField(0);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)

			outss << "d.add("<<menuID<<", "<<parentmID<<", 'MTU-"<< mtuno 
			    <<"', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") 
			    <<"&cardno="<<cardno<<"&ntuno="<< mtuno <<"&flag=" << flag;
			outss <<"','','showframe',d.icon.onu,d.icon.onu);";			

			q.nextRow();
			menuID++;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
}
#endif

void CSidemenu::genEPONUMenu(stringstream & outss, int &card_no, int &menuId, int parent_mId, CppSQLite3DB &db)
{
	char sql[256];
	int node_id;
	
	try {
		// 1st PON
		sprintf(sql, "select NodeId  from EPONUList where CardNo=%d and Pon=0 and NodeState=1 order by  NodeId;", card_no);
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
      		{
       		node_id = q.getIntField(0);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
			outss << "d.add("<<menuId<<", "<<parent_mId +1<<", 'ONU-"<< node_id <<"', '/cgi-bin/show_eponu.cgi?sessionID="<< getReqValue("sessionID") <<"&pon=0&cardno="<<card_no<<"&onunode="<< node_id;
			outss <<"','','showframe',d.icon.onu,d.icon.onu);";

			q.nextRow();
			menuId++;
		}

		// 1st PON
		sprintf(sql, "select NodeId  from EPONUList where CardNo=%d and Pon=1  and NodeState=1 order by  NodeId;", card_no);
		q = db.execQuery(sql);

		while (!q.eof())
      		{
       		node_id = q.getIntField(0);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
			outss << "d.add("<<menuId<<", "<<parent_mId+2<<", 'ONU-"<< node_id <<"', '/cgi-bin/show_eponu.cgi?sessionID="<< getReqValue("sessionID") <<"&pon=1&cardno="<<card_no<<"&onunode="<< node_id<<"&flag=initonu";
			outss <<"','','showframe',d.icon.onu,d.icon.onu);";

			q.nextRow();
			menuId++;
		}
	
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
}

void CSidemenu::genCardsMenu(stringstream & outss)
{
	CppSQLite3DB db;
	int menuId = 2;
	int card_no, card_type;
	int ge_sw=0;

	fsa_list.clear();
	fsa_list_mid.clear();
	encode_list.clear ();
	encode_list_mid.clear ();
	fsa600_list.clear();
	fsa600_list_mid.clear();
	ntu_list.clear();
	ntu_list_mid.clear();
	epn_list.clear();
	epn_list_mid.clear();
	dat_list.clear();
	dat_list_mid.clear();
	
	mtu_list.clear();       //201611
	mtu_list_mid.clear();
	
	try {
		db.open(DYNAMIC_DB);

		CppSQLite3Query q1 = db.execQuery("select CardNo, CardType from Card where CardType=8 and CardNo=17;");
		if(!q1.eof())
			ge_sw=1;

		CppSQLite3Query q = db.execQuery("select CardNo, CardType from Card order by  CardNo;");

		while (!q.eof())
        	{
        		card_no = q.getIntField(0);
			card_type = q.getIntField(1);

			// generate the menu item:
			// 		Node(id, pid, name, url, title, target, icon, iconOpen, open)
			switch(card_type) {
				case CARD_TYPE_CTRL:
						outss << "d.add(" << menuId << ", 1, '" << cardname_ctl << "-" << card_no << "-a', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card,d.icon.card);";
						if(CTools::hasL3Capability())
						{
							outss << "d.add(" << menuId+1 << ", "<< menuId<<", '" <<  multilang::getLabel("l3") << "', '/cgi-bin/l3config.cgi?sessionID="<< getReqValue("sessionID") <<"','','showframe',d.icon.l3,d.icon.l3);";
							menuId++;
						}
						
						if(ge_sw)
						{	//add giga switch card
							menuId++;
							outss << "d.add(" << menuId << ", 1, '" << cardname_sw_ge<< "-" << card_no << "-b', '/cgi-bin/show_ge8.cgi?sessionID="<< getReqValue("sessionID") <<"&flag=base&cardno="<<GW_CARDNO<<"','','showframe',d.icon.card,d.icon.card);";
						}
						break;
				case CARD_TYPE_FSA:
#ifdef COVOND
						outss << "d.add(" << menuId << ", 1, '" << cardname_fsa_covond << "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card,d.icon.card);";
#else
						outss << "d.add(" << menuId << ", 1, '" << cardname_fsa<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card,d.icon.card);";
#endif
						fsa_list.push_back(card_no);
						fsa_list_mid.push_back(menuId);
						break;
				case CARD_TYPE_DAT:
						outss << "d.add(" << menuId << ", 1, '" << cardname_dat<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card,d.icon.card);";
						dat_list.push_back(card_no);
						dat_list_mid.push_back(menuId);
						break;
				case CARD_TYPE_FE8:
						outss << "d.add(" << menuId << ", 1, '" << cardname_fe8<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_MS4V1H:
						outss << "d.add(" << menuId << ", 1, '" << cardname_ms4<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_M4H:
						outss << "d.add(" << menuId << ", 1, '" << cardname_m4h<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_M4S:
						outss << "d.add(" << menuId << ", 1, '" << cardname_m4s<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_EPN:
						outss << "d.add(" << menuId << ", 1, '" << cardname_epn<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";

						outss << "d.add(" << menuId+1 << ", "<< menuId<<", '" <<  "PON 1" << "', '/cgi-bin/epnview.cgi?sessionID="<< getReqValue("sessionID") << "&pon=0&type=0&cardno=" <<card_no <<"','','showframe',d.icon.fiber,d.icon.fiber);";
						outss << "d.add(" << menuId+2 << ", "<< menuId<<", '" <<  "PON 2" << "', '/cgi-bin/epnview.cgi?sessionID="<< getReqValue("sessionID") << "&pon=1&type=0&cardno=" <<card_no <<"','','showframe',d.icon.fiber,d.icon.fiber);";
						epn_list.push_back(card_no);
						epn_list_mid.push_back(menuId);
						menuId +=2 ;
						break;
				case CARD_TYPE_GE8:
						if(card_no != GW_CARDNO)
							outss << "d.add(" << menuId << ", 1, '" << cardname_ge8<< "-" << card_no << "', '/cgi-bin/show_ge8.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"&flag=base"<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_ENCODE:
						outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_encode<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						encode_list.push_back(card_no);
						encode_list_mid.push_back (menuId);
						break;
				case CARD_TYPE_DECODE:
						outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_decode<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						decode_list.push_back(card_no);
						decode_list_mid.push_back (menuId);
						break;
				case CARD_TYPE_FSA600:
#ifdef COVOND
						outss<<"d.add(" <<menuId << ", 1, '" << cardname_fsa600_covond << "-" << card_no <<"', '/cgi-bin/show_card.cgi?sessionID=" << getReqValue ("sessionID") << "&cardno=" << card_no << "', '', 'showframe', d.icon.card1,d.icon.card1);";
#else
						outss<<"d.add(" <<menuId << ", 1, '" << cardname_fsa600 << "-" << card_no <<"', '/cgi-bin/show_card.cgi?sessionID=" << getReqValue ("sessionID") << "&cardno=" << card_no << "', '', 'showframe', d.icon.card1,d.icon.card1);";
#endif
						fsa600_list.push_back (card_no);
						fsa600_list_mid.push_back (menuId);
						break;
#ifdef __MTT__ // 201611
                case CARD_TYPE_MTT:
                        outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_mtt<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no <<"','','showframe',d.icon.card1,d.icon.card1);";
                        //ntu_list.push_back(card_no);
                        //ntu_list_mid.push_back (menuId);
                        mtu_list.push_back(card_no);
                        mtu_list_mid.push_back (menuId);
                break;
#else
                case CARD_TYPE_NTT:
#endif

#if 1 // 201708
                case CARD_TYPE_MTT_411:
                    outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_mtt411<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no <<"','','showframe',d.icon.card1,d.icon.card1);";
                    mtu_list.push_back(card_no);
                    mtu_list_mid.push_back (menuId);
                break;
                
                case CARD_TYPE_MTT_441:
                    outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_mtt441<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no <<"','','showframe',d.icon.card1,d.icon.card1);";
                    mtu_list.push_back(card_no);
                    mtu_list_mid.push_back (menuId);
                break;
#endif

                case CARD_TYPE_NTT_S: // 20150115
						outss<< "d.add(" <<menuId<<   ", 1, '" <<cardname_ntt<< "-" <<card_no<< "', '/cgi-bin/show_card.cgi?sessionID=" <<getReqValue("sessionID") <<"&cardno="<<card_no <<"','','showframe',d.icon.card1,d.icon.card1);";
						ntu_list.push_back(card_no);
						ntu_list_mid.push_back (menuId);
						break;
						
				case CARD_TYPE_HDC:
						outss << "d.add(" << menuId << ", 1, '" << cardname_hdc<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_SDE:
						outss << "d.add(" << menuId << ", 1, '" << cardname_sde<< "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				case CARD_TYPE_MSD:
						outss << "d.add(" << menuId << ", 1, '" << cardname_msd << "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
				default:
						outss << "d.add(" << menuId << ", 1, '" <<  cardname_unknown << "-" << card_no << "', '/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no<<"','','showframe',d.icon.card1,d.icon.card1);";
						break;
						
			} 			
			q.nextRow();
			menuId++;
		}

		for(int i=0; i<fsa_list.size(); i++)
			genONUNodesMenu(outss, fsa_list[i], menuId, fsa_list_mid[i], db);
		for(int i=0; i<dat_list.size(); i++)
			genDAUNodesMenu(outss, dat_list[i], menuId, dat_list_mid[i], db);
		for (int i=0; i < encode_list.size(); i++ )
			genENcodeMenu( outss, encode_list[i], menuId, encode_list_mid[i]);
		for (int i=0; i < decode_list.size(); i++ )
			genDEcodeMenu( outss, decode_list[i], menuId, decode_list_mid[i]);
		for (int i = 0;i < fsa600_list.size (); i++)
			genONU100Menu(outss , fsa600_list[i] , menuId , fsa600_list_mid[i] , db);
		for (int i=0;i < ntu_list.size(); i++)
			genNTTMenu(outss,ntu_list[i],menuId,ntu_list_mid[i],db);
		for (int i=0;i < epn_list.size(); i++)
			genEPONUMenu(outss,epn_list[i],menuId,epn_list_mid[i],db);
			
		for (int i=0;i < mtu_list.size(); i++) // 201611
			genMTTMenu(outss, mtu_list[i], menuId, mtu_list_mid[i],db);
					
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();
}
	
void CSidemenu::output()
{
	CCGISession* cs;
	stringstream outss;
	std::string sid;
	char  buffer[256];

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

	// generate the device menu items
	outss << "d = new dTree('d');";
	outss << "d.add(0,-1,'" << multilang::getLabel("alldev") << "&nbsp;&nbsp;&nbsp;&nbsp; ' );" ;
	outss << "d.add(1, 0, '" << multilang::getLabel("frame") <<"', '/cgi-bin/frameview.cgi?sessionID="<< getReqValue("sessionID")<< "','','showframe',d.icon.olt,d.icon.olt);";

	genCardsMenu(outss);

	outss << "document.write(d);";
	
	addUserVariable("device_menu", outss.str());

	//expand the topo view menu items
	outss.str("");
	if( fsa_list.size() == 0 )
	{
		if( dat_list.size() == 0 )
		{
			outss << "<li> None </li>" << std::endl;
		} 
	} else {
		for(int i=0; i<fsa_list.size(); i++)
		{ 	
			sprintf(buffer, "/cgi-bin/topoview.cgi?sessionID=%s&cardno=%d", sid.c_str(), fsa_list[i]);
			outss << "<li><a href=\"" << buffer << "\" target=\"showframe\">";
#ifdef COVOND
			outss << "MSRing-" << fsa_list[i] << "</a></li>" << std::endl;
#else
			outss << "FSA-" << fsa_list[i] << "</a></li>" << std::endl;
#endif
		}
	}

	if( dat_list.size() != 0 )
	{
		for(int i=0; i<dat_list.size(); i++)
		{ 	
			sprintf(buffer, "/cgi-bin/topoview.cgi?sessionID=%s&cardno=%d", sid.c_str(), dat_list[i]);
			outss << "<li><a href=\"" << buffer << "\" target=\"showframe\">";
			outss << "DATRing-" << dat_list[i] << "</a></li>" << std::endl;
		}
	}

	addUserVariable("onu_networks", outss.str());

	//output with the HTML template
	if( outputHTMLFile("sidemenu.html", true) != 0)
	{
		outputError("Cann't open sidemenu.html\n");
	}	

	free(cs);
   
}

