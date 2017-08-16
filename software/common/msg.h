/* Message Type definition */
#define	MSG_ONU_CFG			0x01
#define	MSG_8995_CFG			0x02
#define	MSG_CARD_CFG			0x03
#define	MSG_NOTIFICATON		0x04
#define	MSG_DATA				0x05

/* Message Code definition */
/* ONU related configuration messages */
#define	C_ONU_GET				0x01
#define	C_ONU_SET				0x02
#define	C_ONU_SETADMIN		0x03
#define	C_ONU_SETBW			0x04
#define	C_ONU_RESET			0x05
#define	C_ONU_GETLIST			0x06
#define C_ONU_GETDATACH		0x07
#define	C_ONU_SETDATACH		0x08
#define C_ONU_GETPORTMIRROR	0x09
#define	C_ONU_SETPORTMIRROR	0x0A
#define C_ONU_GETQoS			0x0B
#define C_ONU_SETQoS			0x0C
#define C_ONU_GETVLAN			0x0D
#define C_ONU_SAVEVLAN			0x0E
#define C_ONU_DELVLAN			0x0F



/* Generic card configuration messages */
#define	C_CARD_GET				0x11
#define	C_CARD_SET				0x12
#define	C_CARD_SETADMIN		    0x13
#define	C_CARD_RESET			0x14
#define	C_CARD_QUERY_ALARM	    0x15
#define C_SHELF_GET     			0x16
#define C_SHELF_SET     			0x17

/* Service card configuration messages */
#define	C_FSA_SET				0x20
#define	C_FSA_GET				0x21
#define	C_ETHER_SET			    0x22
#define	C_FE8_GET				0x23
#define	C_FE8_SET				0x24
#define	C_SFP_GET				0x25
#define	C_ETHER_GET			    0x26
#define	GET_PORT_PERF			0x27
#define SET_PERF_RESET		    0x28
#define	C_FE8_PORT_GET    		0x29
#define C_FSA_NET_GET    		0x2a
#define C_FSA_NET_SET    		0x2b
#define C_FSA_LOWSPD_GET 		0x2c
#define C_FSA_LOWSPD_SET 		0x2d
#define C_FSA_LOWSPD_STAT 	    0x2e
#define C_FSA_LOWSPD_STATRESET 	0x2F

/*FSA600 card configuration messages*/
#define	C_FSA600_INFO    		0x30
#define	C_FSA600_STATE    		0x31  /*used to obtain the  number of inserted sfp and online ONU100*/
#define	C_FSA600_SET    		0x32
#define	C_FSA600_GETRMT   		0x33
#define	C_FSA600_SETRMT   		0x34
#define	C_FSA600_RESETRMT  	    0x35
//#define	C_FSA600_PERF			0x36
#define C_FSA600_RESET 			0x37
#define C_FSA600_SETDFT		    0x38
#define C_FSA600_SETDFTRMT	    0x39

#define GET_ALARMS			    0x41

#define C_FE8_BROADCASTSTORM_GET	0x42
#define C_FE8_BROADCASTSTORM_SET	0x43

/*encode card configuration messages*/
#define C_ENCODE_VIDEO_SET			0x4F
#define C_ENCODE_VIDEO_GET		0x50
#define C_ENCODE_NET_GET			0x51
#define C_ENCODE_NET_SET			0x52
#define C_ENCODE_RESTART			0x53
#define C_ENCODE_MODE_GET			0x54
#define C_ENCODE_MODE_SET			0x55
#define C_ENCODE_OSD_GET			0x56
#define C_ENCODE_OSD_SET			0x57
#define C_ENCODE_NTP_GET			0x58
#define C_ENCODE_NTP_SET			0x59
#define C_ENCODE_SYSTIME_GET		0x5A
#define C_ENCODE_SYSTIME_SET 		0x5B
#define C_ENCODE_PICTURE_GET		0x5D
#define C_ENCODE_PICTURE_SET		0x5E
#define C_ENCODE_PROTOCOL_GET		0x5C
#define C_ENCODE_PROTOCOL_SET		0x5F


/* fsa and switch  card all port statistics count. switch card vlan , multicast....*/
#define C_FSA_GETCOUNT		0x60
#define C_FSA_RESETCOUNT	0x61
#define C_SW_GETCOUNT		0x62
#define C_SW_RESETCOUNT	0x63
#define C_SW_GETVLAN		0x64
#define C_SW_SAVEVLAN		0x65
#define C_SW_DELVLAN		0x66
#define C_SW_GETPORTSTATE	0x67
#define C_SW_GETALLMC		0x68
#define C_SW_SAVEMCG		0x69
#define C_SW_DELMCG		0x6a
#define C_SW_GETPARAM		0x6b
#define C_SW_SETPARAM		0x6c
#define C_SW_SAVEL3INTF	0x6d
#define C_SW_DELL3INTF		0x6e

/*more ONU commands */
#define C_ONU_GETIO			0x71
#define C_ONU_SETIO			0x72
#define C_ONU_GETDSN_LIST	0x73
#define C_ONU_SET_ONUID	0x74

#define C_GE8_NET_GET				0X8A
#define C_GE8_NET_SET				0X8B
#define C_GE8_GETVLAN				0x8C
#define C_GE8_DELVLAN				0x8D
#define C_GE8_SAVEVLAN 				0x8E
#define C_GE8_PORTMODE				0x90
#define C_GE8_GETPORT				0x91
#define C_GE8_SETPORT				0x92
#define C_GE8_STATISTICS			0x93
#define C_GE8_STATISTICS_CLR 		0x94
#define C_GE8_GETIGMP 				0x95
#define C_GE8_SETIGMP				0x96

/* NTT command*/
#define C_NTT_GETSTATE				0xA1
#define C_NTT_INFO                  0xA2
#define C_NTT_SET                   0xA3

#if 1 //201611
/* MTT command*/
#define C_MTT_GETSTATE              0xA1 // 与ntt一样
#define C_MTT_IOCTRL                0xA4
#endif

#define  C_MS4_SWITCH		        0xA8
#define  C_MS4_DEBUG		        0xA9 /* 16 byte */

/* HDC command*/
#define C_HDC_GETCMD			    0xAE
#define C_HDC_SETCMD			    0xAF

/* EPON command */
#define	C_EP_GET_ONULIST		    0xB0
#define	C_EP_GET_ONUSTAT		    0xB1
#define	C_EP_SET_ONUNAME		    0xB2
#define	C_EP_GET_ONUUART		    0xB3
#define	C_EP_SET_ONUUART		    0xB4
#define	C_EP_GET_ONUBW		        0xB5
#define	C_EP_SET_ONUBW		        0xB6
#define	C_EP_GET_ONUIO		        0xB7
#define	C_EP_SET_ONUIO		        0xB8	
#define	C_EP_GET_ONUNETWORK	        0xB9
#define	C_EP_SET_ONUNETWORK	        0xBA
#define	C_EP_RESET_ONU		        0xBB
#define	C_EP_SET_ONU_UPPATH	        0xBC
#define	C_EP_SET_OLT_UPPATH	        0xBD
#define	C_EP_GET_OLTSTAT		    0xBE
#define	C_EP_DELONU_OFFLINE	        0xBF
#define	C_EP_GET_OLTVLAN		    0xC0
#define	C_EP_SET_OLTVLAN		    0xC1
#define	C_EP_SET_ONUIOCTRL		    0xC2        // 20150417 io控制输出

