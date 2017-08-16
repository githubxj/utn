class CFrameView : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CFrameView():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CFrameView()
    {  }

	virtual void output();
private:
	int setShelf();
	int getShelfInfo(SHELF_INFO *);
	const char *fanstate;

};

