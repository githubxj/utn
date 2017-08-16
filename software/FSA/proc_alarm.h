#ifndef __PROC_ALARM__
#define __PROC_ALARM__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define startflag "$$MessageStart$$"

#define CHECK_INTERVAL 	10000
#define CHECK_COUNT 	(1000000/CHECK_INTERVAL)
#define TAG_CHKPOINT 	(5*CHECK_COUNT)
#define TAG_TIMEDOUT 	(10*CHECK_COUNT)
#define TAG_WAITTIME 	(15*CHECK_COUNT)
#define CON_IDLE		(10*CHECK_COUNT)
#define CH				4

//minum alarm holding time: ALARM_THRESHOLD*CHECK_INTERVAL us
#define ALARM_THRESHOLD  10

#define MAX_ALM_SER		26125
#define MIN_ALM_SER			16125

#define ALARM_TYPE_IO		11
#define ALARM_TYPE_MD		12
#define ALARM_TYPE_VLOST	13
#define ALARM_TYPE_SVR		14
#define ALARM_TYPE_CLEAR	90


typedef struct _AlarmParam
{
  char svrip[16];
  unsigned short svrport;
  unsigned int   io_flag1; /* onu io group 1 */
  unsigned int   io_flag2; /* onu io group 2 */
  unsigned int   io_flag3; /* onu io group 3 */
  unsigned int   io_flag4; /* onu io group 4 */
  unsigned int   io_flag5; /* onu io group 5 */
  unsigned int   io_flag6; /* onu io group 6 */
  unsigned int 	 io_mode; /*0: High input voltage report as alarm, 1: no voltage report as alarm */
  unsigned int 	 rpt_format;  /* report format 0: Short format, 1: Covond fomat */
  int area;
  int devid;
  int devid_step;
  int nodeid;
  int alarm_lev;
} AlarmParam;

typedef struct _AlarmToMu
{
  char start_flag[16]; //$$MessageStart$$
  uint8_t version;
  uint32_t serno;
  uint16_t msgtype;
  uint16_t subtype;
  uint32_t msglenth;
  uint32_t result;
  uint32_t nodeId;
  uint32_t devId;
  uint32_t almId;   //告警事件ID
  uint32_t almRank; //告警级别
  uint16_t almtype; //告警类型
  uint16_t almdescriptionId;  //告警描述码
  uint16_t year;
  uint8_t almtime[10];   
  uint8_t  des[256];  
} __attribute__((__packed__)) AlarmToMu;

typedef struct _AlarmShort
{
  char start_flag; //$
  char msg_type; //A:alarm, R:restore, O:set output open, C: set output close
  char lb; //(
  char id[3]; //000
  char rb; //)
  char stop_flag; //#
} __attribute__((__packed__)) AlarmShort;

typedef struct AlarmQElemnt
{
	unsigned short  	type;
	unsigned int 		devid;
	unsigned int 		format;
}AlarmQElemnt;

typedef struct AlarmQueue
{
AlarmQElemnt 	queue[256];
unsigned char		head; //use one byte to indicate the postion
unsigned char		tail;
}AlarmQueue;

void* alarm2Net(void);
void* recv_alarm_7000(void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PROC_ALARM__ */

