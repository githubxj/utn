using namespace std;

class CVlan : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CVlan():CgiRequest()
    	{ 
		
	}

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CVlan()
    {  }

	virtual void output();
	
  private: 
  	void getVlanitems(stringstream & outss);
	int Getvlan();
	int Addvlan();
	int Delvlan();
	int Modifyvlan();
	int Savevlan();
	const char *getmembers( int map );
	std::string m_desthtml;


  };


