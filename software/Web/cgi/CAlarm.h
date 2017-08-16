using namespace std;

class CAlarm : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CAlarm():CgiRequest()
    	{ 
		for(int i=0; i<= MAX_CARD_NUM_UTN; i++)
			card_type[i] = CARD_TYPE_UNKOWN;
	}

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CAlarm()
    {  }

	virtual void output();
	
  private: 
  	void getAlarmRecs(stringstream & outss);
	
	 std::string getseverity(int level);
	std::string getDevice(int alarm_type, int cardno, int nodeid);
	std::string getAlarmName(int alarm_type);


	int 	card_type[MAX_CARD_NUM_UTN + 1];
  };


