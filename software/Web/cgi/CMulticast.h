using namespace std;

class CMulticast : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CMulticast():CgiRequest()
    	{ 
		
	}

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CMulticast()
    {  }

	virtual void output();
	
  private: 
	int GetAllMC();
	int AddMC();
	int DelMC();
	int ModifyMC();
	int SaveMC();
	const char *getmembers( int map );
	std::string m_desthtml;


  };


