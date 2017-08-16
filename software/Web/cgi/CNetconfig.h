using namespace std;

class CNetconfig :public CgiRequest
	{
		public:
			CNetconfig():CgiRequest()
				{}
				
				inline ~CNetconfig()
				{}
				
				virtual void output();
				
			private:
				int ChgNetConfig();
				int getNetConfig();
				
				const char *ip;
				const char *mask;
		
				const char *gate;


		
		};





