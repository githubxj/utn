using namespace std;

class CFsaCard: public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CFsaCard():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CFsaCard()
    {  }

	virtual void output();

  private:
	int procFsaReq();
	const char *getLink(unsigned char type);
	const char *getModeFSA(unsigned char type, int portno);
	const char *getWorkingModefsa(unsigned char type, int portno);
	std::string m_desthtml;

};


