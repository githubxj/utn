#ifndef CGI_TOOLS__H
#define CGI_TOOLS__H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>		
#include <fcntl.h>

#include "const.h"

#define BCM5645_NAME	"/dev/switch"
#define SW_IOCTL_MAGIC 'L'
#define IOCTL_GET_L3_CAP  _IOW( SW_IOCTL_MAGIC, 0x81, unsigned int)	

class CTools {

  public:
    CTools() {

    };

    virtual ~ CTools()
    {

    };


    int GetWebConf(const char* section, char* value)
    {
       char src[MAX_LEN];
       FILE* fp;
       memset(src, 0, sizeof(src));
       if ((fp = fopen(WEB_CONF_FILE, "r+")) == NULL)
       {
           printf("open web conf faild");
	   return -1;
       }
       while (fgets(src, sizeof(src), fp))
       {
          if(strstr(src, section))
          {
             strcpy(value, rindex(src, '=')+1);
             value[strlen(value)-1] = '\0';
             //printf("value:%s\n", value);
             return 0;
          }
       }        
       return -1;
    }

	static int hasL3Capability()
	{
    		int fd;
		int l3cap;
		
		fd = open(BCM5645_NAME, O_RDWR);
		if ( -1 == fd )
			return 0;
		
		if ( 0 != ioctl(fd, IOCTL_GET_L3_CAP, &l3cap ) )
			return 0;
		
		return(l3cap);
	}

    /*  string replace */
    int str_replace(char *str, size_t n, const char *src, const char *des) {
	size_t old_len = strlen(src);
	size_t new_len = strlen(des);
	const char *end = str + n;

	while (str = strstr(str, src))
	{
	    size_t len = strlen(str);
	    if (str + len + new_len - old_len < end)
	    {
		memmove(str + new_len, str + old_len, len + 1);
		memcpy(str, des, new_len);
		str += new_len;
	    } else
	    {
		return -1;
	    }
	}

	return 0;
    }
    void unencode(char *src, int size, char *dest) {
	int i = 0;
	for (; i <= size; i++, src++, dest++)
	{
	    if (*src == '+')
		*dest = ' ';

	    else if (*src == '%')
	    {

		int code;

		if (sscanf(src + 1, "%2x", &code) != 1)
		    code = '?';

		*dest = code;

		src += 2;
	    }

	    else
		*dest = *src;
	}

	*dest = ' ';

	*++dest = ' ';
    }

    /* 0: file exits */
    int fileIfExits(const char* fileName)
    {
        return access(fileName, 0);
    }

  private:


};

#endif
