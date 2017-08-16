#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ioctl.h>

#define WATCHDOG_IOCTL_BASE     'W'
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)


const char wdt_dev[] = "/dev/wdt";
int wdt_fd;

int main(int argc, char *argv[])
{	

	unsigned int timeout;
	int ret;
	int loop_count;
	
			
	wdt_fd = open(wdt_dev, O_RDWR);
	if (wdt_fd  < 0) {
		printf("Can't Open file:%s!\n", wdt_dev);
		exit(1);
	}
		
	timeout=5;
	ret=ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);
	if(ret < 0)
		printf("Watch dog feed failed\n");
	while(1)
	{
		
		ret=ioctl(wdt_fd, WDIOC_KEEPALIVE, &timeout);
		if(ret < 0)
			printf("Watch dog feed failed\n");
		usleep(500000);
		usleep(500000);
		
		loop_count++;
	}
	
	close(wdt_fd);
	
	return 0;
}


