using namespace std;

class CSnmpconfig : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CSnmpconfig():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CSnmpconfig()
    {  }

	virtual void output();

  private:
	int getConfigFile();
	int setConfigFile();
	int conf_save (const char * section, const char * keyname, const char * value  );

	std::string m_desthtml;

};


