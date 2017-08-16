
using namespace std;

class CTopoView : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CTopoView():CgiRequest()
    	{ 
		m_onu_number = 0;
		link_end1 = -1;
		link_end2 = -1;
		ring_error1 = -1;
		ring_error2 = -1;
	}

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CTopoView()
    {  }

	virtual void output();
	
  private: 
  	void getONUList(int card_no);
	
   	void genOLT();
 	void genTopList(int number, int padding);
  	void genBottomList(int number, int offset, int padding);
  	void genLeftList(int number, int padding);
  	void genRightList(int number, int padding);

	inline void genNode(stringstream &outss, ONU_LIST_ENTRY& onu_entry )
	{
		char	onu_name[32];
		memcpy(onu_name, onu_entry.onu_name, MAX_DISPLAYNAME_LEN );
		onu_name[MAX_DISPLAYNAME_LEN] ='\0';

		if(onu_entry.node_state == ONU_STATE_OFFLINE)
		{
			outss <<"<IMG title=\""<< onu_name ;
			outss <<"\" src=\"/images/onu_node-offline.gif\">";			
		}else {			
			outss << "<a href='/cgi-bin/show_onu.cgi?sessionID="<< getReqValue("sessionID");
			if(m_card_type == CARD_TYPE_DAT)
				outss <<"&flag=dau";
			else
				outss <<"&flag=initonu";
			outss <<"&cardno="<<m_card_no <<"&onunode=" << (int) onu_entry.node_id << "'>";
			outss <<"<IMG title=\""<< onu_name ;
			if(onu_entry.node_state == ONU_STATE_ONLINEE)
				outss <<"\" src=\"/images/onu_node.gif\"></a>";
			else
				outss <<"\" src=\"/images/onu_node-dis.gif\"></a>";
		}
	}
	
  private:
  	int				m_card_no;
	int				m_card_type;
	int				m_onu_number;
  	ONU_LIST_ENTRY 	m_onu_list[MAX_ONU_NUM];
	int 				link_end1;		//the last onu id at the link1 side
	int 				link_end2;		//the last onu id at the link2 side
	int 				ring_error1;		//the ring error node
	int 				ring_error2;		//the ring error node
	

};

