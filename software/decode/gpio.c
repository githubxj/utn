#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ioctl.h>

#define 	MODE_DIR			1
#define 	MODE_SET			2
#define 	MODE_READ			3
#define 	MODE_SLOT			4

typedef struct {
	u_int  group;	
	u_int  ctrl_pin;	
	u_int  data_out;	
	u_int  data_in;	
	
	u_int  int_clear;		
	u_int  data_direct;			
	u_int  int_enable;			
	u_int  int_trigger;
	u_int  int_both;			
	u_int  int_riseneg;			
} gpio_params;

/* Use 'g' as magic number */
#define IOC_MAGIC  'g'

#define GPIO_SET_DATA_OUTPUT 	_IOWR(IOC_MAGIC, 8, gpio_params)
#define GPIO_READ_DATA_INPUT 	_IOWR(IOC_MAGIC, 9, gpio_params)
#define GPIO_SET_DATA_DIRECT 	_IOWR(IOC_MAGIC, 10, gpio_params)
#define GPIO_SET_INT_FUNC	_IOWR(IOC_MAGIC, 11, gpio_params)
#define GPIO_SET_INT    	_IOWR(IOC_MAGIC, 12, gpio_params)
#define GPIO_CLEAR_INT    	_IOWR(IOC_MAGIC, 13, gpio_params)

const char gpio_dev[] = "/dev/gpio";

int gpio_fd;

void printUsage()
{
	printf("Usage: gpio  options\n");
	printf("Options: \n");
	printf("\t -i  gpio index(0~31). \n ");
	printf("\t -s set gpio data. \n ");
	printf("\t -r read gpio data. \n ");
	printf("\t -t read slot id. \n ");
	printf("\t -d set direction. 1-output, 0-input \n ");
}

int main(int argc, char *argv[])
{	
	extern char *optarg;
	extern int optind;
	int c;
	int mode = MODE_READ;
	int value = 0;
	int gpio_idx = 0xFFFFFFFF;
	gpio_params 	parm;
	int ret;
	
	while ((c = getopt(argc, argv, "i:s:rd:ht")) != EOF) {
		switch(c) {
		case 'i':
			if(optarg != NULL)
				sscanf(optarg, "%d", &gpio_idx);
			break;
		case 's':
			mode=MODE_SET;
			if(optarg != NULL)
				sscanf(optarg, "%x", &value);
			break;
		case 'r':
			mode=MODE_READ;
			break;
		case 't':
			mode=MODE_SLOT;
			break;
		case 'd':
			mode=MODE_DIR;
			if(optarg != NULL)
				sscanf(optarg, "%x", &value);
			break;
		case 'h':
			printUsage();
			exit(1);
		}
	}
  
	/*initialize the 485 communication port */
	gpio_fd = open(gpio_dev, O_RDWR);
	if (gpio_fd  < 0) {
		printf("Can't Open file:%s!\n", gpio_fd);
		exit(1);
	}

	parm.group = 0;
	if(gpio_idx == 0xFFFFFFFF)
	{
		parm.ctrl_pin = gpio_idx;
		parm.data_out = value;
		parm.data_direct = value;
	} else {
		parm.ctrl_pin = 1 << gpio_idx;
		parm.data_out =  value << gpio_idx;
		parm.data_direct = value << gpio_idx;
	}
		
	switch(mode) {
		case MODE_DIR:
			printf("Set GPIO direction: PIN=0x%08x, DIR=0x%08x\n", parm.ctrl_pin, parm.data_direct);
			ret=ioctl(gpio_fd, GPIO_SET_DATA_DIRECT, &parm);
			if(ret < 0)
				printf("Set GPIO direction failed\n");
			break;
		case MODE_READ:
			printf("Read GPIO value: PIN=0x%08x\n", parm.ctrl_pin);
			ret=ioctl(gpio_fd, GPIO_READ_DATA_INPUT, &parm);
			if(ret < 0)
				printf("READ GPIO value failed\n");
			else
			{
				if(gpio_idx == 0xFFFFFFFF)
					printf("0x%08x\n", parm.data_in);
				else{
					if(parm.data_in & (1 << gpio_idx))
						printf("1\n");
					else
						printf("0\n");						
				}
			}
			break;
		case MODE_SLOT:
			parm.ctrl_pin = 0x27000;
			ret=ioctl(gpio_fd, GPIO_READ_DATA_INPUT, &parm);
			if(ret < 0)
				printf("READ GPIO value failed\n");
			else
			{
				printf("Datain:%08x\n", parm.data_in);
				value =0;
				if(parm.data_in & 0x1000)
					value |= 0x01;
				if(parm.data_in & 0x2000)
					value |= 0x100;
				if(parm.data_in & 0x4000)
					value |= 0x10;
				if(parm.data_in & 0x20000)
					value |= 0x1000;
			}
			
			printf("%04x\n", value);

	
			break;
		case MODE_SET:
			printf("Set GPIO value: PIN=0x%08x, VAL=0x%08x\n", parm.ctrl_pin, parm.data_out);
			ret=ioctl(gpio_fd, GPIO_SET_DATA_OUTPUT, &parm);
			if(ret < 0)
				printf("Set GPIO value failed\n");
			break;
		default:
			printUsage();			
	}
	
        return 0;
}