#if 1 // 201708
#define C_MTT_SEND_DATA             0xE0
#define C_MTT_GET_DATA              0xE1
#define C_MTT_SEND_IO_CTRL          0xE2
#endif

/* LCD display command */
#define C_LCD_NET		            0xF1
#define C_LCD_FAN		            0xF2
#define C_LCD_PWR		            0xF3

/*BACKPLANE command */
#define C_BACK_GET                  0xf4
#define C_BACK_SET                  0xf5


/* Web page refresh check request, pipe cmd only */
#define REFRESH_CHECK			    0xF1

/*define modify flag*/
#define	MD_CARD_ADMIN		        0x01
#define	MD_CARD_NAME		        0x02

#define	MD_ONU_ADMIN		        0x01
#define	MD_ONU_NAME		            0x02
#define	MD_ONU_STORM		        0x04
#define MD_ONU_STMTHRESHOLD         0x08

/*define modify flag for data channel*/
#define MD_DATACH_BRAUDRATE         0x01
#define MD_DATACH_PORTSEL	      	0x02//232 or 485
#define MD_DATACH_RECVBUFF	        0x04 //ONU_UART_CONF structure of  time
#define MD_DATACH_CONTROL	        0x08 //ONU_UART_CONF structure of  time

#define	MD_ETHERPORT_ADMIN			0x01
#define MD_ETHERPORT_MODE 			0x02
#define MD_ETHERPORT_WORKINGMODE 	0X04
#define MD_ETHERPORT_BWMALLOCSEND	0x08
#define	MD_ETHERPORT_BWMALLOCRECV	0x16
#define MD_ETHERPORT_BOUNDEDMAC		0x32

#define	MD_FSA600_PORTSTATE			0x01
#define	MD_FSA600_TRANSMITMODE		0x02
#define	MD_FSA600_UPSTREAM			0x04
#define	MD_FSA600_DOWNSTREAM		0x08


#define	MD_ONU100_TRANSMITMODE		0x01
#define	MD_ONU100_CFGWORKMODE1		0x02
#define	MD_ONU100_CFGWORKMODE2		0x04
#define MD_ONU100_NAME				0x08

#define	MD_FSA_STATISTICTYPE		0x01
#define	MD_FSA_IGMPSNOOPING		    0x02
#define	MD_FSA_STORM				0x04
#define MD_FSA_STMTHRESHOLD		    0x08

#define MD_SW_IGMPQRYTIME		    0x01
#define MD_SW_IGMP_TIMOUT		    0x02
#define MD_SW_IGMP_RPORTMAP		    0x04
#define MD_SW_IGMP_QPORTMAP		    0x08

#define MD_SW_VLAN_PBITMAP		    0x01
#define MD_SW_VLAN_UPBITMAP		    0x02
#define	MD_SW_VLAN_ID			    0x04

#define MD_SW_MC_PBITMAP		    0x01
#define MD_SW_MC_GRPIP		        0x02


/*Notification Definition*/
#define	MSG_NOTIFICAITON_BASE 	    0x80
#define	N_CARD						MSG_NOTIFICAITON_BASE + 0x01
#define	N_CARD_COMM				    MSG_NOTIFICAITON_BASE + 0x02
#define	N_CARD_RESTART			    MSG_NOTIFICAITON_BASE + 0x03

#define	N_FSA_SPF					MSG_NOTIFICAITON_BASE + 0x11
#define	N_FSA_GE					MSG_NOTIFICAITON_BASE + 0x12
#define	N_ONET_NODE_ONLINE		    MSG_NOTIFICAITON_BASE + 0x13
#define	N_ONET_NODE_OFFLINE		    MSG_NOTIFICAITON_BASE + 0x14
#define	N_ONET_RING_STANDBY		    MSG_NOTIFICAITON_BASE + 0x15
#define	N_ONET_RING				    MSG_NOTIFICAITON_BASE + 0x16
#define	N_ONET_RING_BROKEN		    MSG_NOTIFICAITON_BASE + 0x17
#define	N_ONET_LINK_BROKEN		    MSG_NOTIFICAITON_BASE + 0x18

#define	N_ONU_SPF					MSG_NOTIFICAITON_BASE + 0x30
#define	N_ONU_FE					MSG_NOTIFICAITON_BASE + 0x31

#define MAX_DISPLAYNAME_LEN 16

/*******************************************
*	           Generic   data definition 
*	
********************************************/
/* shelf data */
typedef struct SHELF_INFO {
	char          	temperature;
	unsigned char power;	  /*bit0~1: pwr1~2*/
	unsigned char fan_state[3]; 
	unsigned char  fan_config;  /* bit0~2: fan 1~3, bit3: auto; 0-off, 1-on*/
	unsigned char t_threshod_low[3];
	unsigned char t_threshod_high[3];
}__attribute__ ((__packed__)) SHELF_INFO;

/* basic card information */
typedef struct CARD_INFO {
	unsigned char 	card_no;
	unsigned char 	card_type;	
	unsigned char 	admin_state;	/* enabled(1),disabled(2)*/
	char 			card_name[MAX_DISPLAYNAME_LEN];	
	unsigned char		version_HW[2];
	unsigned char		version_FW[2];
	unsigned char		version_SW[2];
	char				version_OS[MAX_DISPLAYNAME_LEN];
}__attribute__ ((__packed__)) CARD_INFO;

/* GE port state on card */
typedef struct GE_PORT_STATE {
	unsigned char 	portIndex;
	unsigned char 	portWorkingMode;
	unsigned char 	portMode;           /*learning-mode(1) bounded-mac(2)*/
	unsigned char  	admin_state;  	/* enabled(1),disabled(2)*/
	unsigned short  	bw_alloc_send;		/*band width allocation */
	unsigned short  	bw_alloc_recv;		/*band width allocation */
	unsigned char 	port_state;
	unsigned char 	bounded_mac[6];
}__attribute__ ((__packed__)) GE_PORT_STATE;

/* FE port state */
typedef struct FE_PORT_STATE {
	unsigned char 	portIndex;
	unsigned char 	portWorkingMode;
	unsigned char 	portMode;		 /*learning-mode(1) bounded-mac(2)*/
	unsigned char  	admin_state;	/* enabled(1),disabled(2)*/
	unsigned short  	bw_alloc_send;		/*band width allocation */
	unsigned short  	bw_alloc_recv;		/*band width allocation */
	unsigned char 	port_state;
	unsigned char 	bounded_mac[6];
}__attribute__ ((__packed__)) FE_PORT_STATE;

