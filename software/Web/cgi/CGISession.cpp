#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;


#include "CGISession.h"

//验证用户登陆请求时使用。
CCGISession::CCGISession(const char *ipAddr)
{
    if (false == CheckIPValid(ipAddr))
    {
	//cout << "ipAddr is invalid" << endl;
	this->bflag = false;
	return;
    }
    memset(this->sessionID, 0, sizeof(sessionID));
    memset(this->clientIP, 0, sizeof(clientIP));
    memcpy(this->clientIP, ipAddr, sizeof(this->clientIP));
    this->affectClientCnt = 0;
    this->currTime = 0;
    this->bflag = true;
}

//当用户不需要创建新的sesion文件时,(一般只在对用户进行局部验证时使用)
CCGISession::CCGISession()
{
    memset(this->clientIP, 0, sizeof(clientIP));
    memset(this->sessionID, 0, sizeof(sessionID));
    this->affectClientCnt = 0;
    this->currTime = 0;
    this->bflag = false;
}

CCGISession::~CCGISession()
{
}

char* CCGISession::GetSessionID()
{
    return this->sessionID;
}
bool CCGISession::CheckIPValid(const char *ipAddr)
{
    return inet_addr(ipAddr) == -1 ? false : true;
}

/**
    功能:
          当接收登陆请求时，调用此接口创建session对象.
          此接口会首先匹配当前所有session文件中，IP地址相同的用户。
          故在构造函数中必须用用户IP地址初始化。 
          若不村在相同IP的session，创建新的session文件。
          若存在相同IP，将session文件中的ClientCnt++, 更新超时时间。
    返回值:
          SUCCESS:        成功创建
          ERROR:          创建失败。
          IPEXIST:        IP地址已经存在
    sessionID放入this->sessionID
*/

int CCGISession::CreateSession()
{
    if (false == bflag)
    {
	//cout << "CreateSession::ip is null" << endl;
	return ERROR;
    }
    //不存在相同IP的session文件，创建新的session
    /* 注: 这里可以不检查IP是否存在 ，因为index.html在登陆之前已经先检查了。*/
    int ret = IsExistIPForSession(this->clientIP);
    if (IPNOTEXIST== ret)
    {
	int sec = GetCurrTime();
	this->currTime = sec;
	srand(sec);
	/* sessionID: 根据当前秒数产生随机数，并且在尾部连接IP地址末尾段 */
	sprintf(this->sessionID, "%d%s", rand(),
		rindex(this->clientIP, '.') + 1);
	this->affectClientCnt++;
	//创建新的session文件，并将当前session对象中的内容写入。
	this->CrtSessionFile();
	return SUCCESS;
    } else if (IPEXIST== ret)
    {
	//cout << "find same clientIP" << endl;
	return IPEXIST;
    } else
    {
	//cout << "CrtSessionFile::error" << endl;
	return ERROR;
    }
}

int CCGISession::CrtSessionFile()
{
    if (this->clientIP == NULL || this->sessionID == NULL)
    {
	//cout << "CrtSessionFile::not init session." << endl;
	return ERROR;
    }
    char fullName[MAX_LEN];
    char session[MAX_LEN];
    char clientIP[MAX_LEN];
    char curr[MAX_LEN];
    char clientCnt[MAX_LEN];
    memset(fullName, 0, sizeof(fullName));
    memset(curr, 0, sizeof(curr));
    memset(clientCnt, 0, sizeof(clientCnt));
    memset(session, 0, sizeof(session));
    memset(clientIP, 0, sizeof(clientIP));

    sprintf(session, "%s%s\n", SESSIONID_SECTION, this->sessionID);
    sprintf(clientIP, "%s%s\n", CLIENTIP_SECTION, this->clientIP);
    sprintf(curr, "%s%ld\n", CURRTIME_SECTION, this->currTime);
    sprintf(clientCnt, "%s%ld\n", CLIENTCNT_SCTION, this->affectClientCnt);

    sprintf(fullName, "%s%s", this->sessionID, SUFNAME);
    FILE *fp;
    if ((fp = fopen(fullName, "w+")) == NULL)
    {
	//cout << "CrtSessionFile::open file faild" << endl;
	return ERROR;
    }
    //printf("sessionID:%s, clientIP:%s, currTime:%s, clientCnt:%s\n", this->sessionID, this->clientIP, curr, clientCnt); 
    fputs(session, fp);
    fputs(clientIP, fp);
    fputs(curr, fp);
    fputs(clientCnt, fp);
    fclose(fp);
    return SUCCESS;
}

