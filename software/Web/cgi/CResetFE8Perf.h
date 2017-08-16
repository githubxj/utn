using namespace std;

class CResetFE8Perf :public CgiRequest
	{
		public:
			CResetFE8Perf():CgiRequest()
				{}
				
				inline ~CResetFE8Perf()
				{}
				
				virtual void output();
				
			private:
				int ResetFe8Perf();
				std::string m_desthtml;
		
		
		};