using namespace std;

class CChgip :public CgiRequest
	{
		public:
			CChgip():CgiRequest()
				{}
				
				inline ~CChgip()
				{}
				
				virtual void output();
				
			private:
				int ChgIPAddress();
				std::string ip;
				


		
		};