/* port performance data */
typedef struct PORT_PERF {
	unsigned long    perfRxByteHi;
	unsigned long    perfRxByteLo;
	unsigned long    perfTxByteHi;
	unsigned long    perfTxByteLo;
}__attribute__ ((__packed__)) PORT_PERF;


/* port statistic count data */
/*counttype value 1:counter1 is transimit packet count, counter2 is receive packet count.
		     2:counter1 is collision count ,  counter2 is transimit packet count.
		     3:counter1 is Drop packet count (MAC) ,  Receive packet count.
		     4:counter1 is CRC error packet count ,  Receive packet count.
*/
typedef struct PORT_COUNT{
	unsigned char    countType;
	unsigned long    portCount1[26];
	unsigned long    portCount2[26];
}__attribute__ ((__packed__)) PORT_COUNT;

/*switch control card configuration data definition*/
typedef struct SW_PORT_COUNT{
	unsigned long    recvbytecount[26];
	unsigned long    recvpktcount[26];
	unsigned long	   recvMCpktcount[26];
	unsigned long	   recvBCpktcount[26];
	unsigned long    tranbytecount[26];
	unsigned long    tranpktcount[26];
	unsigned long	   tranMCpktcount[26];
	unsigned long	   tranBCpktcount[26];
}__attribute__ ((__packed__)) SW_PORT_COUNT;

typedef struct SW_VLAN{
	unsigned char setFlag;
	unsigned int ut_port_bitmap:29;
	unsigned int port_bitmap:29;
	unsigned int vlan_id:12;
}__attribute__ ((__packed__)) SW_VLAN;

typedef struct SW_PORT_STATE{
	struct FE_PORT_STATE		port_state[26];
}__attribute__((__packed__))SW_PORT_STATE;

typedef struct SW_MC_STATE{
	unsigned char setFlag;
	unsigned int grpip;
	unsigned int bitmap;
	unsigned char flag;/*1:statistic group, 2:dynamic group  */
}__attribute__((__packed__))SW_MC_STATE;


typedef struct SW_CONFIG{
	unsigned char setFlag;
	unsigned int qry_time;/*igmp snooping query interval*/
	unsigned int igmp_timeout; 
	unsigned int igmp_qportmap; /* query port map */
	unsigned int igmp_rportmap; /* router port map */

}__attribute__((__packed__))SW_CONFIG;

typedef struct SW_L3INTF{
	unsigned char setFlag;
	unsigned char id;
	unsigned char vlan; 
	unsigned char reserved; 
	char 		ip[16]; 
	char 		mask[16];
}__attribute__((__packed__))SW_L3INTF;

/*******************************************
*	FSA card related configuration data definition 
*	
********************************************/
/* SFP port info on card */
typedef struct SFP_INFO {
	unsigned char   sfpIndex;	/*from snmp,currently considered 1 is for near,2 is for far.*/
	unsigned char   sfpCompliance;   /* 0: not compliant with SFF-8472, 0x01: compliant with SFF-8472, 0xFF: no data */
	char			  sfpVendor[MAX_DISPLAYNAME_LEN];
	char			  sfpPartNumber[MAX_DISPLAYNAME_LEN];
	unsigned char   sfpConnector;  		/* 0x01: SC, 0x07: LC, 0x22: RJ45, others: unsupported */
	unsigned char   sfpTransCode;		/* bit0: SingleMode, bit2: MultiMode, bit3: MultiMode, others: unsupported*/
	unsigned char   sfpSmLength;		/* unit: km, valid only when sfpTransCode is SingleMode*/
	unsigned char   sfpMmLength;		/* unit: 10m, only when sfpTransCode is MultiMode*/
	unsigned char   sfpCopperLength;  	/* unit: m*/
	unsigned char   sfpBrSpeed; 		/* unit: 100Mbps */
	unsigned short  sfpWavelength; 	/* unit: nm */
	char			  sfpTemperature;	/* -127c~127c*/
	unsigned short  sfpTranPower; 		/* unit 0.1uW*/
	unsigned short  sfpRecvPower;		/* unit 0.1uW*/
}__attribute__ ((__packed__)) SFP_INFO;

/* FSA card state information */
typedef struct FSA_CARD_STATE {
	unsigned char 			spf_n_state;	
	unsigned char 			spf_f_state;	
	struct GE_PORT_STATE 	geport_state[2];
	unsigned char 			network_state;
	unsigned char 			statistic_type;
	unsigned char				igmp_snooping;/*1:enable,2:disable*/ 
											/* bit 7: 1-use bit6, 5, 4 as port clock delay config, 0-compatible with old version*/
											/* bit 6, 5, 4: 1- set clock delay for port group3, 2, 1; 0-no clock delay */
	unsigned char 			broadcaststorm_ctrl;/*1:enable, 2: disable*/
	unsigned char 			storm_threshold;
	
}__attribute__ ((__packed__)) FSA_CARD_STATE;

typedef struct FSA_NETWORK {
	char 			ip[16];	
	char 			mask[16];	
	char 			gateway[16];	
}__attribute__ ((__packed__)) FSA_NETWORK;

typedef struct FSA_LOWSPD_CONFIG {
	unsigned short 	rs_port;	
	unsigned short 	rs_port_step;	
	char			 	as_ip[16];	
	unsigned short 	as_port;	
	unsigned long 		alarm_mask[6];	
	unsigned short 	dev_id;	
	unsigned short 	dev_id_step;	
	unsigned char 	alarm_mode;	
	unsigned char 	alarm_format;	
	char			 	as_ip2[16];	
	unsigned short 	as_port2;	
	unsigned char 	alarm_format2;	
	unsigned char 	reserved;	
}__attribute__ ((__packed__)) FSA_LOWSPD_CONFIG;

typedef struct FSA_LOWSPD_STATE {
	unsigned short 	rs_connection[5];	
	unsigned short 	rs_port[5];	
	unsigned long	 	rs_snd[5];	
	unsigned long	 	rs_rcv[5];	
	unsigned short 	alarm_dev_id[6];	
	unsigned short 	alarm_count[6];	
}__attribute__ ((__packed__)) FSA_LOWSPD_STATE;

typedef struct CARD_INFO_FE8 {
	struct FE_PORT_STATE		port_state[8];
}__attribute__ ((__packed__)) CARD_INFO_FE8;

typedef struct ONU_LIST_ENTRY {
	unsigned char		node_id;
	unsigned char		node_sn;
	unsigned char		node_state;	/* 0: not used; 1: Online and Enabled; 2: Online but Disabled; 3: Offline */
	unsigned char		network_state;	/* 0: not valide; 1: link, onu is at FSA near end-fiber1, 2: link, onu is at FSA far end -fiber2, 3: ring, */
	char				onu_name[MAX_DISPLAYNAME_LEN];	
}__attribute__ ((__packed__)) ONU_LIST_ENTRY;

