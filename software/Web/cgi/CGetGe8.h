using namespace std;

class CGetGE8: public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CGetGE8():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CGetGE8()
    {  }

	virtual void output();

  private:
	int getGe8Data();
	const char *getStateGe8(unsigned char type);
	const char *getLink(unsigned char type);
	const char *getModeGE8(unsigned char type);
	const char *getWorkingModeGE8(unsigned char type);
	const char *getBaudrate(unsigned char type );
	const char *getprioritySchemeSelect(unsigned char type);
	const char *getPortselect(unsigned char type );
	const char *getmembers(unsigned char map);
	std::string m_desthtml;

};


