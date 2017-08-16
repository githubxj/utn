using namespace std;

class CRoute :public CgiRequest
{
public:
	CRoute():CgiRequest() {}
	inline ~CRoute()	{}
				
	virtual void output();
				
private:

	int saveRoute();
	int delRoute();
	void newRoute(stringstream & outss);
	void getRouteList(stringstream & outss, stringstream & outss2);

};





