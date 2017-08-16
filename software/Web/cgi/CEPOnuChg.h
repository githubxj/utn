using namespace std;

class CEPOnuChg :public CgiRequest
	{
		public:
			
			CEPOnuChg():CgiRequest()
				{}
				
			inline ~CEPOnuChg()
			{}
			
			virtual void output();
			
		private:
			int updataOnuInfo();
			std::string m_desthtml;
			
		
		};
