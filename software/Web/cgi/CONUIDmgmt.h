class CONUIDmgmt : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CONUIDmgmt():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CONUIDmgmt()
    {  }

	virtual void output();

private:
	std::string m_desthtml;

	int getONUList();
	int getFSAList();
	int setID();
	int resetOnu();

};