int CCGISession::CheckIPExist(const char* client_ip)
{
    struct dirent *ptr;
    DIR *dir = opendir(CURRENTDIR);
    char fileName[MAX_LEN];
    char input[MAX_LEN][MAX_LEN];
    memset(input, 0, sizeof(input));
    memset(fileName, 0, sizeof(fileName));
    int i = -1; 
    while ((ptr = readdir(dir)) != NULL)
    {
	if (strstr(ptr->d_name, SUFNAME))
	{
	    //printf("find it. fileName:%s\n", ptr->d_name);
	    FILE *fp = NULL;
	    char src[MAX_LEN];
	    char des[MAX_LEN];
	    char ipAddr[MAX_LEN];
	    bool flag = false;
	    memset(src, 0, sizeof(src));
	    memset(this->sessionID, 0, sizeof(this->sessionID));
	    if ((fp = fopen(ptr->d_name, "r+")) == NULL)
	    {
		printf("CheckIPExist::open file faild"); 
		closedir(dir);
		return ERROR;
	    }
            i = 0;
	    while (fgets(src, sizeof(src), fp))
	    {
		if (strstr(src, SESSIONID_SECTION))
		{
		    strcpy(this->sessionID, rindex(src,'=')+1);
                    *(strchr(this->sessionID, '\n')) = '\0';
                    strcpy(input[i++], src); 
		    continue;
		}

		if (strstr(src, CLIENTIP_SECTION))
		{
		    //IP相同
                    memset(ipAddr, 0, sizeof(ipAddr));
                    strcpy(ipAddr, rindex(src, '=') + 1);
                    *(strchr(ipAddr, '\n')) = '\0';
                    if(!strcmp(ipAddr, client_ip))
                    {
                       flag = true;
                       strcpy(input[i++], src); 
		       continue;
                    }
                    else
                       break; 
		}

                if (strstr(src, CURRTIME_SECTION) && flag)
		{
                    this->currTime = GetCurrTime();
		    sprintf(input[i++], "%s%ld\n", CURRTIME_SECTION, this->currTime);
		    continue;
		}

		if (strstr(src, CLIENTCNT_SCTION) && flag)
		{
		    int cnt = 0;
		    memset(des, 0, sizeof(des));
		    memcpy(des, rindex(src, '=') + 1,strlen(src) - strlen(CLIENTCNT_SCTION));
		    if (ERROR != (cnt = atoi(*des == '\0' ? "-1" : des)))
                    {
			this->affectClientCnt = ++cnt;
		        sprintf(input[i++], "%s%d", CLIENTCNT_SCTION, cnt);
                        //closedir(dir);
		        //fclose(fp); 
                        continue;
                    }
		    else
		    {
			printf("CheckIPExist::file content error");
			fclose(fp);
			closedir(dir);
			return ERROR;
		    }
		}
	    }//end while
            if(flag)
            {
                //printf("i:%d\n", i);
                rewind(fp);
                for(int cnt = 0; cnt < i; cnt++) 
                {
                    //printf("input[%d]:%s", cnt, input[cnt]);
                    fputs(input[cnt],fp);
                }
                //closedir(dir);
	        //fclose(fp);
                return IPEXIST;
            }
            fclose(fp);
       }//end if
    }//end while
    closedir(dir);
    return IPNOTEXIST;
}


int CCGISession::CheckSessionExist(const char *sessionID,
				   const char *ipAddr)
{
    char fullName[MAX_LEN];
    memset(fullName, 0, sizeof(fullName));
    sprintf(fullName, "%s%s", sessionID, SUFNAME);
    if (FILENOTEXIST == CheckFileExists(fullName))
    {
	//cout << "CheckFileExists::file open faild, fullName:" << fullName << endl;
	return FILENOTEXIST;
    } else
    {
	//找到session文件，表示当前用户是合法登陆的用户，更改此session的失效时间
	//传入此次请求的客户端IP地址，检查和已有session文件中的是否一致，若不一致，表示当前请求非法
	return AlterTimeout(fullName, ipAddr);
    }
}

/* 打开所有的session文件，检查ip地址是否有相同的，
   若相同表示用户开了多个IE窗口,并且检查是否超时，释放session文件，若无， 复用session文件  
   若没有相同的IP，检查口令，则产生新的session文件。
*/
int CCGISession::IsExistIPForSession(const char* ipAddr)
{
    if(NULL==ipAddr)
       return ERROR;
    return CheckIPExist(ipAddr);
}

/*
   功能: 
        此接口在用户具体操作时，在执行CGI前首先执行此接口去判断当前请求是否合法。
        检查sessionID是否已经存在, 若存在并且没有超时，则接受用户的操作请求，否则不接受。
        当sessionID和ip都相同时，表示当前用户请求是合法的。
   返回值:
        IPNOTEXIST:        表示当前发起请求的客户端IP地址在相应的session文件不一致。 
        FILENOTEXIST:      表示请求的sessionID非法。   
        ERROR:             表示在查询的过程中产生了未知的错误。
        SUCCESS:           表示用户的请求合法有效。(已更新用户的超时时间)         
        ISTIMEOUT          表示当前用户的session存在，并且客户端IP地址也合法，但是请求时，已经超时。 
    
*/
int CCGISession::IsExistIDForSession(const char *sessionID,
				     const char *ipAddr)
{
    return CheckSessionExist(sessionID, ipAddr);
}

