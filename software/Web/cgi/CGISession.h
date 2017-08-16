/**
 * Title: CGI Session基类
 *
 * Description: 以文件的形式，实现CGI session的创建、销毁的功能。
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

/*  外部请求一般使用下面的接口就可以了*/
   int  CreateSession();                           //  创建session
   int  DestorySession(const char* sessionID);     //  销毁session
/*  通过sessionID,判断当前用户是否合法 */
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

   char sessionID[SESSIONID_LEN];   // session编号 
   char clientIP[IPADDR_LEN];       // 请求的客户端IP地址 
   long currTime;                   // 记录每次操作的当前时间, 单位秒 
   long affectClientCnt;            // 当前session关联的客户端(IE)的个数 
   bool bflag;                      // 标志

};

#endif

