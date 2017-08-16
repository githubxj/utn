using namespace std;

class CGetEPOnu : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CGetEPOnu():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CGetEPOnu()
    {  }

	virtual void output();

  private:
	int getOnuData();
	const char *getLink(unsigned char type);
	const char *getBaudrate(unsigned char type );
	const char *getPortselect(unsigned char type );
	std::string m_desthtml;

};

