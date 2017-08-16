using namespace std;

class CGetCount :public CgiRequest
	{
		public:
			CGetCount():CgiRequest()
				{}
				
				inline ~CGetCount()
				{}
				
				virtual void output();
				
			private:
				int getCardCount();
				std::string m_desthtml;
		
		
		};
