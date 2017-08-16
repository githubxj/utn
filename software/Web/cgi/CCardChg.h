using namespace std;

class CCardChg :public CgiRequest
	{
		public:
			CCardChg():CgiRequest()
				{}
				
				inline ~CCardChg()
				{}
				
				virtual void output();
				
			private:
				int updataCardInfo();
			  	const char *getCompress ( unsigned char type);
				const char * getStream ( unsigned char type );
				const char *getChannelNO(unsigned char channelno);
		
		};

