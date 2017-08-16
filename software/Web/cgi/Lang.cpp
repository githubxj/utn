#include <string.h>
#include <stdio.h>
#include "const.h"
#include "alarm_type.h"
#include "iniFile.h"

namespace  multilang {

/*!  Structure to save multi-language labels */
static const struct langmap {
	char * const name;
	char * const en;
	char * const ch;
} langmaps[] = {
	{ "card", "Card", "卡"},
	{ "alldev", "All Devices", "所有设备"},
	{ "frame", "Frame", "机框"},
	{ "l3", "L3 switch", "三层交换"},
	{ "login_name", "User:", "用户："},
	{ "login_passwd", "Password:","密码："},
	{ "submit", "Submit","提交" },
	{ "modify", "Modify","修改" },
	{ "add", "Add","增加" },
	{ "delete", "Delete","删除" },
	{ "cancel", "Cancel","取消" },
	{ "reset", "Reset","重启" },
	{ "login", "Login","登录" },
	{ "relogin", "Re-Login","重新登录" },
	{ "next", "Next Page","下一页" },
	{ "prev", "Previous Page","上一页" },	
	{ "mtopo", "Network View","传输网络拓扑" },
	{ "mdevice", "Devices","设备管理" },
	{ "malarm", "Alarms","故障告警查询" },
	{ "macur", "Current Alarms","当前故障信息" },
	{ "clearcur", "Alarm Clear","清除故障信息" },
	{"reboot_inform", "The Modification will take effect after system reboot!", "您提交的修改需要系统重启后才能生效！"},
	{ "fan_r", "Working", "正常运转"},
	{ "fan_s", "Stopped", "停止"},
	{"fiber_on","Up","正常"},
	{"fiber_off","Down","无信号"},
	{"view","View","查看"},
#ifdef COVOND
	{"fsa","MAT","MAT"},
	{"onu","MAU","MAU"},
#else
	{"fsa","FSA","FSA"},
	{"onu","ONU","ONU"},
#endif
	{"none","No","无"},
	{ "", "", ""},
	
};


static const struct errormap {
	int const id;
	char * const en;
	char * const ch;
} errormaps[] = {
	{ ERROR_NO, "Operation Successful!", "操作成功！"},
	{ ERROR_SYSTEM_GENERAL, "System internal error!", "系统错误！"},
	{ ERROR_LOGIN_FAIL, "Wrong login user name or password!", "用户名或密码错误！"},
	{ ERROR_LOGIN_TIMEOUT, "Login time! You'll be redirected to login page automaticly in 3 seconds or click the following link to re-login manually.",
				 "登录超时！页面将自动跳转到登录页面，或点击下面链接重新登录。"},
	{ ERROR_WRONG_PWD, "Wrong original password!", "原密码错误！"},
	{ ERROR_COMM_LOCAL, "Local process communication error!", "本地进程通信错误！"},
	{ ERROR_COMM_485, "Card 485 communication error!", "板块485通信错误！"},
	{ ERROR_COMM_FIBER, "Fiber communication error!", "光纤通信错误！"},
	{ ERROR_COMM_PARSE, "Communication message parse error!", "通信消息解析错误！"},
	{ ERROR_COMM, "Communication error!", "通信错误！"},
	{ ERROR_WRONG_ARGS, "Command argument error!", "命令参数错误！"},
	{ ERROR_DATETIME_FORMAT, "Date and Time format error!", "日期与时间格式错误！"},
	{ ERR_SFP_INCOMPATIBLE,"SFP is incompatible!","SFP不兼容！"},
	{ ERROR_NEWID_EXIST,"New Node ID is alreay used!","新的节点号已存在！"},
	{ ERROR_DSN_NOTMATCH,"Device serial number does not match with the node!","设备序列号错误！"},
	{ ERROR_NOTFOUND, "Object not found!","对象未找到！"},
	{ ERROR_LIMITATION, "Exceeds system limitation!","超出系统容量！"},
	{ ERROR_DATABASE, "Database error!","数据库错误！"},
	{ ERROR_I2C, "I2C bus error!","I2C读写错误！"},
	{ ERROR_HDMI, "HDMI Output!","HDMI输出！"},
	{ ERROR_UP_INPROGRESS, "Upgrade in progress...!","升级进行中...！"},
	{ ERROR_UP_FILE, "Upgrade file error!","升级文件处理错误！"},
	{ ERROR_UP_ERROR, "Upgrade failed!","升级失败！"},
};

static const struct asmap {
	char * const en;
	char * const ch;
} asmaps[] = {
	{ "Disabled", "禁用"},
	{ "Enabled", "启用"},
};

static char null_label[] = " ";
static char lang = 'c';
char buff[128];
void init()
{
	IniFilePtr confFile;
	const char * var;

	confFile = iniFileOpen(WEB_CONF_FILE);
	if(confFile == 0)
		return;

	var = iniGetStr(confFile, "sys", LANG_KEYNAME);
	if(var != NULL)
	{
		lang = *var;
	}
	else 
		lang = 'c';

	iniFileFree(confFile);
	return;

}

const char *getLabel(const char *name)
{
	int x;

	if(name == 0)
		return null_label;
	
	for (x=0;x<sizeof(langmaps) / sizeof(langmaps[0]); x++) {
		if (!strcasecmp(langmaps[x].name, name)) {
			if(lang == 'c')
				return langmaps[x].ch;
			else
				return langmaps[x].en;
		}
	}

	/* no predefined label found, return the label name itself */
	return name;
}

const char *getError(int id)
{
	int x;

	for (x=0;x<sizeof(errormaps) / sizeof(errormaps[0]); x++) {
		if ( errormaps[x].id == id) {
			if(lang == 'c')
				return errormaps[x].ch;
			else
				return errormaps[x].en;
		}
	}

	// return general error
	if(lang == 'c')
		return errormaps[1].ch;
	else
		return errormaps[1].en;
}

const char *getAdminState(unsigned char state)
{
	if(state !=0)
		state = 1;

	if(lang == 'c')
		return asmaps[state].ch;
	else
		return asmaps[state].en;
	
}

const char *getSfpState(unsigned char state)
{	
	if(lang == 'c')
	{
		switch(state)
		{		
			case 1:			return("模块已移除");
			case 2:			return ("正常");
			case 3:			return ("无信号");
			case 4:			return ("错误");
			default:			return ("未知");
		}
	} else {
		switch(state)
		{		
			case 1:			return("Removed");
			case 2:			return ("Up");
			case 3:			return ("Down");
			case 4:			return ("Error");
			default:			return ("Unknown");
		}
	}
}


const char *getSeverity(int level)
{	
	if(lang == 'c')
	{
		switch(level)
		{		
			case ALARM_SERVERITY_ERROR:			return("严重");
			case ALARM_SERVERITY_WARNING:			return ("主要");
			case ALARM_SERVERITY_INFO:			return ("普通");
			default:			return ("未知");
		}
	} else {
		switch(level)
		{		
			case ALARM_SERVERITY_ERROR:			return("ERROR");
			case ALARM_SERVERITY_WARNING:			return ("WARNING");
			case ALARM_SERVERITY_INFO:			return ("INFORM");
			default:			return ("Unknown");
		}
	}
}


const char *getPerfhead(int cardno, int nodeid)
{	

	if(lang == 'c')
	{
			if ( 0 == nodeid )
			{
					sprintf(buff, "%d号卡", cardno);
					
			}
			else
			{
#ifdef COVOND
					sprintf(buff, "%d号卡%d号MAU", cardno ,nodeid);
#else
					sprintf(buff, "%d号卡%d号ONU", cardno ,nodeid);
#endif
			}
	} 
	else
 {
			if ( 0 == nodeid )
			{
					sprintf(buff, "Card-%d", cardno);
					
			}
			else
			{
#ifdef COVOND
					sprintf(buff, "Card-%d MAU-%d", cardno ,nodeid);
#else
					sprintf(buff, "Card-%d ONU-%d", cardno ,nodeid);
#endif
			}
	}
	
	return buff;
}


const char *getstatistictype(unsigned char type)
{
	if ( lang == 'c' )
	{
			switch(type)
				{

					case 1:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' selected> 收发包统计</option>\
								      <option value='2' > 冲突包统计</option>\
								      <option value='3' > 丢弃包统计</option>\
								      <option value='4' > 错误包统计</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > 收发包统计</option>\
								      <option value='2' selected> 冲突包统计</option>\
								      <option value='3' > 丢弃包统计</option>\
								      <option value='4' > 错误包统计</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > 收发包统计</option>\
								      <option value='2' > 冲突包统计</option>\
								      <option value='3' selected> 丢弃包统计</option>\
								      <option value='4' > 错误包统计</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > 收发包统计</option>\
								      <option value='2' > 冲突包统计</option>\
								      <option value='3' > 丢弃包统计</option>\
								      <option value='4' selected> 错误包统计</option>\
								      </select>");							
						default:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > 收发包统计</option>\
								      <option value='2' > 冲突包统计</option>\
								      <option value='3' > 丢弃包统计</option>\
								      <option value='4' > 错误包统计</option>\
								      <option value='5' selected> 不支持</option></select>");	
		}

				
	}
	else
	{
			switch(type)
				{

					case 1:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' selected> Packets Statistics</option>\
								      <option value='2' > Collision Packets Statistics</option>\
								      <option value='3' > Drop Packets Statistics</option>\
								      <option value='4' > Error Packets Statistics</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > Packets Statistics</option>\
								      <option value='2' selected> Collision Packets Statistics</option>\
								      <option value='3' > Drop Packets Statistics</option>\
								      <option value='4' > Error Packets Statistics</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > Packets Statistics</option>\
								      <option value='2' > Collision Packets Statistics</option>\
								      <option value='3' selected> Drop Packets Statistics</option>\
								      <option value='4' > Error Packets Statistics</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > Packets Statistics</option>\
								      <option value='2' > Collision Packets Statistics</option>\
								      <option value='3' > Drop Packets Statistics</option>\
								      <option value='4' selected> Error Packets Statistics</option>\
								      </select>");							
						default:
							return("<select class='fixsize'  name='statistic_type' id='statistic_type'> \
								      <option value='1' > Packets Statistics</option>\
								      <option value='2' > Collision Packets Statistics</option>\
								      <option value='3' > Drop Packets Statistics</option>\
								      <option value='4' > Error Packets Statistics</option>\
								      <option value='5' selected> Not-Supported</option></select>");	
		}
	}



}


const char *getsnoopingstate(unsigned char state)
{
	
	if ( lang == 'c' )
	{
			switch(state)
				{

					case 1:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' selected> 启用</option>\
								      <option value='2' > 禁用</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' > 启用</option>\
								      <option value='2' selected> 禁用</option>\
								      </select>");			
						
						default:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' > 启用</option>\
								      <option value='2' > 禁用</option>\
								      <option value='3' selected> 不支持</option></select>");	
		}

				
	}
	else
	{
			switch(state)
				{

					case 1:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' selected> ON</option>\
								      <option value='2' > OFF</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' > ON</option>\
								      <option value='2' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize'  name='igmp_snooping' id='igmp_snooping'> \
								      <option value='1' > ON</option>\
								      <option value='2' > OFF</option>\
								      <option value='3' selected> Not-Supported</option></select>");	
		}
	}

	
	
}



const char *getstormstate(unsigned char state)
{
	
	if ( lang == 'c' )
	{
			switch(state)
				{

					case 1:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' selected> 启用</option>\
								      <option value='2' > 禁用</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' > 启用</option>\
								      <option value='2' selected> 禁用</option>\
								      </select>");			
						
						default:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' > 启用</option>\
								      <option value='2' > 禁用</option>\
								      <option value='3' selected> 不支持</option></select>");	
		}

				
	}
	else
	{
			switch(state)
				{

					case 1:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' selected> ON</option>\
								      <option value='2' > OFF</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' > ON</option>\
								      <option value='2' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='storm_ctrl' name='storm_ctrl' onchange='onloadstorm();'> \
								      <option value='1' > ON</option>\
								      <option value='2' > OFF</option>\
								      <option value='3' selected> Not-Supported</option></select>");	
		}
	}
	
	
	
}

const char *getAlarmMode(unsigned char mode)
{
	if ( lang == 'c' )
	{
			switch(mode)
			{
					case 1:
							return("<select class='fixsize'  name='alarm_mode' id='alarm_mode'> \
								      <option value='0' > 常开</option>\
								      <option value='1' selected> 常闭</option>\
								      </select>");			
					default:
							return("<select class='fixsize'  name='alarm_mode' id='alarm_mode'> \
								      <option value='0' selected> 常开</option>\
								      <option value='1' > 常闭</option>\
								      </select>");			
		}
	}
	else
	{
			switch(mode)
			{
					case 1:
							return("<select class='fixsize'  name='alarm_mode' id='alarm_mode'> \
								      <option value='0' > Open </option>\
								      <option value='1' selected> Closed </option>\
								      </select>");			
					default:
							return("<select class='fixsize'  name='alarm_mode' id='alarm_mode'> \
								      <option value='0' selected> Open</option>\
								      <option value='1' > Closed</option>\
								      </select>");			
		}
	}
}

const char *getAlarmFormat(unsigned char format)
{
	if ( lang == 'c' )
	{
			switch(format)
			{
					case 1:
							return("<select class='fixsize' id='alarm_format' name='alarm_format'> \
								      <option value='0' > 短格式</option>\
								      <option value='1' selected> 长格式</option>\
								      </select>");			
					default:
							return("<select class='fixsize' id='alarm_format' name='alarm_format'> \
								      <option value='0' selected> 短格式</option>\
								      <option value='1' > 长格式</option>\
								      </select>");			
		}
	}
	else
	{
			switch(format)
			{
					case 1:
							return("<select class='fixsize' id='alarm_format' name='alarm_format'> \
								      <option value='0' > Short </option>\
								      <option value='1' selected> Long </option>\
								      </select>");			
					default:
							return("<select class='fixsize' id='alarm_format' name='alarm_format'> \
								      <option value='0' selected> Short</option>\
								      <option value='1' > Long</option>\
								      </select>");			
		}
	}
}

const char *getAlarmFormat2(unsigned char format)
{
	if ( lang == 'c' )
	{
			switch(format)
			{
					case 1:
							return("<select class='fixsize' id='alarm_format2' name='alarm_format2'> \
								      <option value='0' > 短格式</option>\
								      <option value='1' selected> 长格式</option>\
								      </select>");			
					default:
							return("<select class='fixsize' id='alarm_format2' name='alarm_format2'> \
								      <option value='0' selected> 短格式</option>\
								      <option value='1' > 长格式</option>\
								      </select>");			
		}
	}
	else
	{
			switch(format)
			{
					case 1:
							return("<select class='fixsize' id='alarm_format2' name='alarm_format2'> \
								      <option value='0' > Short </option>\
								      <option value='1' selected> Long </option>\
								      </select>");			
					default:
							return("<select class='fixsize' id='alarm_format2' name='alarm_format2'> \
								      <option value='0' selected> Short</option>\
								      <option value='1' > Long</option>\
								      </select>");			
		}
	}
}


const char *getport1priorityEnable(unsigned char state)
{
	if ( lang == 'c' )
	{
		switch(state)
		{
			case 1:
				return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
				<option value='1' selected> 启用</option>\
				<option value='0' > 禁用</option>\
				</select>");			
			case 0:
				return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' selected> 禁用</option>\
				</select>");			
						
			default:
				return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' > 禁用</option>\
				<option value='2' selected> 不支持</option></select>");	
		}			
	}
	else
	{
			switch(state)
				{
					case 1:
							return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
								      <option value='1' selected> ON</option>\
								      <option value='0' > OFF</option>\
								      </select>");			
						case 0:
							return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='port1priority_enable' name='port1priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' > OFF</option>\
								      <option value='2' selected> Not-Supported</option></select>");	
		}
	}	
}


const char *getport2priorityEnable(unsigned char state)
{
	if ( lang == 'c' )
	{
		switch(state)
		{
			case 1:
				return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
				<option value='1' selected> 启用</option>\
				<option value='0' > 禁用</option>\
				</select>");			
			case 0:
				return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' selected> 禁用</option>\
				</select>");			
						
			default:
				return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' > 禁用</option>\
				<option value='2' selected> 不支持</option></select>");	
		}			
	}
	else
	{
			switch(state)
				{
					case 1:
							return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
								      <option value='1' selected> ON</option>\
								      <option value='0' > OFF</option>\
								      </select>");			
						case 0:
							return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='port2priority_enable' name='port2priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' > OFF</option>\
								      <option value='2' selected> Not-Supported</option></select>");	
		}
	}	
}

const char *getport3priorityEnable(unsigned char state)
{
	if ( lang == 'c' )
	{
		switch(state)
		{
			case 1:
				return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
				<option value='1' selected> 启用</option>\
				<option value='0' > 禁用</option>\
				</select>");			
			case 0:
				return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' selected> 禁用</option>\
				</select>");			
						
			default:
				return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' > 禁用</option>\
				<option value='2' selected> 不支持</option></select>");	
		}			
	}
	else
	{
			switch(state)
				{
					case 1:
							return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
								      <option value='1' selected> ON</option>\
								      <option value='0' > OFF</option>\
								      </select>");			
						case 0:
							return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='port3priority_enable' name='port3priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' > OFF</option>\
								      <option value='2' selected> Not-Supported</option></select>");	
		}
	}	
}

const char *getport4priorityEnable(unsigned char state)
{
	if ( lang == 'c' )
	{
		switch(state)
		{
			case 1:
				return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
				<option value='1' selected> 启用</option>\
				<option value='0' > 禁用</option>\
				</select>");			
			case 0:
				return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' selected> 禁用</option>\
				</select>");			
						
			default:
				return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' > 禁用</option>\
				<option value='2' selected> 不支持</option></select>");	
		}			
	}
	else
	{
			switch(state)
				{
					case 1:
							return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
								      <option value='1' selected> ON</option>\
								      <option value='0' > OFF</option>\
								      </select>");			
						case 0:
							return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='port4priority_enable' name='port4priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' > OFF</option>\
								      <option value='2' selected> Not-Supported</option></select>");	
		}
	}	
}

const char *getport5priorityEnable(unsigned char state)
{
	if ( lang == 'c' )
	{
		switch(state)
		{
			case 1:
				return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
				<option value='1' selected> 启用</option>\
				<option value='0' > 禁用</option>\
				</select>");			
			case 0:
				return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' selected> 禁用</option>\
				</select>");			
						
			default:
				return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
				<option value='1' > 启用</option>\
				<option value='0' > 禁用</option>\
				<option value='2' selected> 不支持</option></select>");	
		}			
	}
	else
	{
			switch(state)
				{
					case 1:
							return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
								      <option value='1' selected> ON</option>\
								      <option value='0' > OFF</option>\
								      </select>");			
						case 0:
							return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' selected> OFF</option>\
								      </select>");			
			
						default:
							return("<select class='fixsize' id='port5priority_enable' name='port5priority_enable' onchange='onloadscheme();'> \
								      <option value='1' > ON</option>\
								      <option value='0' > OFF</option>\
								      <option value='2' selected> Not-Supported</option></select>");	
		}
	}	
}

const char *getdatach(unsigned char type)
{
	
	if ( lang == 'c' )
	{
			switch(type)
				{

					case 0:
							return("<option value='0' selected> 调试端口</option>\
								      <option value='1' > 数据通讯</option>");			
						case 1:
							return("<option value='0' > 调试端口</option>\
								      <option value='1' selected> 数据通讯</option>");			
						
						default:
							return("<option value='0' > 调试端口</option>\
								      <option value='1' > 数据通讯</option>\
								      <option value='2' selected> 不支持</option>");	
				}				
	}
	else
	{
			switch(type)
				{

					case 0:
							return("<option value='0' selected> Debug Port</option>\
								      <option value='1' > Data Communication</option>");			
						case 1:
							return("<option value='0' > Debug Port</option>\
								      <option value='1' selected> Data Communication</option>");			
			
						default:
							return("<option value='0' > Debug Port</option>\
								      <option value='1' > Data Communication</option>\
								      <option value='2' selected> Not-Supported</option>");	
		}
	}
}

const char *getDisplayMode(unsigned char mode)
{
	if ( lang == 'c' )
	{
			switch(mode)
			{
					case 16:
							return("<option value='0' >高清单画面</option>\
								      <option value='16' selected>高清4画面</option>\
								      <option value='21' >拼接4画面</option>\
								      <option value='22' >拼接9画面</option>" );			
					case 21:
							return("<option value='0' >高清单画面</option>\
								      <option value='16' >高清4画面</option>\
								      <option value='21' selected>拼接4画面</option>\
								      <option value='22' >拼接9画面</option>" );			
					case 22:
							return("<option value='0' >高清单画面</option>\
								      <option value='16' >高清4画面</option>\
								      <option value='21' >拼接4画面</option>\
								      <option value='22' selected>拼接9画面</option>" );			
					default:
							return("<option value='0' selected>高清单画面</option>\
								      <option value='16' >高清4画面</option>\
								      <option value='21' >拼接4画面</option>\
								      <option value='22' >拼接9画面</option>" );			
		  }
	}
	else
	{
			switch(mode)
			{
					case 16:
							return("<option value='0' >Single Picture</option>\
								      <option value='16' selected>4 Pictures In One</option>\
								      <option value='21' >Combined 4</option>\
								      <option value='22' >Combined 9</option>" );			
					case 21:
							return("<option value='0' >Single Picture</option>\
								      <option value='16' >4 Pictures In One</option>\
								      <option value='21' selected>Combined 4</option>\
								      <option value='22' >Combined 4</option>" );			
					case 22:
							return("<option value='0' >Single Picture</option>\
								      <option value='16' >4 Pictures In One</option>\
								      <option value='21' >Combined 4</option>\
								      <option value='22' selected>Combined 4</option>" );			
					default:
							return("<option value='0' selected>Single Picture</option>\
								      <option value='16' >4 Pictures In One</option>\
								      <option value='21' >Combined 4</option>\
								      <option value='22' >Combined 4</option>" );			
		}
	}
}


char *getVOutputList(int disp_mode1, int disp_mode2)
{
	static char buffer[1024];
	int flag;

	strcpy(buffer, "");	
	if ( lang == 'c' )
	{
		flag=1;
		if((disp_mode1 != 21) && (disp_mode1 != 22))
		{
			//not pj mode
			flag=0;
			if(disp_mode1 == 16)
			{
				strcat(buffer, "<option value='0' >输出通道1画面1</option>");
				strcat(buffer, "<option value='1' >输出通道1画面2</option>");
				strcat(buffer, "<option value='2' >输出通道1画面3</option>");
				strcat(buffer, "<option value='3' >输出通道1画面4</option>");
			}
			else
				strcat(buffer, "<option value='0' >输出通道1</option>");
		}
		
		if((disp_mode2 != 21) && (disp_mode2 != 22))
		{
			//not pj mode
			flag=0;
			if(disp_mode2 == 16)
			{
				strcat(buffer, "<option value='16' >输出通道2画面1</option>");
				strcat(buffer, "<option value='17' >输出通道2画面2</option>");
				strcat(buffer, "<option value='18' >输出通道2画面3</option>");
				strcat(buffer, "<option value='19' >输出通道2画面4</option>");
			}
			else
				strcat(buffer, "<option value='16' >输出通道2</option>");
		}
	
		if(flag)
			strcat(buffer, "<option value='-1' >拼接模式不支持切换！</option>");
	} else {
		flag=1;
		if((disp_mode1 != 21) && (disp_mode1 != 22))
		{
			//not pj mode
			flag=0;
			if(disp_mode1 == 16)
			{
				strcat(buffer, "<option value='0' >Output 1 ch-1</option>");
				strcat(buffer, "<option value='1' >Output 1 ch-2</option>");
				strcat(buffer, "<option value='2' >Output 1 ch-3</option>");
				strcat(buffer, "<option value='3' >Output 1 ch-4</option>");
			}
			else
				strcat(buffer, "<option value='0' >Output 1</option>");
		}
		
		if((disp_mode2 != 21) && (disp_mode2 != 22))
		{
			//not pj mode
			flag=0;
			if(disp_mode2 == 16)
			{
				strcat(buffer, "<option value='0' >Output 2 ch-1</option>");
				strcat(buffer, "<option value='1' >Output 2 ch-2</option>");
				strcat(buffer, "<option value='2' >Output 2 ch-3</option>");
				strcat(buffer, "<option value='3' >Output 2 ch-4</option>");
			}
			else
				strcat(buffer, "<option value='0' >Output 2</option>");
		}
	
		if(flag)
			strcat(buffer, "<option value='-1' >Not Support in Combined Mode!</option>");
	}
	
	return(buffer);
}


const char *getstatisticname(int type, int countno)
{
	if(lang == 'c')
	{
			switch(type)
			{
					case 2:
						if ( 1 == countno )
							return ("冲突包统计");
						else
							return ("发送包统计");
					case 3:
						if ( 1 == countno )
							return ("丢弃包统计");
						else
							return ("接收包统计");
							
					case 4:
						if ( 1 == countno )
							return ("CRC错误包统计");
						else
							return ("接收包统计");				
					default:
						if ( 1 == countno )
							return ("发送包统计");
						else
							return ("接收包统计");	
							
			}
	} 
	else
 {
			switch(type)
			{
					case 2:
						if ( 1 == countno )
							return ("Collision packet count");
						else
							return ("Transmit packet count");
					case 3:
						if ( 1 == countno )
							return ("Drop packet count");
						else
							return ("Receive packet count");
							
					case 4:
						if ( 1 == countno )
							return ("CRC error packet count");
						else
							return ("Receive packet count");				
					default:
						if ( 1 == countno )
							return ("Transmit packet count");
						else
							return ("Receive packet count");	
							
			}
	}
	
	
}

const char *getAlarmName(int alarm_type, int card_type)
{
	if(lang == 'c')
	{
		switch(alarm_type) {
			case ATYPE_PSAON:
				return("电源A上电");
			case ATYPE_PSAOFF:
				return("电源A掉电");
			case ATYPE_PSBON:
				return("电源B上电");
			case ATYPE_PSBOFF:
				return("电源B掉电");
			case ATYPE_FANON:
				return("风扇启动");
			case ATYPE_FANOFF:
				return("风扇关闭");
			case ATYPE_CARDONLINE:
				return("板卡上线");
			case ATYPE_CARDOFFLINE:
				return("板卡下线");
			case ATYPE_CARDFAILURE:
				return("板卡错误");
			case ATYPE_CARDRESTART:
				return("板卡重启");	
			case ATYPE_ONUONLINE:
				if( CARD_TYPE_EPN == card_type)
						return ("ONU上线");
				else
#ifdef COVOND
				return("MAU上线");
#else
				return("ONU上线");
#endif
			case ATYPE_ONUOFFLINE:
				if( CARD_TYPE_EPN == card_type)
						return ("ONU下线");
				else
#ifdef COVOND
				return("MAU下线");
#else
				return("ONU下线");
#endif
			case ATYPE_ONUSFPINSERTED:
#ifdef COVOND
				return("MAU SFP 光模块插入");
#else
				return("ONU SFP 光模块插入");
#endif
			case ATYPE_ONUSFPREMOVED:
#ifdef COVOND
				return("MAU SFP 光模块移除");	
#else
				return("ONU SFP 光模块移除");	
#endif
			case ATYPE_ONUSFPERROR:
#ifdef COVOND
				return("MAU SFP 光模块故障");	
#else
				return("ONU SFP 光模块故障");	
#endif
			case ATYPE_ONUPORTUP:
				if( CARD_TYPE_EPN == card_type)
						return ("ONU电口上线");
				else
#ifdef COVOND
				return("MAU电口上线");
#else
				return("ONU电口上线");
#endif
			case ATYPE_ONUPORTDOWN:
				if( CARD_TYPE_EPN == card_type)
						return ("ONU电口下线");
				else
#ifdef COVOND
				return("MAU电口下线");	
#else
				return("ONU电口下线");	
#endif
			case ATYPE_ONETRINGSTANDBYFAILURE:
				return("备环错误");
			case ATYPE_ONETRINGSTANDBYRESTORE:
				return("备环错误恢复");				
			case ATYPE_ONETRINGFAILURE:
				return("光纤环错误");	
			case ATYPE_ONETRINGRESTORE:
				return("光纤环恢复");
			case ATYPE_ONETRINGBROKEN:
				return("光纤环断开");
			case ATYPE_ONETLINKBROKEN:
				return("光纤链路断开");
			case ATYPE_ONETLINKRESTORE:
				return("光纤链路恢复");
			case ATYPE_SFPINSERTED:
				return("SFP光模块插入");
			case ATYPE_SFPREMOVED:
				return("SFP光模块移除");
			case ATYPE_PORTUP:
				return("电口上线");
			case ATYPE_PORTDOWAN:
				return("电口下线");
			case ATYPE_RMTPOWERDOWN:
				return ("远端掉电");
			case ATYPE_FXUP:
				return ("光口上线");
			case ATYPE_FXDOWN:
				return ("光口下线");
			case ATYPE_FANFAIL:
				return ("风扇故障");
			default:
				return("系统故障");	

		}
	} else {
		switch(alarm_type) {
			case ATYPE_PSAON:
				return("PSAON");
			case ATYPE_PSAOFF:
				return("PSAOFF");
			case ATYPE_PSBON:
				return("PSBON");
			case ATYPE_PSBOFF:
				return("PSBOFF");
			case ATYPE_FANON:
				return("FANON");
			case ATYPE_FANOFF:
				return("FANOFF");
			case ATYPE_CARDONLINE:
				return("CARDONLINE");
			case ATYPE_CARDOFFLINE:
				return("CARDOFFLINE");
			case ATYPE_CARDFAILURE:
				return("CARDFAILURE");
			case ATYPE_CARDRESTART:
				return("CARDRESTART");
			case ATYPE_ONUONLINE:
				if( CARD_TYPE_EPN == card_type)
						return ("ONUONLINE");
				else
#ifdef COVOND
				return("MAUUONLINE");
#else
				return("ONUONLINE");
#endif
			case ATYPE_ONUOFFLINE:
				if( CARD_TYPE_EPN == card_type)
						return ("ONUOFFLINE");
				else
#ifdef COVOND
				return("MAUOFFLINE");
#else
				return("ONUOFFLINE");
#endif
			case ATYPE_ONUSFPINSERTED:
#ifdef COVOND
				return("MAUSFPINSERTED");
#else
				return("ONUSFPINSERTED");
#endif
			case ATYPE_ONUSFPREMOVED:
#ifdef COVOND
				return("MAUSFPREMOVED");
#else
				return("ONUSFPREMOVED");
#endif
			case ATYPE_ONUSFPERROR:
#ifdef COVOND
				return("MAUSFPERROR");
#else
				return("ONUSFPERROR");
#endif
			case ATYPE_ONUPORTUP:
				if( CARD_TYPE_EPN == card_type)
						return ("ONUPORTUP");
				else
#ifdef COVOND
				return("MAUPORTUP");
#else
				return("ONUPORTUP");
#endif
			case ATYPE_ONUPORTDOWN:
				if( CARD_TYPE_EPN == card_type)
						return ("ONUPORTDOWN");
				else
#ifdef COVOND
				return("MAUPORTDOWN");
#else
				return("ONUPORTDOWN");
#endif
			case ATYPE_ONETRINGSTANDBYFAILURE:
				return("RINGSTANDBYFAILURE");
			case ATYPE_ONETRINGSTANDBYRESTORE:
				return("RINGSTANDBYRESTORE");
			case ATYPE_ONETRINGFAILURE:
				return("RINGFAILURE");
			case ATYPE_ONETRINGRESTORE:
				return("RINGRESTORE");
			case ATYPE_ONETRINGBROKEN:
				return("RINGBROKEN");
			case ATYPE_ONETLINKBROKEN:
				return("LINKBROKEN");
			case ATYPE_ONETLINKRESTORE:
				return("LINKRESTORE");
			case ATYPE_SFPINSERTED:
				return("SFPINSERTED");
			case ATYPE_SFPREMOVED:
				return("SFPREMOVED");
			case ATYPE_PORTUP:
				return("PORTUP");
			case ATYPE_PORTDOWAN:
				return("PORTDOWN");
			case ATYPE_RMTPOWERDOWN:
				return ("REMOTEPOWERDOWN");
			case ATYPE_FXUP:
				return ("FXUP");
			case ATYPE_FXDOWN:
				return ("FXDOWN");
			case ATYPE_FANFAIL:
				return ("FANFAIL");
			default:
				return("System");

		}
	}
}


const char *getDevice(int alarm_type, int card_type, int nodeid)
{
		if (lang == 'c')
		{
				switch(alarm_type)
			 {
					case ATYPE_PSAON:
					case ATYPE_PSAOFF:
					case ATYPE_PSBON:
					case ATYPE_PSBOFF:
						return("电源");
					case ATYPE_FANON:
					case ATYPE_FANOFF:
						return("风扇");
					case ATYPE_CARDONLINE:
					case ATYPE_CARDOFFLINE:
					case ATYPE_CARDFAILURE:
					case ATYPE_CARDRESTART:
						switch (card_type)
						{
							case CARD_TYPE_CTRL:	
								return ("控制卡");
							case CARD_TYPE_FSA:
#ifdef COVOND
								return ("MAT1000卡");
#else
								return ("FSA卡");
#endif
							case CARD_TYPE_DAT:
								return ("DAT1000卡");
							case CARD_TYPE_FE8:
								return ("FE8卡");
							case CARD_TYPE_ENCODE:
								return ("编码卡");
#ifdef __MTT__ // 201611								
							case CARD_TYPE_MTT:
								return ("MTT卡");
#else							   
                            case CARD_TYPE_NTT:
							case CARD_TYPE_NTT_S: // 20150115
								return ("NTT卡");
#endif

#if 1 // 201708
                            case CARD_TYPE_MTT_411:
                                return ("MTT411");
                            case CARD_TYPE_MTT_441:
                                return ("MTT441");
#endif

							case CARD_TYPE_MS4V1H:
								return ("MS4V1H卡");
							case CARD_TYPE_M4H:
								return ("M4H卡");
							case CARD_TYPE_M4S:
								return ("M4S卡");
							case CARD_TYPE_EPN:
								return ("EPON卡");
							case CARD_TYPE_FSA600:
#ifdef COVOND
								return ("DAT600卡");
#else
								return ("FSA600卡");
#endif
							default:	
								return ("卡");
						}
					case ATYPE_ONUONLINE:
					case ATYPE_ONUOFFLINE:
					case ATYPE_ONUSFPINSERTED:
					case ATYPE_ONUSFPREMOVED:
					case ATYPE_ONUPORTUP:
					case ATYPE_ONUPORTDOWN:
						if ( CARD_TYPE_FSA600 == card_type)
#ifdef COVOND
							return ("DAU100");
#else
							return ("ONU100");
#endif
						else if ( CARD_TYPE_EPN == card_type)
								return ("ONU");
						else{
#ifdef COVOND
							return ("MAU");
#else
							return ("ONU");
#endif
						}
					case ATYPE_ONETRINGSTANDBYFAILURE:
					case ATYPE_ONETRINGSTANDBYRESTORE:
					case ATYPE_ONETRINGFAILURE:
					case ATYPE_ONETRINGRESTORE:
					case ATYPE_ONETRINGBROKEN:
					case ATYPE_ONETLINKBROKEN:
					case ATYPE_ONETLINKRESTORE:
#ifdef COVOND
						return ("MAT1000卡");
#else
						return ("FSA卡");
#endif
					case ATYPE_SFPINSERTED:
					case ATYPE_SFPREMOVED:
					case ATYPE_ONUSFPERROR:
						switch (card_type)
						{
							case CARD_TYPE_FSA:
								if(nodeid == 0)
#ifdef COVOND
									return("MAT1000卡");
#else
									return("FSA卡");
#endif
								else
#ifdef COVOND
									return("MAU");
#else
									return("ONU");
#endif
							case CARD_TYPE_DAT:
								if(nodeid == 0)
									return("DAT1000卡");
								else
									return("DAU");
							case CARD_TYPE_FSA600:
#ifdef COVOND
								return ("DAT600卡");
#else
								return ("FSA600卡");
#endif
							default:	
									return ("卡");	
						}
					case ATYPE_PORTUP:
					case ATYPE_PORTDOWAN:
							switch (card_type)
							{
								case CARD_TYPE_FSA:
#ifdef COVOND
									return ("MAT1000卡");
#else
									return ("FSA卡");
#endif
								case CARD_TYPE_DAT:
									return ("DAT1000卡");
								case CARD_TYPE_FE8:
									return ("FE8卡");
								case CARD_TYPE_ENCODE:
									return ("编码卡");
								case CARD_TYPE_FSA600:
#ifdef COVOND
									return ("DAT600卡");
#else
									return ("FSA600卡");
#endif
#ifdef __MTT__ // 201611
                                case CARD_TYPE_MTT:
                                    return ("MTU");
#else
								case CARD_TYPE_NTT:
								case CARD_TYPE_NTT_S: // 20150115
									return ("NTU");
#endif									
#if 1 // 201708
                                                            case CARD_TYPE_MTT_411:
                                                                return ("MTT411");
                                                            case CARD_TYPE_MTT_441:
                                                                return ("MTT441");
#endif
								default:	
									return ("卡");
							}
					case ATYPE_FXUP:
					case ATYPE_FXDOWN:
						switch(card_type)
						{
						case  CARD_TYPE_FSA600:
#ifdef COVOND
						return ("DAT600卡");
#else
						return ("FSA600卡");
						
#endif
#if 1 // 201708
                                                    case CARD_TYPE_MTT_411:
                                                        return ("MTT411");
                                                    case CARD_TYPE_MTT_441:
                                                        return ("MTT441");
#endif
						case CARD_TYPE_NTT:
						case CARD_TYPE_NTT_S: // 20150115
							return  ("NTT 卡");
						case  CARD_TYPE_EPN:
							return  ("EPON Card");
						}
					case ATYPE_RMTPOWERDOWN:
#ifdef COVOND
						return ("DAU100");
#else
						return ("ONU100");
#endif
					default:
						return("系统");
				}	
		}
		else
		{
				switch(alarm_type)
			 {
					case ATYPE_PSAON:
					case ATYPE_PSAOFF:
					case ATYPE_PSBON:
					case ATYPE_PSBOFF:
						return("Power");
					case ATYPE_FANON:
					case ATYPE_FANOFF:
						return("Fan");
					case ATYPE_CARDONLINE:
					case ATYPE_CARDOFFLINE:
					case ATYPE_CARDFAILURE:
					case ATYPE_CARDRESTART:
						switch (card_type)
						{
							case CARD_TYPE_CTRL:	
								return ("Control Card");
							case CARD_TYPE_FSA:
#ifdef COVOND
								return ("MAT1000 Card");
#else
								return ("FSA Card");
#endif
							case CARD_TYPE_DAT:
								return ("DAT1000 Card");
							case CARD_TYPE_FE8:
								return ("FE8 Card");
							case CARD_TYPE_ENCODE:
								return ("ENCode Card");
							case CARD_TYPE_NTT:
							case CARD_TYPE_NTT_S: // 20150115
								return ("NTT Card");
#if 1 // 201708
                                                            case CARD_TYPE_MTT_411:
                                                                return ("MTT411");
                                                            case CARD_TYPE_MTT_441:
                                                                return ("MTT441");
#endif
							case CARD_TYPE_MS4V1H:
								return ("MS4V1H Card");
							case CARD_TYPE_M4S:
								return ("M4S Card");
							case CARD_TYPE_M4H:
								return ("M4H Card");
							case CARD_TYPE_EPN:
								return ("EPON Card");
							case CARD_TYPE_FSA600:
#ifdef COVOND
								return ("MAT600 Card");
#else
								return ("FSA600 Card");
#endif
							default:	
								return ("Card");
						}
					case ATYPE_ONUONLINE:
					case ATYPE_ONUOFFLINE:
					case ATYPE_ONUSFPINSERTED:
					case ATYPE_ONUSFPREMOVED:
					case ATYPE_ONUPORTUP:
					case ATYPE_ONUPORTDOWN:
						if ( CARD_TYPE_FSA600 == card_type)
#ifdef COVOND
							return ("DAU100");
#else
							return ("ONU100");
#endif
						else if ( CARD_TYPE_EPN == card_type)
								return ("ONU");
						else{
#ifdef COVOND
							return ("MAU");
#else
							return ("ONU");
#endif
						}
					case ATYPE_ONETRINGSTANDBYFAILURE:
					case ATYPE_ONETRINGSTANDBYRESTORE:
					case ATYPE_ONETRINGFAILURE:
					case ATYPE_ONETRINGRESTORE:
					case ATYPE_ONETRINGBROKEN:
					case ATYPE_ONETLINKBROKEN:
					case ATYPE_ONETLINKRESTORE:
#ifdef COVOND
						return ("MAT1000 Card");
#else
						return ("FSA Card");
#endif
					case ATYPE_SFPINSERTED:
					case ATYPE_SFPREMOVED:
					case ATYPE_ONUSFPERROR:
						switch (card_type)
						{
							case CARD_TYPE_FSA:
								if(nodeid == 0)
#ifdef COVOND
								return("MAT1000 Card");
#else
								return("FSA Card");
#endif
								else
#ifdef COVOND
									return("MAU");
#else
									return("ONU");
#endif
							case CARD_TYPE_DAT:
								if(nodeid == 0)
								return("DAT1000 Card");
								else
									return("DAU");
							case CARD_TYPE_FSA600:
#ifdef COVOND
								return ("DAT600 Card");
#else
								return ("FSA600 Card");
#endif
							default:	
									return ("Card");	
						}
					case ATYPE_PORTUP:
					case ATYPE_PORTDOWAN:
							switch (card_type)
							{
								case CARD_TYPE_FSA:
#ifdef COVOND
									return ("MAT1000 Card");
#else
									return ("FSA Card");
#endif
								case CARD_TYPE_DAT:
									return ("DAT1000 Card");
								case CARD_TYPE_FE8:
									return ("FE8 Card");
								case CARD_TYPE_ENCODE:
									return ("ENCode Card");
								case CARD_TYPE_FSA600:
#ifdef COVOND
									return ("DAT600 Card");
#else
									return ("FSA600 Card");
#ifdef __MTT__ // 201611
                                case CARD_TYPE_MTT:
                                    return ("MTU");
#else
								case CARD_TYPE_NTT:
								case CARD_TYPE_NTT_S: // 20150115
									return  ("NTU");
#endif									
#endif
#if 1 // 201708
                                                            case CARD_TYPE_MTT_411:
                                                                return ("MTT411");
                                                            case CARD_TYPE_MTT_441:
                                                                return ("MTT441");
#endif
								default:	
									return ("Card");
							}
					case ATYPE_FXUP:
					case ATYPE_FXDOWN:
						switch(card_type)
						{
						case  CARD_TYPE_FSA600:
						
#ifdef COVOND
						return ("DAT600 Card");
#else
						return ("FSA600 Card");
#endif

#ifdef __MTT__ // 201611
                        case CARD_TYPE_MTT:
                            return ("MTU");
#else
						case  CARD_TYPE_NTT:
						case CARD_TYPE_NTT_S: // 20150115
							return  ("NTT Card");
#endif							
#if 1 // 201708
                                                    case CARD_TYPE_MTT_411:
                                                        return ("MTT411");
                                                    case CARD_TYPE_MTT_441:
                                                        return ("MTT441");
#endif
						case  CARD_TYPE_EPN:
							return  ("EPON Card");
						}
					case ATYPE_RMTPOWERDOWN:
#ifdef COVOND
						return ("DAT100");
#else
						return ("ONU100");
#endif
					default:
						return("System");
				}	
		}	
}

}