int CCGISession::CheckFileExists(const char *sid)
{
    return access(sid, 0);
}

int CCGISession::DelSessionFile(const char *sid)
{
    return unlink(sid);
}

/**
     功能: 
         处理用户注销请求。
         首先读取session文件中的连接客户端个数，若等于<=1 表示可以删除当前session文件。
         >1 表示有多个IE连接此session文件，只需要更新cnt。          
     返回值: 
         
*/
int CCGISession::DestorySession(const char *sessionID)
{
    assert(sessionID);
    char fullName[MAX_LEN];
    memset(fullName, 0, sizeof(fullName));
    sprintf(fullName, "%s%s", sessionID, SUFNAME);
/*
    if (1 < AlterClientCnt(fullName, -1))
	return SUCCESS;
    else
    {
*/
	//若无多个IE连接此session，则直接删除此文件。  
	DelSessionFile(fullName);
//    }
    return 0;
}

//返回当前session文件中修改前的连接的客户端个数，若大于1，则自动更新修改值。
int CCGISession::AlterClientCnt(const char *fullName,
				const int modifyValue)
{
    FILE *fp;
    if ((fp = fopen(fullName, "r+")) == NULL)
    {
	//cout << "AlterClientCnt::open file faild" << endl;
	return ERROR;
    }
    char src[MAX_LEN];
    char des[MAX_LEN];
    memset(src, 0, sizeof(src));
    memset(des, 0, sizeof(des));
    while (fgets(src, sizeof(src), fp))
    {
	if (strstr(src, CLIENTCNT_SCTION))
	{
	    int cnt = 0; 
            long offset=0;
	    memset(des, 0, sizeof(des));
	    memcpy(des, rindex(src, '=') + 1,
		   strlen(src) - strlen(CLIENTCNT_SCTION));
            offset = -(strlen(src));
	    if (ERROR != (cnt = atoi(*des== '\0' ? "-1" : des)) && cnt > 1)
	    {
		memset(des, 0, sizeof(des));
		sprintf(des, "%s%d  ", CLIENTCNT_SCTION, cnt+modifyValue);
                fseek(fp, offset, SEEK_CUR);
                fputs(des, fp);
                fclose(fp);
                return cnt;
	    } else
	    {
		//cout << "CheckIPExist::file content error" << endl;
		fclose(fp);
		return ERROR;
	    }
	}
    }
    fclose(fp);
    //cout << "AlterClientCnt::not found CLIENTCNT_SCTION!" << endl;
    return ERROR;
}

int CCGISession::AlterTimeout(const char *fullName, const char *ipAddr)
{
    FILE *fp;
    if ((fp = fopen(fullName, "r+")) == NULL)
    {
	//cout << "AlterTimeout::open file faild" << endl;
	return ERROR;
    }
    char src[MAX_LEN];
    char des[MAX_LEN];
    memset(src, 0, sizeof(src));

    while (fgets(src, sizeof(src), fp))
    {
	if (strstr(src, CLIENTIP_SECTION))
	{
	    memset(des, 0, sizeof(des));
	    sprintf(des, "%s%s\n", CLIENTIP_SECTION, ipAddr);
	    //ip若不相同, 表示请求非法。
            //printf("<P>AlterTimeout:: src:%s, des:%s", src, des);
	    if (strcmp(src, des))
		return IPNOTEXIST;
	}
	if (strstr(src, CURRTIME_SECTION))
	{
	    int tmpTime = 0;
	    //检查是否超时
	    memset(des, 0, sizeof(des));
	    memcpy(des, rindex(src, '=') + 1,
		   strlen(src) - strlen(CLIENTCNT_SCTION));
	    if (ERROR != (tmpTime = atoi(*des == '\0' ? "-1" : des)))
	    {
		if (ERROR == IsTimeout(tmpTime))
		{
		    //cout << "AlterTimeout::time out!" << endl;
		    fclose(fp);
		    return ISTIMEOUT;
		}
	    } else
	    {
		//cout << "AlterTimeout::file content error" << endl;
		fclose(fp);
		return ERROR;
	    }
	    sprintf(des, "%s%d", CURRTIME_SECTION, GetCurrTime());
            int offset = -(strlen(src));
            fseek(fp, offset, SEEK_CUR);
	    fputs(des, fp);
	    fclose(fp);
	    return SUCCESS;
	}
    }
    fclose(fp);
    return ERROR;
}

int CCGISession::GetCurrTime()
{
    return time((time_t *) NULL);
}

int CCGISession::IsTimeout(const int currTime)
{
    return (GetCurrTime() - currTime) >= TIMEOUT ? ERROR: SUCCESS;
}
