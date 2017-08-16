using namespace std;

class CL3Config :public CgiRequest
{
public:
	CL3Config():CgiRequest() {}
	inline ~CL3Config()	{}
				
	virtual void output();
				
private:

	int saveL3Interface();
	int delL3Interface();
	void getL3InterfaceList(stringstream & outss);
	void getVLANList(stringstream & outss, int select_id);
	int getL3Interface(int id);
	int getAvailabelIntfID();

};





