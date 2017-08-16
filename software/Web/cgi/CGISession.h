/**
 * Title: CGI Session����
 *
 * Description: ���ļ�����ʽ��ʵ��CGI session�Ĵ��������ٵĹ��ܡ�
 *
 * @author momo 
 * @version 1.0
 */

#ifndef CGI_SESSION__H
#define CGI_SESSION__H

#include "const.h" 


class CCGISession
{

public:
   CCGISession(); 
   CCGISession(const char *ipAddr);
   virtual ~CCGISession();

/*  �ⲿ����һ��ʹ������ĽӿھͿ�����*/
   int  CreateSession();                           //  ����session
   int  DestorySession(const char* sessionID);     //  ����session
/*  ͨ��sessionID,�жϵ�ǰ�û��Ƿ�Ϸ� */
   int  IsExistIDForSession(const char* sessionID, const char* ipAddr); 
   int  IsExistIPForSession(const char* client_ip);
/**************************************/   

   int  IsTimeout(const int currTime);
   bool CheckIPValid(const char* ipAddr);
   int  GetCurrTime();

   void ErrPrintInfo(const int errCode);
   char* GetSessionID();
private:
   int  CrtSessionFile();
   int  CheckIPExist(const char* ipAddr);           
   int  AlterClientCnt(const char* fullName, const int modifyValue);
   int  DelSessionFile(const char *sid);
   int  CheckSessionExist(const char* sessionID, const char* ipAddr);
   int  CheckFileExists(const char *sid);
   int  AlterTimeout(const char* fullName, const char* ipAddr);

   char sessionID[SESSIONID_LEN];   // session��� 
   char clientIP[IPADDR_LEN];       // ����Ŀͻ���IP��ַ 
   long currTime;                   // ��¼ÿ�β����ĵ�ǰʱ��, ��λ�� 
   long affectClientCnt;            // ��ǰsession�����Ŀͻ���(IE)�ĸ��� 
   bool bflag;                      // ��־

};

#endif

