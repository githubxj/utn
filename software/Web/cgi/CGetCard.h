using namespace std;

class CGetCard : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CGetCard():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CGetCard()
    {  }

	virtual void output();

  private:
	int getCardData();
	const char * getCardName(unsigned char type);
	const char * makeVersion(unsigned char high, unsigned char low);
	const char *getLink(unsigned char type);
	const char *getModeFE8(unsigned char type);
	const char *getWorkingModefe8(unsigned char type);
	const char *getProtocol(const char *protocol);
	const char *getChannelNO(unsigned char channelno);
	const char *getChannel(unsigned char channelno);
	const char *getOSDLocation (unsigned char location);
	const char *getOSDColor(unsigned char color);
	const char *showTime(unsigned char state);
	const char *showVlost(unsigned char vlost);
	const char *getCompresstype ( unsigned char type);
	const char * getStreamtype ( unsigned char type );
	const char *get_gop(unsigned char gop);
	const char *get_resolving(const char *resolving);
	const char *get_vid_bit_type(unsigned char vid_bit_type);
	const char *show_cbr(unsigned char vid_bit_type);
	const char *get_vbr(unsigned char vbr);
	const char *getLockstate ( unsigned char state );
	const char *getTransimitmode ( unsigned char mode , unsigned char type);
	const char *getSFPstate ( unsigned char state );
	const char *getWorkingModefsa600rmt ( unsigned char type, int portno, int readonly  );
	std::string m_desthtml;

};