/*******************************************
*	ONU node configuration data definition 
*	
********************************************/
/* ONU configuration information */
typedef struct ONU_INFO {
	unsigned char  				node_id;
	unsigned char  				node_sn;
	unsigned char  				admin_state;	/* enabled(1),disabled(2)*/
	char 						onu_name[MAX_DISPLAYNAME_LEN];	
	unsigned char 				spf_n_state;	
	unsigned char 				spf_f_state;	
	unsigned char 			broadcaststorm_ctrl;/*1:enable, 2: disable*/
	unsigned short 			storm_threshold;	/*0~2047*/
}__attribute__ ((__packed__)) ONU_INFO;

/* ONU UART configuration */
typedef struct ONU_UART_CONF {
	unsigned char	  			setFlag;
	unsigned char 				port;		/*bit(1,0): "00" port 1, "01", port 2, */
	unsigned char  				baudrate;		/* bit(2...0):"111"~"000" baudrate:115200,57600,38400,19200,9600,4800,2400,1200*/
	unsigned char  				function;			/* Port 1: 0x00:Debug port,0x01:Data comm, Port2: 0x00:232,0x01:485 */
	unsigned char  				time;				/* RX,0ms~255ms*/
//	unsigned char  				control;				/* bit(1,0): databits, 00~11 - 5~8; bit(2): stopbit, 0:1, 1:2; bit(5,4,3): parity, xx0-no, 001:Odd, 011:Even, 101:Mark, 111:space */
}__attribute__ ((__packed__)) ONU_UART_CONF;

/* ONU UART configuration */
typedef struct ONU_IO {
	unsigned char	  				in;
	unsigned char 				out_select;	/*set bit mask: bit0 - O1, bit1 - O2, bit2 - O3 ... */	
	unsigned char  				out_value;		
}__attribute__ ((__packed__)) ONU_IO;

/* ONU PORT MIRROR configuration */
typedef struct ONU_PORTMIRROR_CONF {
	unsigned char	  				setFlag;
	unsigned char					port1[3];		/*port1[0]:Mirror Port1, port1[1]:Rx port1 to be mirrored, port1[2]:Tx port1 to be mirrored    0x00:unchecked,0x01:checked*/
	unsigned char					port2[3];
	unsigned char					port3[3];
	unsigned char					port4[3];
	unsigned char					port5[3];
	unsigned char					badframes;		/*Including bad frames*/
	unsigned char					sourceanddestination;	/*Match both source and destination ports*/
}__attribute__ ((__packed__)) ONU_PORTMIRROR_CONF;

/* ONU QoS configuration */
typedef struct ONU_QoS_CONF {
	unsigned char	  				setFlag;
	unsigned char					priorityEnable[5];		/*priorityEnable[0] for Port1 ...... priorityEnable[4] for Port5    0x00:disable,0x01:enable*/
	unsigned char					prioritySchemeSelect;		/*0x00:always deliver high priority packets first; 0x01:deliver high/low packets at ratio 10/1; 0x10:deliver high/low packets at ratio 5/1; 0x11:deliver high/low packets at ratio 2/1*/
}__attribute__ ((__packed__)) ONU_QoS_CONF;


/* ONU VLAN configuration */
typedef struct ONU_VLAN_CONF{
	unsigned char setFlag;
	unsigned char valid;	/*0x01:the entry is valid;0x00:the entry is invalid*/
	unsigned char membership;
	unsigned char id;
	unsigned short vlan_id;
}__attribute__ ((__packed__)) ONU_VLAN_CONF;


/*******************************************
*	SET definition 
*	
********************************************/
typedef struct CARD_SET {
	unsigned char	  setFlag;	
	unsigned char 	admin_state;	
	char 		card_name[MAX_DISPLAYNAME_LEN];
}__attribute__ ((__packed__)) CARD_SET;


typedef struct ETHER_PORT_SET {
	unsigned char	  	setFlag;
	unsigned char  	admin_state;
	unsigned char 	portMode;
	unsigned char 	portWorkingMode;
	unsigned short  	bw_alloc_send;
	unsigned short  	bw_alloc_recv;
	unsigned char 	bounded_mac[6];
}__attribute__ ((__packed__)) ETHER_PORT_SET;

typedef struct CARD_SET_FSA {
	unsigned char 	setFlag;
	unsigned char 	admin_state;
	char 		card_name[MAX_DISPLAYNAME_LEN];
	ETHER_PORT_SET  	port[2];
	unsigned char 		statistic_type;
	unsigned char				igmp_snooping;/*1:enable,2:disable*/
											/* bit 7: 1-use bit6, 5, 4 as port clock delay config, 0-compatible with old version*/
											/* bit 6, 5, 4: 1- set clock delay for port group3, 2, 1; 0-no clock delay */
	unsigned char 			broadcaststorm_ctrl;/*1:enable, 2: disable*/
	unsigned char 			storm_threshold;
	}__attribute__ ((__packed__)) CARD_SET_FSA;
	


typedef struct ONU_SET {
	unsigned char	  setFlag;
	unsigned char 	admin_state;	
	char 		onu_name[MAX_DISPLAYNAME_LEN];
	unsigned char 			broadcaststorm_ctrl;/*1:enable, 2: disable*/
	unsigned short 			storm_threshold;	/*0~2047*/
}__attribute__ ((__packed__)) ONU_SET;

typedef struct ONU_SETID {
	unsigned char		new_onuid;
	char				onu_dsn[MAX_ONUDSN_LEN];
}__attribute__ ((__packed__)) ONU_SETID;


/*******************************************************
*  Message  for FSA600 Card
*
**********************************************************/

typedef struct FSA600_PORT_485 {
 	unsigned char   PortState;  /* 1:lock 0:unlock*/
	unsigned char   TransmitMode; /* 1:cut-througth 0:store-forward*/
	unsigned char   Fxlink; /* 1:up 0:down*/
	unsigned char   SfpState; /* 1:inserted 0:removed*/
	unsigned char   UpStreamFlag; /* 1:512K 0:32K*/
	unsigned char   UpStream;
	unsigned char   DownStreamFlag; /*, 1:512K 0:32K*/
	unsigned char   DownStream;
 }__attribute__ ((__packed__)) FSA600_PORT_485;

typedef struct FSA600_STATE_485 {
	unsigned char   Fxlink; /*bits 0:5 for onu100 1~6, 1:up 0:down*/
	unsigned char   SfpState; /*bits 0:5 for onu100 1~6, 1:inserted 0:removed*/
}__attribute__ ((__packed__)) FSA600_STATE_485;

typedef struct FSA600_SET_485 {
	unsigned char setFlag;
	unsigned char   PortState;  /*  1:lock 0:unlock*/
	unsigned char   TransmitMode; /* 1:cut-througth 0:store-forward*/
	unsigned char   UpStreamFlag; /*  1:512K 0:32K*/
	unsigned char   UpStream;
	unsigned char   DownStreamFlag; /* 1:512K 0:32K*/
	unsigned char   DownStream;
}__attribute__ ((__packed__)) FSA600_SET_485;

