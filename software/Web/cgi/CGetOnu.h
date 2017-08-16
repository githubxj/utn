using namespace std;

class CGetOnu : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CGetOnu():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CGetOnu()
    {  }

	virtual void output();

  private:
	int getOnuData();
	const char *getLink(unsigned char type);
	const char *getModeONU(unsigned char type);
	const char *getWorkingModeONU(unsigned char type);
	const char *getBaudrate(unsigned char type );
	const char *getprioritySchemeSelect(unsigned char type);
	const char *getPortselect(unsigned char type );
	const char *getStopbit(unsigned char type );
	const char *getDatabit(unsigned char type );
	const char *getParity(unsigned char type );
	const char *getmembers(unsigned char map);
	std::string m_desthtml;

};

