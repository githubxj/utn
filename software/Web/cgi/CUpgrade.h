using namespace std;

class CUpgrade : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CUpgrade():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CUpgrade()
    {  }
	
	virtual void output();

  private: 
	int do_update(const char* filename);


};

