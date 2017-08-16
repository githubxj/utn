
using namespace std;

class CEpnView : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CEpnView():CgiRequest()
    	{ 
	}

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CEpnView()
    {  }

	virtual void output();
	
  private: 
  	int getEponuList(int pon);
  	int delOfflineEponu(int pon);
	

	
  private:
  	int				m_card_no;
};

