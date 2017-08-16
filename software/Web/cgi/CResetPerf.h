using namespace std;

class CResetPerf :public CgiRequest
	{
		public:
			CResetPerf():CgiRequest()
				{}
				
				inline ~CResetPerf()
				{}
				
				virtual void output();
				
			private:
				int ResetPerf();
				std::string m_desthtml;
		
		
		};