typedef struct FSA600_GETRMT {
	unsigned char   OnuId; 	/*onu100(1)onu100(2)onu100(3)onu100(4)onu100(5)onu100(6)*/
	unsigned char   TransmitMode; /*1:cut-througth 2:store-forward*/
	unsigned char   CurWorkMode1; /* Auto(1), 100M-full(2), 100M-half(3), 10M-full(4), 10M-half(5) */
	unsigned char   CfgWorkMode1;
	unsigned char   Txlink1; /* 1:up 2:down*/
	unsigned char   CurWorkMode2; /* Auto(1), 100M-full(2), 100M-half(3), 10M-full(4), 10M-half(5) */
	unsigned char   CfgWorkMode2;
	unsigned char   Txlink2; /* 1:up 2:down*/
	char 		onu_name[MAX_DISPLAYNAME_LEN];

}__attribute__ ((__packed__)) FSA600_GETRMT;

typedef struct FSA600_SETRMT {
	unsigned char setFlag;
	unsigned char   OnuId; 	/*onu100(1)onu100(2)onu100(3)onu100(4)onu100(5)onu100(6)*/
	unsigned char   TransmitMode; /*1:cut-througth 2:store-forward*/
	unsigned char   CfgWorkMode1;
	unsigned char   CfgWorkMode2;
	char 		onu_name[MAX_DISPLAYNAME_LEN];
	unsigned char   padding[3];
}__attribute__ ((__packed__)) FSA600_SETRMT;

typedef struct FSA600_ALARM_INFO {
	unsigned char  		fxstate;   /*bits 0:5 for onu100 1~6   1:up,link, 0:down ,unlink.*/
	unsigned char  		sfpstate;
	unsigned char 		rmt_tx1state;		
	unsigned char 		rmt_tx2state;	
	unsigned char 		rmtPwrDown;
}__attribute__ ((__packed__)) FSA600_ALARM_INFO;


typedef struct RERESH_CHECK_REQ {
	unsigned long			sessionID;
	unsigned char  		type;   /*1:tree, 2:frame panel, 3:topo*/
}__attribute__ ((__packed__)) RERESH_CHECK_REQ;

/*******************************************
*	Alarm and nodification definition 
*	
********************************************/
/* card alarm information */   
typedef struct ALARM_INFO {
        unsigned char                           alarm_type;
        unsigned char                           port_no;
}__attribute__ ((__packed__)) ALARM_INFO;

/* FSA and ONU alarm information */
typedef struct ONET_ALARM_INFO {
	unsigned char  				alarm_type;
	unsigned char  				node_id;
	unsigned char 				port_no;	
	unsigned char 				reason;	
}__attribute__ ((__packed__)) ONET_ALARM_INFO;

/* fe8 card alarm information */   
typedef struct FE8_ALARM_INFO {
        unsigned char                           port_up;/*bits 0:7 for port1~8  .*/
        unsigned char                           port_down;
}__attribute__ ((__packed__))FE8_ALARM_INFO;

/* ntt card alarm information */   
typedef struct NTT_ALARM_INFO {
        unsigned char               port_up;/*bits 0:7 for port1~8  .*/
        unsigned char               port_down;
	    unsigned char				fiber_on;
	    unsigned char               fiber_off;
}__attribute__ ((__packed__))NTT_ALARM_INFO;

#if 1 // 201611
typedef struct MTT_ALARM_INFO
{
    //unsigned char               cardflag;
    unsigned char               rmt_portup; 
    unsigned char               rmt_portdown;   
    unsigned char               loc_portup;
    unsigned char               loc_portdown;
}__attribute__ ((__packed__))MTT_ALARM_INFO;
#endif

#if 1 // 201708
typedef struct MTT411_ALARM_INFO
{
    //unsigned char               cardflag;
    unsigned char               rmt_portup; 
    unsigned char               rmt_portdown;   
    unsigned char               loc_portup;
    unsigned char               loc_portdown;
}__attribute__ ((__packed__))MTT411_ALARM_INFO;


/*
前1电源	前1光纤	前1独立网口	后1独立网口
前1共享网口1	前1共享网口2	前1共享网口3	前1共享网口4
前1共享网口5	前1共享网口6	前1共享网口7	前1共享网口8

*/
typedef struct MTT441_ALARM_INFO
{
    unsigned char mtu1_front_power:2;        // 电源
    unsigned char mtu1_front_fiber:2;        // 光口
    unsigned char mtu1_front_net:2;          // 前端独立网口
    unsigned char mtu1_back_net:2;           // 后端独立网口
    
    unsigned char mtu1_front_net1:2;
    unsigned char mtu1_front_net2:2;
    unsigned char mtu1_front_net3:2;
    unsigned char mtu1_front_net4:2;
    
    unsigned char mtu1_front_net5:2;
    unsigned char mtu1_front_net6:2;
    unsigned char mtu1_front_net7:2;
    unsigned char mtu1_front_net8:2;
    
    unsigned char mtu2_front_power:2;
    unsigned char mtu2_front_fiber:2;
    unsigned char mtu2_front_net:2;          
    unsigned char mtu2_back_net:2;
    
    unsigned char mtu2_front_net1:2;
    unsigned char mtu2_front_net2:2;
    unsigned char mtu2_front_net3:2;
    unsigned char mtu2_front_net4:2;
    
    unsigned char mtu2_front_net5:2;
    unsigned char mtu2_front_net6:2;
    unsigned char mtu2_front_net7:2;
    unsigned char mtu2_front_net8:2;
}__attribute__((__packed__)) MTT441_ALARM_INFO;


#endif

typedef struct EPON_ALARM_INFO {
	unsigned char  				alarm_type;
	unsigned char 				pon;	
	unsigned char  				node_id;
	unsigned char 				port_no;	
}__attribute__ ((__packed__)) EPON_ALARM_INFO;


/* get alarms command response */
typedef struct GET_ALARMS_RSP {
    unsigned char	alarmNum; 		/* alarm count in the response */
	unsigned char rsp_type;		/*0: general card alarm, 1: fsa alarm ; 2:fsa 600 alarm ;
	                            3:fe8 alarm;4:ntt alarm; 5:EPON alarm, 6:mtt alarm*/
	union {
    	ALARM_INFO	alarms[1];
    	ONET_ALARM_INFO fsa_alarms[1];
    	EPON_ALARM_INFO epon_alarms[1];
    	FSA600_ALARM_INFO fsa600_alarms;
    	FE8_ALARM_INFO fe8_alarms;
    	NTT_ALARM_INFO ntt_alarms;
#if 1 // 201611
        MTT_ALARM_INFO mtt_alarms;
#endif

#if 1 // 201708
        MTT411_ALARM_INFO mtt_alarm_411;
        MTT441_ALARM_INFO mtt_alarm_441;
#endif
	} data;
}__attribute__ ((__packed__)) GET_ALARMS_RSP;


