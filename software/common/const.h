// message start flag for 485 serial port communication
#define	COM_MSG_FLAG		0xFE

//card type define
#define CARD_TYPE_CTRL		0x01
#define CARD_TYPE_FSA			0x02
#define CARD_TYPE_FE8			0x03
#define CARD_TYPE_FSA155 		0x04
#define CARD_TYPE_ENCODE		0x05
#define CARD_TYPE_FSA600		0x06
#define CARD_TYPE_DECODE		0x07
#define CARD_TYPE_GE8         	0x08
#define CARD_TYPE_NTT        	0x09
#define CARD_TYPE_MS4V1H		0x0a
#define CARD_TYPE_EPN			0x0b
#define CARD_TYPE_DAT			0x0c
#define CARD_TYPE_HDC			0x0d
#define CARD_TYPE_SDE			0x0E    
#define CARD_TYPE_MSD			0x0F    
#define CARD_TYPE_M4S			0x10    
#define CARD_TYPE_M4H			0x11    
#define CARD_TYPE_NTT_S         0x12        // 20150115 add by xj


#ifdef __MTT__
#define CARD_TYPE_MTT        	0x09            // 201611 临时
#else
#define CARD_TYPE_MTT        	0x13            // 201611
#endif


#if 1 // 201708
#define CARD_TYPE_MTT_411       0x13            // 201611
#define CARD_TYPE_MTT_441       0x15            // 201611
#endif

#define CARD_TYPE_SDIENC        0x14

#define CARD_TYPE_UNKOWN 	    0x7F



//fan state
#define  		FAN_OFF		0
#define  		FAN_ON			1
#define  		FAN_FAIL		2
#define  		FAN_AUTO		8

//define onu STATUS
#define 	ONU_STATE_NOTUSE		0
#define 	ONU_STATE_ONLINEE	1
#define 	ONU_STATE_ONLINED	2
#define 	ONU_STATE_OFFLINE		3

//define onu network STATUS
#define 	ONU_NET_INVALIDE		0
#define 	ONU_NET_NEAREND		1
#define 	ONU_NET_FAREND		2
#define 	ONU_NET_RING			3
#define 	ONU_NET_RING_O1D		4
#define 	ONU_NET_RING_O2D		5

//define web page refresh type
#define 	REFRESH_TYPE_TREE		1
#define 	REFRESH_TYPE_FRAME	2
#define 	REFRESH_TYPE_TOPO		3

//card number
#define  CARD_TOTEL_NUM   		0x10

// define card number
#define 	MAX_CARD_NUM_UTN		13
#define 	CTRL_CARDNO			7
#define 	LCD_CARDNO			0x10
#define 	GW_CARDNO 			0x11
#define	BACK_CARDNO			0x12

//define max onu100 number and max port number
#define   MAX_ONU100_NUM		6
#define   MAX_PORT_NUM			8

//define alarm serverity
#define 	ALARM_SERVERITY_ERROR		0x01
#define 	ALARM_SERVERITY_WARNING		0x02
#define 	ALARM_SERVERITY_INFO			0x03

//define ether port working mode
#define	ETH_AUTO			    1
#define	ETH_100MFULL		    2
#define	ETH_100MHALF		    3
#define	ETH_10MFULL		        4
#define	ETH_10MHALF		        5
#define	ETH_1000M			    6
#define	ETH_NA				    10

//define error code
#define  ERROR_NO				0x00	
#define  ERROR_SYSTEM_GENERAL	0x01	
#define  ERROR_UNKNOWN			0x02
#define  ERROR_LOGIN_FAIL		0x11	
#define  ERROR_LOGIN_TIMEOUT	0x12	
#define  ERROR_WRONG_PWD		0x13	
#define  ERROR_COMM_LOCAL		0x14	
#define  ERROR_COMM_485		    0x15	
#define  ERROR_COMM_FIBER		0x16	
#define  ERROR_COMM_PARSE		0x17	
#define  ERROR_COMM				0x18	
#define  ERROR_WRONG_ARGS		0x19	
#define  ERROR_DATETIME_FORMAT  0x1A
#define  ERR_SFP_INCOMPATIBLE	0x1B
#define  ERROR_NEWID_EXIST		0x1C	
#define  ERROR_DSN_NOTMATCH	    0x1D	
#define  ERROR_NOTFOUND		    0x1E	
#define  ERROR_LIMITATION		0x1F
#define  ERROR_DATABASE		    0x20
#define  ERROR_I2C				0x21
#define  ERROR_HDMI				0x22
#define  ERROR_UP_INPROGRESS	0x23
#define  ERROR_UP_FILE			0x24
#define  ERROR_UP_ERROR			0x28


