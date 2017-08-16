using namespace std;

class CClearAlarm : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CClearAlarm():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CClearAlarm()
    {  }

	virtual void output();
	
  private: 
  	int  ClearAlarm();

};