/*******************************************
*	           message header  definition 
*	
********************************************/
/**************485 communication header *****************/
typedef struct MSG_HEAD_485 {
	unsigned char  				flag;
	unsigned char  				code;
	unsigned char 				rsp;	
	unsigned char 				dst;	
	unsigned char 				src;	
	unsigned char 				onu_nodeid;	/*for encode card ,it's means channel no*/
	unsigned short 				length;	
}__attribute__ ((__packed__)) MSG_HEAD_485;

/**************pipe communication header *****************/
typedef struct MSG_HEAD_CMD {
	unsigned char  				msg_code;
	unsigned char 				card_no;	
	unsigned char 				node_id;	
	unsigned short 				length;	
}__attribute__ ((__packed__)) MSG_HEAD_CMD;

typedef struct MSG_HEAD_CMD_RSP {
	unsigned char  				rsp_code;
	unsigned char  				msg_code;
	unsigned char 				card_no;	
	unsigned char 				node_id;	
	unsigned short 				length;	
}__attribute__ ((__packed__)) MSG_HEAD_CMD_RSP;


/*******************************************************
*  Message payload for FE8 Card
*
**********************************************************/
//	C_CARD_GET		
//req len: 0
//data: CARD_SET_485
//rsp len: sizeof(CARD_INFO)
//data: CARD_INFO

//	C_FE8_GET		
//req len: 0
//rsp len: sizeof(CARD_INFO_FE8)
//data: CARD_INFO_FE8

//	C_FE8_SET		
//req len: 40
//data: CARD_SET_485FE
//rsp len: 0
typedef struct CARD_SET_485FE {
	unsigned char 		Index;/*0:for card.1~8:for eight ports.*/
	unsigned char 		admin_state;		
	ETHER_PORT_SET  	port;
}__attribute__ ((__packed__)) CARD_SET_485FE;


//	C_FE8_BROADCASTSTORM_SET and C_FE8_BROADCASTSTORM_GET
//req len: 4 and 0
//data: BROADCASTSTORM_CTRL_FE
//rsp len: 0 and 4
typedef struct BROADCASTSTORM_CTRL_FE {
	unsigned char	  	setFlag;
	unsigned char 		broadcaststorm_ctrl;/*1:enable, 2: disable*/
	unsigned short 		storm_threshold;	/*0~2047*/
}__attribute__ ((__packed__)) BROADCASTSTORM_CTRL_FE;


//C_CARD_RESET	
//req len: 0
//rsp len: 0

//GET_PORT_PERF		
//req len: 8
//data: FE_PORTINDEX
//rsp len: 32
//data: PORT_PERF

typedef struct FE_PORTINDEX {
	unsigned char 		portIndex;	
	unsigned char	  		padding[7];	
}__attribute__ ((__packed__)) FE_PORTINDEX;

//SET_PERF_RESET	
//req len: 8
//data: FE_PORTINDEX
//rsp len: 0

//	GET_ALARMS		
//req len: 0
//rsp len: sizeof(GET_ALARMS_RSP)
//data: GET_ALARMS_RSP





/*******************************************************
*  Message  for Encode Card
*
**********************************************************/
typedef struct ENCODE_VIDEO_STATE{
	unsigned char channelno;
	char vidBitType[4];
	char vidBitRate[9];
	char resolving[8];
	char frameRate[3];
	char gop[4];
}__attribute__((__packed__)) ENCODE_VIDEO_STATE;

typedef struct ENCODE_NET_STATE{
	unsigned char channelno;
	unsigned char ipaddr[17];
	unsigned char mask[17];
	unsigned char gateway[17];
}__attribute__((__packed__)) ENCODE_NET_STATE;


typedef struct ENCODE_MODE_STATE{
	unsigned char channelno;
	//unsigned char compresstype;
	unsigned char streamtype;
}__attribute__((__packed__))ENCODE_MODE_STATE;


typedef struct ENCODE_PROTOCOL_STATE{
	unsigned char channelno;
	char protocol[6];
	char ip[16];
 	char port[8];
}__attribute__((__packed__))ENCODE_PROTOCOL_STATE;



typedef struct ENCODE_OSD_STATE{
	unsigned char channelno;
	char location[4];
	unsigned char show_time;
	char channel_name[MAX_DISPLAYNAME_LEN];
	char color[4];
}__attribute__((__packed__))ENCODE_OSD_STATE;


typedef struct ENCODE_PICTURE_STATE{
	unsigned char channelno;
	unsigned char bright[4];
	unsigned char contrast[4];
	unsigned char chroma[4];
	unsigned char saturation[4];
}__attribute__((__packed__))ENCODE_PICTURE_STATE;


typedef struct ENCODE_NTP{
	unsigned char server_addr[17];
	//unsigned short interval;
	char interval[8];
}__attribute__((__packed__))ENCODE_NTP;

typedef struct ENCODE_SYSTIME{
	unsigned char time[20];
}__attribute__((__packed__))ENCODE_SYSTIME;

typedef struct ENCODE_CTRL{
	unsigned char channelno;
	unsigned char flag;
	unsigned char ipaddr[17];
	unsigned short portno;
}__attribute__((__packed__))ENCODE_CTRL;

/* ONU IO Report */
typedef struct ONU_IO_REPORT {
	unsigned char  				node_id;
	unsigned char 				io_state;	/*bit 0~7: io input 1~8 */
}__attribute__ ((__packed__)) ONU_IO_REPORT;

//add by yp
//GE8_STATISTICS
typedef struct GE8_STATISTICS {
	unsigned char 	portIndex;
	unsigned short Rx_Packets;
	unsigned short Tx_Packets;
	unsigned long Rx_Packets_B;
	unsigned long Tx_Packets_B;
	unsigned short Rx_Error;
	unsigned short Tx_Error;
}__attribute__ ((__packed__)) GE8_STATISTICS;

typedef struct GE8_IGMP {
	unsigned char 	State;
	unsigned char   Vid[20];
	unsigned char	Snoop_state[20];
	unsigned char	Query_state[20];
	unsigned char 	Route_port;
	unsigned char	Ipmc_flood;
}__attribute__ ((__packed__)) GE8_IGMP;

typedef struct GE8_NET_STATE{
	unsigned char channelno;
	unsigned char ipaddr[17];
	unsigned char mask[17];
	unsigned char gateway[17];
	unsigned char mac[6];
	unsigned char enable;
}__attribute__((__packed__)) GE8_NET_STATE;

typedef struct LCD_NET{
	unsigned char ipaddr[16];
	unsigned char mask[16];
	unsigned char gateway[16];
}__attribute__((__packed__)) LCD_NET;

typedef struct LCD_FAN{
	unsigned char temperature[8]; /* temperature string, only 5 characters used,  e.g. "26.00" */
	unsigned char fan_states[4];  /* 0: stop, 1: low, 2: middle, 3: fast, 4:failed */
}__attribute__((__packed__)) LCD_FAN;