//define limitations
#define MAX_LEN					256
#define IPADDR_LEN				20 
#define MAX_ONU_NUM			    24
#define MAX_EPONU_NUM			32
#define MAX_NTU_NUM             2
#define MAX_MTU_NUM             2        // 201611
#define MAX_DISPLAYNAME_LEN	    16
#define MAX_NAME2_LEN			32
#define MAX_VLAN_NUM			5
#define MAX_ONUDSN_LEN		    16

#define MAX_CMDBUFF_SIZE		1024
#define MAX_485MSG_LEN			512
#define MAX_485MSG_FULLLEN	    528

#if 0 // 201708
#define TIMEOUT_485MSG			300000 //500ms可改为20ms来测试一下
#else
#define TIMEOUT_485MSG			20000 //改为20ms来测试一下
#endif

#define TIMEOUT_485MSGLONG		800000

#if 0 // 201708
#define TIMEOUT_485MSG_SND	    200000 //200ms
#else
#define TIMEOUT_485MSG_SND	    50000 //50ms
#endif

#define TIMEOUT_CMDMSG_LOOP	    100000 //100ms
#define TIMEOUT_CACHE_FE8       5000000

/* session */
#define SUCCESS                                0
#define ERROR                                  -1
#define FILEEXIST                              -2
#define FILENOTEXIST                           -3
#define IPEXIST                                -9
#define IPNOTEXIST                             -10
#define ISTIMEOUT                              -11
#define CURRENTDIR                             "./"
#define SESSIONID_LEN                          32
#define TIMEOUT                                3600   //seconds
#define SESSIONID_SECTION                      "SESSIONID=" 
#define CLIENTIP_SECTION                       "CLIENTIP="
#define CURRTIME_SECTION                       "CURRTIME=" 
#define CLIENTCNT_SCTION                       "CLIENTCNT=" 
#define SUFNAME                                	".sen"
#define FLAGFILENAME                           "FLAGFILE"

/* configure file */
#define STRING_PARAM_VALUE_LEN          100
#define INI_FILE_NAME_LEN                      100

#define FAN_STATE                       "FAN_STATE="
#define PASSWD_SECTION                         "PASSWD="
#define IPADDR_SECTION				"IPADDR="
#define NET_MASK						"NETMASK="
#define GATE_WAY						"GATEWAY="
#define DEF_USERNAME                           "admin"
#define LANG_KEYNAME			   "LANG"

/* define the semaphore name for cmd pipe */
#define SEM_PIPE_KEY		0x1100

/* define file location */
#define WEB_CONF_FILE		"/mnt/utn/etc/web.conf"
#define IP_CONFIG			"/mnt/etc/ifcfg0"
#define IO_CONF_FILE		"/mnt/etc/onuio.ini"
#define FSA_IP_CONFIG		"/mnt/etc/ifcfg-eth0"
#define CONF_DB				"/mnt/utn/utnconfig.db"	
#define DYNAMIC_DB			"/var/dynamic.db"	
#define SNMP_AGENT_CONFIG 	"/mnt/utn/etc/snmpagent.conf"
#define SNMPD_CONFIG		"/mnt/utn/etc/snmpd.conf"
#define CHG_SNMPD_CONFIG	"/mnt/utn/etc/chsnmpd.conf"
#define SSW_PARAM_CONFIG	"/mnt/utn/etc/ssw.conf"
#define SHELF_CONFIG       "/mnt/utn/etc/shelf.conf"

#define SCHED_PRIORITY_HIGH	23
#define SCHED_PRIORITY_MIDDLE	22
#define SCHED_PRIORITY_NORMAL	21
#define SCHED_PRIORITY_LOW		20

