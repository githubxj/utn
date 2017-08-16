/************************************************************
*	getFSAState: query the FSA board states
*	param-out:  FSA_CARD_STATE *, the pointer of buffer that hold the result
*	return:  	0  - success
*			-1 - fail
************************************************************/
int getFSAState (FSA_CARD_STATE * state);

/***********************************************************
*	getONUList: get the list of ONU nodes that connected to the FSA
*	param-out:  ONU_LIST_ENTRY* , the first pointer of entry buffer that hold the result
*	return:  	-1 - fail
*			other - the number ONU nodes
***********************************************************/
int getONUList(ONU_LIST_ENTRY* pList);

int getONUDSNList(char* pList);

/***********************************************************
*	setFSAAdmin: set the FSA card and port admin state
*	param-in:  index 0: FSA card, 1~2 GE port
*	param-in:  admin_state 2: disable, 1: enable
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setFSAAdmin(unsigned char index, unsigned char admin_state);


/***********************************************************
*	getONUInfo: get the ONU node data
*	param-in:  onu id
*	param-out:  ONU_LIST_ENTRY* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUInfo(unsigned char onu_id, ONU_INFO * onu_info);

/***********************************************************
*	getONUUartInfo: get the ONU node Uart Info
*	param-in:  onu id
*	param-out:  ONU_UART_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUUartInfo(unsigned char onu_id, ONU_UART_CONF *onuUartConfInfo, unsigned char *num);


/***********************************************************
*	getONUPortMirror: get the ONU node Port Mirror
*	param-in:  onu id
*	param-out:  ONU_PORTMIRROR_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUPortMirror(unsigned char onu_id, ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo);


/***********************************************************
*	getONUQoS: get the ONU node QoS
*	param-in:  onu id
*	param-out:  ONU_QoS_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUQoS(unsigned char onu_id, ONU_QoS_CONF *onuQoSConfInfo);


/***********************************************************
*	getONUVLAN: get the ONU node VLAN
*	param-in:  onu id
*	param-out:  ONU_VLAN_CONF* , the  pointer of  buffer that hold the result
*	param-out:  unsigned char* , the  pointer of  VLAN Table Number that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int getONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANConfInfo, unsigned char *num);


/***********************************************************
*	setONUName: set the ONU node name
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUName(unsigned char onu_id, char * name);


/***********************************************************
*	setONUStorm: set the ONU storm
*	param-in:  onu id
*	param-in:  broadcaststorm_ctrl 1:enable, 2: disable
*	param-in:  storm_threshold
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUStorm(unsigned char onu_id, ONU_SET * onuSet);


/***********************************************************
*	setONUAdmin: set the ONU node and port admin state
*	param-in:  onu id
*	param-in:  index 0: ONU self, 1~4 ONU FE port
*	param-in:  admin_state 2: disable, 1: enable
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUAdmin(unsigned char onu_id,  unsigned char index, unsigned char admin_state);


/***********************************************************
*	setONUUart: set the ONU node Uart
*	param-in:  onu id
*	param-in:  bus_Baudrate bit(7):232/485  (0:232,1:485),bit(2...0):"111"~"000" baudrate:115200,57600,38400,19200,9600,4800,2400,1200
*	param-in:  Function 0x00:debug port,0x01:data comm
*	param-in:  Time RX,1ms~256ms
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUUart(unsigned char onu_id, ONU_UART_CONF *onuUartConf);


/***********************************************************
*	setONUPortMirror: set the ONU node Port Mirror
*	param-in:  onu id
*	param-in:  onuPortMirrorConf
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUPortMirror(unsigned char onu_id, ONU_PORTMIRROR_CONF *onuPortMirrorConf );


/***********************************************************
*	setONUQoS: set the ONU node QoS
*	param-in:  onu id
*	param-in:  onuQoSConf
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int setONUQoS(unsigned char onu_id, ONU_QoS_CONF *onuQoSConf);


/***********************************************************
*	saveONUVLAN: save the ONU node VLAN
*	param-in:  onu id
*	param-in:  onuVLANSave
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int saveONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANSave);

/***********************************************************
*	delONUVLAN: delete the ONU node VLAN table
*	param-in:  onu id
*	param-in:  onuVLANDel
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int delONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANDel);

/***********************************************************
*	setONUBW: set the ONU node bandwidth
*	param-in:  onu id
*	param-in:  index 1~4 ONU FE port
*	param-in:  bw 
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUBW(unsigned char onu_id, unsigned char index, unsigned short bw_send,  unsigned short bw_recv);

/***********************************************************
*	setONUFEPort: set the ONU node FE port
*	param-in:  onu id
*	param-in:  index 0~3 ONU FE port
*	param-in:  ETHER_PORT_SET* 
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUFEPort(unsigned char onu_id, unsigned char index, ETHER_PORT_SET* port_set);
int getONUFEPort(unsigned char onu_id, unsigned char index, FE_PORT_STATE * port_info);

int getSFPInfo(unsigned char onu_id, unsigned char index, SFP_INFO * sfp_info);
int getFSASFPInfo(SFP_INFO * sfp_info, int index);


int getPortPerf(unsigned char node_id, int index, PORT_PERF* perf);
int resetPortPerf(unsigned char node_id, int index );

/***********************************************************
*	resetONU: reset the ONU node
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int resetONU(unsigned char onu_id);

/***********************************************************
*	getAlarm: get the  FSA and ONU current alarm information
*	param-out:  ONET_ALARM_INFO* , the first pointer of entry buffer that hold the result
*	param-in:  max_len, the maximun number of alarms that  ONET_ALARM_INFO* can hold
*	return:  	-1 - fail
*			0 - no alarm
*			other - the number alarms
***********************************************************/
int getAlarm(ONET_ALARM_INFO* alarm, int max_len);


/***********************************************************
*	sendRSData: send  serial data to the ONU RS232 port
*	param-in:  onu id
*	param-in:  data, the raw data need to be sent
*	param-in:  data_len, the lenth of sending data
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int sendRSData(unsigned char onu_id, unsigned char port, char * data, int data_len);

/***********************************************************
*	recvRSData: receive  serial data from ONU RS232 port
*	param-out:  onu id that data come from
*	param-out:  data, the raw data need to be sent
*	param-in:  max_len, the maxium lenth of receive data buffer than can store
*	return:  	0  - success , no data received
*			1~... - received data length
*			-1 - fail
***********************************************************/
int proc_recvRSData(unsigned char onu_id, unsigned char port, char * data, int max_len);


int getONUIO(unsigned char onu_id, ONU_IO *onu_io);
int setONUIO(unsigned char onu_id, ONU_IO *onu_io );

void initSetONUUART(unsigned char onu_id, ONU_UART_CONF* uart_set);

void fsalib_cleanup();

int fsalib_init();

int setFSAstate( CARD_SET_FSA *state );

int setPortClockDelay(int cfg_val);

