namespace multilang {

void init();
const char *getLabel(const char *name);
const char *getError(int  id);

/* get Ethernet port working state */
const char *getEPortState(unsigned char state);

/* get SFP optical port working state */
const char *getOPortState(unsigned char state);

/* get Admin  state */
const char *getAdminState(unsigned char state);

/* get alarm level*/
const char *getSeverity(int level);

/* get alarm type name  */
const char *getAlarmName(int alarm_type, int card_type);

/* get alarm device  */
const char *getDevice(int alarm_type, int cardno, int nodeid);

/* get perf head name */
const char *getPerfhead(int cardno, int nodeid);

/* get port statistic count head name */
const char *getstatisticname(int type, int countno);

/* get port broadcast storm ctrl name */
const char *getstormstate(unsigned char state);

/* get port QoS ctrl name */
const char *getport1priorityEnable(unsigned char state);

/* get port QoS ctrl name */
const char *getport2priorityEnable(unsigned char state);

/* get port QoS ctrl name */
const char *getport3priorityEnable(unsigned char state);

/* get port QoS ctrl name */
const char *getport4priorityEnable(unsigned char state);

/* get port QoS ctrl name */
const char *getport5priorityEnable(unsigned char state);

/*get onu data channel*/
const char *getdatach(unsigned char type);

/* get port statistic count type */
const char *getstatistictype(unsigned char type);

const char *getsnoopingstate(unsigned char state);

/* get SFP modul state */
const char *getSfpState(unsigned char state);

const char *getAlarmMode(unsigned char mode);
const char *getAlarmFormat(unsigned char format);
const char *getAlarmFormat2(unsigned char format);
const char *getDisplayMode(unsigned char mode);
char *getVOutputList(int disp_mode1, int disp_mode2);

}
