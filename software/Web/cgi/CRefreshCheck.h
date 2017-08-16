using namespace std;

class CRefreshCheck :public CgiRequest
	{
		public:
			CRefreshCheck():CgiRequest()
				{}
				
				inline ~CRefreshCheck()
				{}
				
				virtual void output();
				
		
		};