typedef struct LCD_PWR{
	unsigned char pwr1; /* 0:no power, 1: OK*/
	unsigned char pwr2;
}__attribute__((__packed__)) LCD_PWR;


typedef  struct  FAN_CTL{
	unsigned char  fan_state;  /* bit0~2: fan 1~3, bit3: auto; 0-off, 1-on*/
}__attribute__((__packed__)) FAN_CTL;


typedef  struct  BACK_INFO{
	char          temperature;
	unsigned char power;	  /*bit0~1: pwr1~2*/
	unsigned char fan_state; /* bit0~2: fan 1~3, bit3: auto; 0-off, 1-on*/
	unsigned int  fan_speed[3];
}__attribute__((__packed__)) BACK_INFO;

typedef  struct  NTT_STATE{
	unsigned char		link;  /*bit 0~1: 0-down, 1-up */
	unsigned char 	ntu_state;	   /*bit 0~7: 0-down, 1-up */
}__attribute__((__packed__)) NTT_STATE;

#if 1   // mtt 201611
typedef struct MTT_STATE
{
    unsigned char FD1State;
    unsigned char FD2State;
    unsigned char Loc_EthState;
    unsigned char Rmt1_card_type;
    unsigned char Rmt1_ethstate;
    unsigned char Rmt1_otherState;
    unsigned char Rmt1_temp;
    unsigned char Rmt2_card_type;
    unsigned char Rmt2_ethstate;
    unsigned char Rmt2_otherState;
    unsigned char Rmt2_temp;    
}__attribute__((__packed__)) MTT_STATE;
#endif

#if 1 // 201708 form MTT_GD_411
typedef struct MTT_411_STATE
{
    // mtt
    unsigned char mtt_optic1:2;
    unsigned char mtt_optic2:2;
    unsigned char mtt_front_net1:2;
    unsigned char mtt_front_net2:2;
    
    unsigned char mtt_back_net1:2;
    unsigned char mtt_back_net2:2;
    unsigned char mtt_back_net3:2;
    unsigned char mtt_back_net4:2;
    
    unsigned char mtt_rsv1;
    unsigned char mtt_rsv2;
    
    // mtu1
    unsigned char mtu1_optic:2;
    unsigned char mtu1_rsv:2;
    unsigned char mtu1_net1:2;
    unsigned char mtu1_net2:2;
    
    unsigned char mtu1_net_share1:2;
    unsigned char mtu1_net_share2:2;
    unsigned char mtu1_net_share3:2;
    unsigned char mtu1_net_share4:2;
    
    unsigned char mtu1_io_in:2;
    unsigned char mtu1_io_out:2;
    unsigned char mtu1_data_in:2;
    unsigned char mtu1_data_out:2;
    
    unsigned char mtu1_rsv;
    
    // rpc1
    unsigned char rpc1_rsv1:2;
    unsigned char rpc1_rsv2:2;
    unsigned char rpc1_out_ac1:2;
    unsigned char rpc1_out_ac2:2;
    
    unsigned char rpc1_out_dc1:2;
    unsigned char rpc1_out_dc2:2;
    unsigned char rpc1_out_dc3:2;
    unsigned char rpc1_out_dc4:2;
    
    unsigned char rpc1_out_dc5:2;
    unsigned char rpc1_out_dc6:2;
    unsigned char rpc1_out_dc7:2;
    unsigned char rpc1_out_dc8:2;
    
    unsigned char rpc1_io_in:2;
    unsigned char rpc1_io_out:2;
    unsigned char rpc1_rsv3:2;
    unsigned char rpc1_rsv4:2;
    
    // mtu2
    unsigned char mtu2_optic:2;
    unsigned char mtu2_rsv:2;
    unsigned char mtu2_net1:2;
    unsigned char mtu2_net2:2;
    
    unsigned char mtu2_net_share1:2;
    unsigned char mtu2_net_share2:2;
    unsigned char mtu2_net_share3:2;
    unsigned char mtu2_net_share4:2;
    
    unsigned char mtu2_io_in:2;
    unsigned char mtu2_io_out:2;
    unsigned char mtu2_data_in:2;
    unsigned char mtu2_data_out:2;
    
    unsigned char mtu2_rsv;
    
    // rpc2
    unsigned char rpc2_rsv1:2;
    unsigned char rpc2_rsv2:2;
    unsigned char rpc2_out_ac1:2;
    unsigned char rpc2_out_ac2:2;
    
    unsigned char rpc2_out_dc1:2;
    unsigned char rpc2_out_dc2:2;
    unsigned char rpc2_out_dc3:2;
    unsigned char rpc2_out_dc4:2;
    
    unsigned char rpc2_out_dc5:2;
    unsigned char rpc2_out_dc6:2;
    unsigned char rpc2_out_dc7:2;
    unsigned char rpc2_out_dc8:2;
    
    unsigned char rpc2_io_in:2;
    unsigned char rpc2_io_out:2;
    unsigned char rpc2_rsv3:2;
    unsigned char rpc2_rsv4:2;
}__attribute__((__packed__)) MTT_411_STATE;

/*
MTU1:
NC	电源2	NC	光路2	NC	前端独立网口	后端独立网口1	后端独立网口2
网口1	网口2	网口3	网口4	网口5	网口6	网口7	网口8
前端开关量状态
前端节点温度-整数部分
前端节点温度-小数部分

MTU2:
NC  电源2   NC  光路2   NC  前端独立网口    后端独立网口1   后端独立网口2
网口1   网口2   网口3   网口4   网口5   网口6   网口7   网口8
前端开关量状态
前端节点温度-整数部分
前端节点温度-小数部分

*/
typedef struct MTT_441_STATE
{
    unsigned char mtu1_resv1:1;
    unsigned char mtu1_power:1;
    unsigned char mtu1_resv2:1;
    unsigned char mtu1_fiber:1;
    unsigned char mtu1_resv3:1;
    unsigned char mtu1_front_netport:1;
    unsigned char mtu1_back_netport1:1;
    unsigned char mtu1_back_netport2:1;
    
    unsigned char mtu1_net_port8:1;
    unsigned char mtu1_net_port7:1;
    unsigned char mtu1_net_port6:1;
    unsigned char mtu1_net_port5:1;
    unsigned char mtu1_net_port4:1;
    unsigned char mtu1_net_port3:1;
    unsigned char mtu1_net_port2:1;
    unsigned char mtu1_net_port1:1;
    
    unsigned char mtu1_io1_state:2;
    unsigned char mtu1_io2_state:2;
    unsigned char mtu1_io3_state:2;
    unsigned char mtu1_io4_state:2;
    
    unsigned char mtu1_temp_int;         // 整数部分
    unsigned char mtu1_temp_fract;       // 小数部分
    
    unsigned char mtu2_resv1:1;
    unsigned char mtu2_power:1;
    unsigned char mtu2_resv2:1;
    unsigned char mtu2_fiber:1;
    unsigned char mtu2_resv3:1;
    unsigned char mtu2_front_netport:1;
    unsigned char mtu2_back_netport1:1;
    unsigned char mtu2_back_netport2:1;
    
    unsigned char mtu2_net_port8:1;
    unsigned char mtu2_net_port7:1;
    unsigned char mtu2_net_port6:1;
    unsigned char mtu2_net_port5:1;
    unsigned char mtu2_net_port4:1;
    unsigned char mtu2_net_port3:1;
    unsigned char mtu2_net_port2:1;
    unsigned char mtu2_net_port1:1;
    
    unsigned char mtu2_io1_state:2;
    unsigned char mtu2_io2_state:2;
    unsigned char mtu2_io3_state:2;
    unsigned char mtu2_io4_state:2;
    
    unsigned char mtu2_temp_int;         // 整数部分
    unsigned char mtu2_temp_fract;       // 小数部分
}__attribute__((__packed__)) MTT_441_STATE;

