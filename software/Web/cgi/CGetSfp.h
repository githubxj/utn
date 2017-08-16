using namespace std;

class CGetSfp : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CGetSfp():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CGetSfp()
    {  }

	virtual void output();

  private:
	int getSFPInfo();
	const char *CGetSfp::getTransMode(unsigned char type);
	const char *getInterfaceType(unsigned char type);
	std::string m_desthtml;

};

