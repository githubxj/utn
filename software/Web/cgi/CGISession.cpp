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

//��֤�û���½����ʱʹ�á�
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

//���û�����Ҫ�����µ�sesion�ļ�ʱ,(һ��ֻ�ڶ��û����оֲ���֤ʱʹ��)
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
    ����:
          �����յ�½����ʱ�����ô˽ӿڴ���session����.
          �˽ӿڻ�����ƥ�䵱ǰ����session�ļ��У�IP��ַ��ͬ���û���
          ���ڹ��캯���б������û�IP��ַ��ʼ���� 
          ����������ͬIP��session�������µ�session�ļ���
          ��������ͬIP����session�ļ��е�ClientCnt++, ���³�ʱʱ�䡣
    ����ֵ:
          SUCCESS:        �ɹ�����
          ERROR:          ����ʧ�ܡ�
          IPEXIST:        IP��ַ�Ѿ�����
    sessionID����this->sessionID
*/

int CCGISession::CreateSession()
{
    if (false == bflag)
    {
	//cout << "CreateSession::ip is null" << endl;
	return ERROR;
    }
    //��������ͬIP��session�ļ��������µ�session
    /* ע: ������Բ����IP�Ƿ���� ����Ϊindex.html�ڵ�½֮ǰ�Ѿ��ȼ���ˡ�*/
    int ret = IsExistIPForSession(this->clientIP);
    if (IPNOTEXIST== ret)
    {
	int sec = GetCurrTime();
	this->currTime = sec;
	srand(sec);
	/* sessionID: ���ݵ�ǰ���������������������β������IP��ַĩβ�� */
	sprintf(this->sessionID, "%d%s", rand(),
		rindex(this->clientIP, '.') + 1);
	this->affectClientCnt++;
	//�����µ�session�ļ���������ǰsession�����е�����д�롣
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
		    //IP��ͬ
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
	//�ҵ�session�ļ�����ʾ��ǰ�û��ǺϷ���½���û������Ĵ�session��ʧЧʱ��
	//����˴�����Ŀͻ���IP��ַ����������session�ļ��е��Ƿ�һ�£�����һ�£���ʾ��ǰ����Ƿ�
	return AlterTimeout(fullName, ipAddr);
    }
}

/* �����е�session�ļ������ip��ַ�Ƿ�����ͬ�ģ�
   ����ͬ��ʾ�û����˶��IE����,���Ҽ���Ƿ�ʱ���ͷ�session�ļ������ޣ� ����session�ļ�  
   ��û����ͬ��IP�������������µ�session�ļ���
*/
int CCGISession::IsExistIPForSession(const char* ipAddr)
{
    if(NULL==ipAddr)
       return ERROR;
    return CheckIPExist(ipAddr);
}

/*
   ����: 
        �˽ӿ����û��������ʱ����ִ��CGIǰ����ִ�д˽ӿ�ȥ�жϵ�ǰ�����Ƿ�Ϸ���
        ���sessionID�Ƿ��Ѿ�����, �����ڲ���û�г�ʱ��������û��Ĳ������󣬷��򲻽��ܡ�
        ��sessionID��ip����ͬʱ����ʾ��ǰ�û������ǺϷ��ġ�
   ����ֵ:
        IPNOTEXIST:        ��ʾ��ǰ��������Ŀͻ���IP��ַ����Ӧ��session�ļ���һ�¡� 
        FILENOTEXIST:      ��ʾ�����sessionID�Ƿ���   
        ERROR:             ��ʾ�ڲ�ѯ�Ĺ����в�����δ֪�Ĵ���
        SUCCESS:           ��ʾ�û�������Ϸ���Ч��(�Ѹ����û��ĳ�ʱʱ��)         
        ISTIMEOUT          ��ʾ��ǰ�û���session���ڣ����ҿͻ���IP��ַҲ�Ϸ�����������ʱ���Ѿ���ʱ�� 
    
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
     ����: 
         �����û�ע������
         ���ȶ�ȡsession�ļ��е����ӿͻ��˸�����������<=1 ��ʾ����ɾ����ǰsession�ļ���
         >1 ��ʾ�ж��IE���Ӵ�session�ļ���ֻ��Ҫ����cnt��          
     ����ֵ: 
         
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
	//���޶��IE���Ӵ�session����ֱ��ɾ�����ļ���  
	DelSessionFile(fullName);
//    }
    return 0;
}

//���ص�ǰsession�ļ����޸�ǰ�����ӵĿͻ��˸�����������1�����Զ������޸�ֵ��
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
	    //ip������ͬ, ��ʾ����Ƿ���
            //printf("<P>AlterTimeout:: src:%s, des:%s", src, des);
	    if (strcmp(src, des))
		return IPNOTEXIST;
	}
	if (strstr(src, CURRTIME_SECTION))
	{
	    int tmpTime = 0;
	    //����Ƿ�ʱ
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
