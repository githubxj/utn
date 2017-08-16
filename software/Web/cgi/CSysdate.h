using namespace std;

class CSysdate : public CgiRequest
{
  public:
     
    
    /*!
     * \brief  constructor.
     *
     */
    CSysdate():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CSysdate()
    {  }

	virtual void output();

private:
	int getDate();
	int setDate();

};


