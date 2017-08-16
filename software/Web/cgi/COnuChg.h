using namespace std;

class COnuChg :public CgiRequest
	{
		public:
			
			COnuChg():CgiRequest()
				{}
				
			inline ~COnuChg()
			{}
			
			virtual void output();
			
		private:
			int updataOnuInfo();
			int Addvlan();
			int Modifyvlan();
			int Savevlan();
			int Delvlan();
			std::string m_desthtml;
			
		
		};
