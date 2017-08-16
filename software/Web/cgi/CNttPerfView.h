using namespace std;

class CNttPerf :public CgiRequest
	{
		public:
			CNttPerf():CgiRequest()
				{}
				
				inline ~CNttPerf()
				{}
				
				virtual void output();
				
			private:
				int getNttPerfInfo();
				std::string m_desthtml;
		
		
		};
