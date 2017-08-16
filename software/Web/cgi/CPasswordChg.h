using namespace std;

class CPasswordChg : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CPasswordChg():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CPasswordChg()
    {  }

	virtual void output();

  private: 
  	int updatePwd(const char * oldpwd, const char * newpwd);

};

