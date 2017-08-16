using namespace std;

class CGetPerfFe8 :public CgiRequest
	{
		public:
			CGetPerfFe8():CgiRequest()
				{}
				
				inline ~CGetPerfFe8()
				{}
				
				virtual void output();
				
			private:
				int getCardFe8PerfInfo();
				std::string m_desthtml;
		
		
		};