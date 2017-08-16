using namespace std;
class CReboot:public CgiRequest
{

public:
	CReboot():CgiRequest()
				{}

	inline ~CReboot()
		{}
	virtual void output();

	private:

		int reboot();





};