typedef union MTT_STATE_TYPE
{
    MTT_STATE       mtt;
    MTT_411_STATE   mtt_411;
    MTT_441_STATE   mtt_441;
};
#endif


/* EPON Message define */
typedef struct EPONU_LIST_REQ{
	unsigned char		type; /* not use */ 
	unsigned char		pon; /* 0~1, EPON port index */
	unsigned char		group;  /* 0~ 10, ONU  批次 */
}__attribute__ ((__packed__)) EPONU_LIST_REQ;

typedef struct EPONU_LIST_ENTRY {
	unsigned char		node_id;
	unsigned char 	distance; /* onu distance, unit: 100m*/
	unsigned char 	state; /* 1:online, 0: offline*/
	unsigned char		mac[6];	/*onu mac address */
	char				onu_name[MAX_NAME2_LEN];	
}__attribute__ ((__packed__)) EPONU_LIST_ENTRY;

typedef struct EPONU_LIST_RSP {
	unsigned char		onu_units; /* onu list count in this response */ 
	unsigned char		more; /*1: has more onu, 0: all onus are included in this response */
	EPONU_LIST_ENTRY  onulist[1];
}__attribute__ ((__packed__)) EPONU_LIST_RSP;

typedef struct EPONU_ONU_REQ{
	unsigned char		pon; /* 0~1, EPON port index */
	unsigned char		node_id;
}__attribute__ ((__packed__)) EPONU_ONU_REQ;

typedef struct EPONU_INFO {
	unsigned char  	pon;
	unsigned char  	node_id;
	char 			onu_name[MAX_NAME2_LEN];	
	unsigned char 	distance; /* onu distance, unit: 100m*/
	unsigned char 	state; /* 1:online, 0: offline*/
	unsigned char		version_FW[2];
	unsigned char 	port_state[4];	/* onu Port 1~4 state, 0:down, 1: up */
}__attribute__ ((__packed__)) EPONU_INFO;

typedef struct EPONU_NAME {
	unsigned char  	pon;
	unsigned char  	node_id;
	unsigned char  	onu_name[MAX_NAME2_LEN];			
}__attribute__ ((__packed__)) EPONU_NAME;

typedef struct EPONU_NETWORK {
	unsigned char  	pon;
	unsigned char  	node_id;
	char 			ip[16];	
	char 			mask[16];	
	char 			gateway[16];	
}__attribute__ ((__packed__)) EPONU_NETWORK;

typedef struct EPONU_UART {
	unsigned char  	pon;
	unsigned char  	node_id;
	unsigned char  	mode;			/*  UART mode 0:232, 1:485 */
	unsigned char  	baudrate;		/* bit(2...0):"111"~"000" baudrate:115200,57600,38400,19200,9600,4800,2400,1200*/
	unsigned short  	tcp_port;		/* tcp listen port*/
}__attribute__ ((__packed__)) EPONU_UART;

typedef struct EPONU_BW {
	unsigned char  	pon;
	unsigned char  	node_id;
	unsigned long  	max_bandwidth;			/* unit: kbps  */
	unsigned char  	vlan;		/* 1: use "vlan", 0: not use */
}__attribute__ ((__packed__)) EPONU_BW;

typedef struct EPONU_IO {
	unsigned char  	pon;
	unsigned char  	node_id;
	char			as_ip[16];	/* alarm server IP */
	unsigned short 	as_port;		/* alarm server port */
	unsigned short 	local_port;	/* local port, used for receive alarm out */
	unsigned short 	dev_id;		/* device ID */
	unsigned char 	alarm_mode;	/* 0: triger alarm with open circut, 1: close circut*/
	unsigned char 	alarm_format;	/* 0: long format, short format */
}__attribute__ ((__packed__)) EPONU_IO;

#if 1 // 20150417
typedef struct EPONU_IOCTRL {
	unsigned char  	pon;
    unsigned char   node_id;		/*onu id, max 24*/ /*20150514*/
	unsigned char  	iono;           /*1~2*/
    unsigned char    iocmd;         /*1:open, 0:close*/
}__attribute__((__packed__))EPONU_IOCTRL;
#endif

typedef struct EPONU_UPPATH {
	unsigned char  	pon;
	unsigned char  	node_id;
	char 		  	path[MAX_NAME2_LEN];			/* upgrade  path */
}__attribute__ ((__packed__)) EPONU_UPPATH;

typedef struct EPONU_UPSTAT {
	unsigned char  	pon;
	unsigned char  	node_id;
	char 		  	state;			/* 0: OK, ready to upgrade, 1: upgrade in progress, 2: failed*/
}__attribute__ ((__packed__)) EPONU_UPSTAT;

typedef struct EPN_CARD_STATE {
	unsigned char 	fiber1_state;	/*1:sfp removed, 2: up, 3: down, 4:error */
	unsigned char 	fiber2_state;	
}__attribute__ ((__packed__)) EPN_CARD_STATE;

typedef struct EPN_OLT_VLAN {
	unsigned char 	vlan_bitmap[5];	/* bit0: PON0, bit1: PON1, bit2:GE0, bit3: GE1, bit4: back */
}__attribute__ ((__packed__)) EPN_OLT_VLAN;

typedef struct HDEC_CFG {
	unsigned char  	source_type;
	unsigned char  	frame_rate;
	unsigned char  	dec_mode;
	unsigned char  	cvbs_mode;
	unsigned short  	filter;
	unsigned char  	display_mode1;
	unsigned char  	output_mode1;
	unsigned char		transport_type;	
	unsigned char		pj_no[2];	
	unsigned char  	output_type[2];
	char 			reserved1[11];	
	unsigned short  	group_port1;
	unsigned char  	display_mode2;
	unsigned char  	output_mode2;
	char 			group_ip2[16];	
	unsigned short  	group_port2;
}__attribute__ ((__packed__)) HDEC_CFG;


