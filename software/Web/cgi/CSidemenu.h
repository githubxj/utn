using namespace std;

class CSidemenu : public CgiRequest
{
  public:     
    
    /*!
     * \brief  constructor.
     *
     */
    CSidemenu():CgiRequest()
    	{ }

    /*!
     * \brief Destructor 
     *
     * Delete this  object
     */
     inline
    ~CSidemenu()
    {  }

	virtual void output();

  private: 
  	void genCardsMenu(stringstream & outss);
  	void genONUNodesMenu(stringstream & outss, int &card_no, int &menuId,  int parent_mId, CppSQLite3DB &db);
    	void genDAUNodesMenu(stringstream & outss, int &card_no, int &menuId,  int parent_mId, CppSQLite3DB &db);
	void genEPONUMenu(stringstream & outss, int &card_no, int &menuId,  int parent_mId, CppSQLite3DB &db);
	void genENcodeMenu(stringstream &outss, int &cardno, int &memuId, int parentmID);
	void genDEcodeMenu(stringstream &outss, int &cardno, int &memuId, int parentmID);
	void genNTTMenu(stringstream &outss, int &cardno, int &memuId, int parentmID,CppSQLite3DB &db);
	void genONU100Menu (stringstream &outss, int &cardno, int &menuID, int parentmID, CppSQLite3DB &db);
	void genMTTMenu(stringstream &outss, int &cardno, int &memuId, int parentmID,CppSQLite3DB &db);

  private: 
	std::vector<int>  fsa_list;
	std::vector<int>  fsa_list_mid;
	std::vector<int> encode_list;
	std::vector<int> encode_list_mid;
	std::vector<int> decode_list;
	std::vector<int> decode_list_mid;
	std::vector<int> fsa600_list;
	std::vector<int> fsa600_list_mid;
	std::vector<int> ntu_list;
	std::vector<int> ntu_list_mid;
	std::vector<int> epn_list;
	std::vector<int> epn_list_mid;
	std::vector<int>  dat_list;
	std::vector<int>  dat_list_mid;

    std::vector<int> mtu_list;          // 201611
    std::vector<int> mtu_list_mid;
};


