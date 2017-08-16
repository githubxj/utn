#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "include/ConfigInfo.h"
#include "include/CGISession.h"

int main(void)
{
    CTools t;

    fprintf(stdout, "Content-type:text/html\n\n");
    char *data = getenv("QUERY_STRING");
    char* CltIpAddr = getenv("REMOTE_ADDR");
    char oldPasswd[PASSWD_LEN], newPasswd_src[PASSWD_LEN], new2Passwd[PASSWD_LEN];
    char newPasswd_des[PASSWD_LEN];
    char src[MAX_LEN][MAX_LEN];
    char des[MAX_LEN][MAX_LEN];
    char tmp[MAX_LEN][MAX_LEN];
    char sessionID[SESSIONID_LEN];
 
    int i = 0, line = 0;
    FILE *fp;
    memset(src, 0, sizeof(src));
    memset(des, 0, sizeof(des));
    memset(tmp, 0, sizeof(tmp));

    memset(oldPasswd, 0, sizeof(oldPasswd));  //用户输入的旧密码
    memset(newPasswd_src, 0, sizeof(newPasswd_src)); //用户输入的新密码1
    memset(newPasswd_des, 0, sizeof(newPasswd_des));
    memset(new2Passwd, 0, sizeof(new2Passwd));  //用户输入的新密码2
    memset(sessionID, 0, sizeof(sessionID));

    if (sscanf(data, "oldPasswd=%[^&]&newPasswd=%[^&]&new2Passwd=%[^&]&WSessionID=%[^&]", oldPasswd, newPasswd_src, new2Passwd, sessionID) != CONFIG_PARAM_CNT)
    {
        fprintf(stdout, "param cnt is error!");
	return 0;
    }
    CCGISession* cs = new CCGISession();                  
    int ret = 0;
    if(ret=cs->IsExistIDForSession(sessionID, CltIpAddr))
    {
       delete cs;
       fprintf(stdout, "session timeout!");
       return 0;
    }
    t.unencode(newPasswd_src, strlen(newPasswd_src), newPasswd_des); 
    t.str_replace(oldPasswd, SECTION_MAX_LEN, " ", "");
    t.str_replace(newPasswd_des, SECTION_MAX_LEN, " ", "");

    if ((fp = fopen(WEB_CONF_FILE, "r+w")) == NULL)
    {
        fprintf(stdout, "打开web.conf失败,请确认此配置文件是否存在.:%s", WEB_CONF_FILE); 
	return -1;
    }
    while (fgets(src[i], sizeof(src[i]), fp))
    {
        strncpy(tmp[i], src[i], sizeof(src[i]));
	if (strstr(src[i], "PASSWD"))
	{
            memset(des[i], 0, sizeof(des[i]));
            /* MU每次fgets后会在后面加上'\n'，比较时驱除\n和空格 */
            t.str_replace(src[i], MAX_LEN, "\n", "");
            t.str_replace(src[i], MAX_LEN, " ", "");
            if(strcmp(oldPasswd, rindex(src[i], '=')+1))
            {
               fprintf(stdout, "<P>原密码不正确!\n", rindex(src[i], '=')+1, oldPasswd);
               return -1;
            }
            else
               sprintf(des[i], "PASSWD=%s\n", newPasswd_des);
	}
        else
        {
             memset(des[i], 0, sizeof(des[i]));
             memcpy(des[i], tmp[i], sizeof(tmp[i]));
        }
	line = i++;
    }
    rewind(fp);
    for (i = 0; i <= line; i++)
         fputs(des[i], fp); 
    fclose(fp);
    fprintf(stdout, "<P> 修改成功!"); 
    return 0;
}

