using namespace std;

class CHDCard: public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CHDCard():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CHDCard()
    {  }

	virtual void output();

  private:
	int procHDReq();
	const char *getSourceType(unsigned char type);
	const char *getProtocol(unsigned char type);
	const char *getFrameRate(unsigned char frame_rate);
	const char *getDecMode(unsigned char frame_rate);
	const char *getCVBSMode(unsigned char var);
	const char *getOutputMode1(unsigned char var);
	const char *getOutputMode2(unsigned char var);
	const char *getOutputType(unsigned char var);
	int get_display_mode_by_string(const char *str);
	int genSwitchCmd(char *out, int ch, int display_mode);
	std::string m_desthtml;

};


