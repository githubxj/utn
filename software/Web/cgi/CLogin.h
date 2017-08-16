using namespace std;

class CLogin : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CLogin():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CLogin()
    {  }
	
	virtual void output();

  private: 
  	int checkLogin(const char * user, const char * pwd);

};

