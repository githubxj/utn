/* vi: set sw=8 ts=8: */
#include <linux/autoconf.h> 
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <linux/major.h>
#include <linux/types.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <asm/param.h>
#include <math.h>
#include <unistd.h>

typedef struct _PPC405_RTC_TIME 
{
	unsigned char years;
	unsigned char months;
	unsigned char dates;
	unsigned char hours;
	unsigned char minutes;
	unsigned char seconds;
} PPC405_RTC_TIME, *P_PPC405_RTC_TIME;

void set_rtc_time(P_PPC405_RTC_TIME time);
void get_rtc_time(P_PPC405_RTC_TIME time);

void set_rtc_time(P_PPC405_RTC_TIME time)
{
	int retval;
	struct rtc_time rtc_tm;
	time_t tm;
	struct tm tm_time;
	int fd;
	fd = open ("/dev/rtc_8025", O_RDWR);
	if(fd < 0)
	{
		printf("Failed to open /dev/rtc_8025.\n");
		return;
	}

	rtc_tm.tm_year = time->years + 2000 - 1900;
	rtc_tm.tm_mon = time->months - 1;
	rtc_tm.tm_mday = time->dates;
	rtc_tm.tm_hour = time->hours;
	rtc_tm.tm_min = time->minutes;
	rtc_tm.tm_sec = time->seconds;

	retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	if (retval == -1) {
        	perror("RTC_SET_TIME ioctl");
	        //exit(errno);
		close(fd);
                return;
	}
//	printf("fd = %d retval = %d\n", fd, retval);
	tm_time.tm_year = rtc_tm.tm_year;
	tm_time.tm_mon = rtc_tm.tm_mon;
	tm_time.tm_mday = rtc_tm.tm_mday;
	tm_time.tm_hour = rtc_tm.tm_hour;
	tm_time.tm_min = rtc_tm.tm_min;
	tm_time.tm_sec = rtc_tm.tm_sec;
	tm = mktime(&tm_time);
	stime(&tm);
	close(fd);
}

void get_rtc_time(P_PPC405_RTC_TIME time)
{
	int fd;
	int retval;
	struct rtc_time rtc_tm;
	fd = open ("/dev/rtc_8025", O_RDWR);
        if(-1 == fd)
        {
	        perror("RTC_RD_TIME open");
                return;
        }
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if(retval == -1) {
	        perror("RTC_RD_TIME ioctl");
	        //exit(errno);
	}
        else
        {
		time->years = rtc_tm.tm_year + 1900 - 2000;
		time->months = rtc_tm.tm_mon + 1;
		time->dates = rtc_tm.tm_mday;
		time->hours = rtc_tm.tm_hour;
		time->minutes = rtc_tm.tm_min;
		time->seconds = rtc_tm.tm_sec;
	}
	close(fd);
}


int main(int argc, char *argv[])
{	
	PPC405_RTC_TIME time;
	PPC405_RTC_TIME set_time;
	char *s, *t;
   	 int c;
	  char buf[256];
	 unsigned long year, mon, mday, hour, min, sec;


	if((argc >= 2) && (strcmp(argv[1], "-h") == 0))
	{
		printf("Usage: rtc_time <options> \n");
		printf("Options: \n");
		printf("\t -g get the system time. \n ");
		printf("\t -s YYYY-MM-DD HH:MM:SS \n\t set the system time. \n ");
		
		return 1;
	}
	
	if((argc >= 2) && (strcmp(argv[1], "-g") == 0))
	{
		get_rtc_time( &time );
		printf("  current time:  %02d:%02d:%02d   %02d.%02d.%d\n",
                        time.hours, time.minutes, time.seconds,
                        time.months, time.dates, time.years+2000);
		
	}

	if((argc >= 3) && (strcmp(argv[1], "-s") == 0))
	{
		sscanf(argv[2], "%d-%d-%d", &year, &mon, &mday);

		if(argc > 3)
			sscanf(argv[3], "%d:%d:%d", &hour, &min, &sec);
		else
		{
			hour=0;
			min=0;
			sec=0;
		}
			
	        if(sec  > 59 || min > 59 || hour > 23 ||
	           mday == 0 || mday > 31 || mon == 0 || mon > 12 || year > 2099 || year < 2000)
	        {
			printf("Error time parameters! Please enter again!\n");
			return 1;
	        }

	        set_time.years = year-2000;
	        set_time.months = mon;
	        set_time.dates = mday;
	        set_time.hours = hour;
	        set_time.minutes = min;
	        set_time.seconds = sec;
	        set_rtc_time( &set_time );
		get_rtc_time( &time );
		printf("  after set time is:  %02d:%02d:%02d   %02d.%02d.%d\n",
                        time.hours, time.minutes, time.seconds,
                        time.months, time.dates, time.years+2000);


		
	}
	
	get_rtc_time( &time );
	sprintf(buf, "/bin/date %02d%02d%02d%02d%4d.%02d", time.months, time.dates , time.hours , time.minutes , time.years + 2000, time.seconds);
	system(buf);
	
	return 0;
}


