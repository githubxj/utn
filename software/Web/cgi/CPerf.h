using namespace std;

class CGetPerf :public CgiRequest
	{
		public:
			CGetPerf():CgiRequest()
				{}
				
				inline ~CGetPerf()
				{}
				
				virtual void output();
				
			private:
				int getCardPerfInfo();
				std::string m_desthtml;
		
		
		};
