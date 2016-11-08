#include "contiki.h"
#include "xprintf.h"
#include "basictype.h"
#include "sim900a.h"
#include "iodef.h"
#include "string.h"
#include "lib/ringbuf.h"
#include "sysprintf.h"
#include "gprsProtocol.h"
#include "hwgg.h"
#include "dev_info.h"
#include "app485.h"



PROCESS(sim900a_hard_init, "sim900a_hard_init");
PROCESS(sim900a_check_process, "sim900a_check");
PROCESS(sim900a_cfggprs_process, "sim900a_cfggprs");
PROCESS(sim900a_tcpudp_con_process, "sim900a_tcpcon");
PROCESS(sim900a_app_process, "sim900a_app");
PROCESS(sim900a_send_process, "sim900a_send");
PROCESS(gprs_resp_process, "sim900a_resp");

PROCESS(gprs_sms_phone_process, "sim900a_sms_phone");



//receive data from sim900a
process_event_t sim900_event_resp;
process_event_t	sim900_event_send_data;
process_event_t sim900_event_send_data_wait;
process_event_t	sim900_event_send_cmd;
process_event_t	sim900_event_heart;
process_event_t sim900_event_fire_warn;

process_event_t sim900_event_start_sms_phone;
process_event_t sim900_event_send_sms_phone;
process_event_t sim900_event_send_sms;
process_event_t sim900_event_send_sms_wait;

process_event_t sim900_event_fire_tran;


#define TIME_CHECK_GPRS_AT		100
#define TIME_CLOSE_GPRS_ECHO	200
#define TIME_CHECK_GPRS_SIM		200
static struct etimer et_gprs;
//static struct etimer et_gprs_status;
//static struct etimer et_gprs_heart_periodic;
static u_short uwSeq = 0;

static SIM900A_MSG sim900_rx;
static SIM900A_MSG sim900_tx;
static SIM900A_MSG sim900_tx_tran;
static SIM900A_MSG sim900_app;
//const u_char serverIp[]="119.29.155.148";
//const u_char serverPort[]="4567";
const u_char serverIp[]="119.29.224.28";
const u_char serverPort[]="4567";
//const u_char serverPort[]="8960";
//const u_char serverPort[]="4570";
//static u_char dataSrcFlag = SIM900A_DATA_NONE;
static enum GPRS_STATE gprsState = SIM900A_ERROR;
static u_char ubNetState = 0;

const u_char routermac[GPRS_F_MAC_LEN]={0x12,0x21,0x33,0x44};

static u_char ubCount = 0;

struct process *gprs_process = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
const u_char  *modetbl[2]={"TCP","UDP"};//连接模式


//发生火灾报警
//\u53d1\u751f\u706b\u707e\u62a5\u8b66
//const u_char warnMsg[] = {0x53,0xd1,0x75,0x1f,0x70,0x6b,0x70,0x7e,0x62,0xa5,0x8b,0x66};
const u_char warnMsg[] = "53d1751f706b707e62a58b66";


static u_char smsCenterPhone[32]={0x00};
const u_char smsCenterPrex[] = "089168";	//
const u_char smsDestPrex[] = "11000D9168";
const u_char smsDestCodeLen[] = "0008AA0C";  //data length fix 发生火灾报警 12 bytes




static volatile u_char ubWaitAckCount = 0;  //count no ack from sever, when this num > 6,reconnect server

u_char getNetState(void)
{
	return ubNetState;
}

void setNetState(u_char ubState)
{
	ubNetState = ubState;
}



/**
 *
 * @param presp pointer response data from gprs
 * @return none
 */
void sim_at_response(const char * presp)
{
	xprintf(presp);
}


/**
 *
 * @param pcTarget The pointer to target string
 * @param pcFindStr The pointer to the find str
 * @return if the  pcFindStr string  is a substring of the pcTarget string
 * return the addr the pcFindStr first position,else return null
 */
u_char* sim900a_check_cmd(const char *pcTarget, const char* pcFindStr)
{
	char *strx = NULL;
	strx = (char *)strstr(pcTarget,pcFindStr);
	return (u_char*)strx;
}



/**
 *
 * @param  cmd send num to gprs
 * @return
 * note this function need to check grps ack in app layer
 */
u_char sim900a_send_cmd_num(u_char cmd)
{
	uart4_send_char(cmd);
	return 0;
}

/**
 *
 * @param  cmd cmd str to gprs, not add '\r\n'
 * @return
 * note this function need to check grps ack in app layer
 */
u_char sim900a_send_cmd(const u_char *cmd)
{
	 u_char ubaBuf[64] = {0};
	xsprintf(ubaBuf, "%s\n", cmd);
	MEM_DUMP(9, "->cm", ubaBuf, strlen(ubaBuf));
	//xfprintf(uart4_send_char, "%s\n", cmd);
	uart4_send_bytes(ubaBuf, strlen(ubaBuf));
	return 0;
}

//136 -7888 -1212 {31 76 88 18 12}

void smsPnoneByteReverse(u_char *psmsNum, const u_char * pcsmsNum)
{
	int smsLen = strlen(pcsmsNum);
	int i = 0;

	if (smsLen < 10)
	{
		return;
	}

	for (i = 0; i < smsLen; i++)
	{
		if (i%2 == 0)
		{
			psmsNum[i] = pcsmsNum[i+1];
			psmsNum[i+1] = pcsmsNum[i];
		}
	}
	
	psmsNum[smsLen-1]= 'F';
	psmsNum[smsLen]= pcsmsNum[smsLen-1];

	XPRINTF((8, "SMS=%s\n", psmsNum));
}

/**
 *
 * @param  chr char
 * @return hex value of the chr if vilad,else return 0
 */
u_char sim900a_chr2hex(u_char chr)
{
	if(chr>='0'&&chr<='9')
		return chr-'0';
	if(chr>='A'&&chr<='F')
		return (chr-'A'+10);
	if(chr>='a'&&chr<='f')
		return (chr-'a'+10);
	return 0;
}


//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u_char sim900a_hex2chr(u_char hex)
{
	if(hex<=9)
		return hex+'0';
	if(hex>=10&&hex<=15)
		return (hex-10+'A');
	return '0';
}


void sim900a_tcpudp_connect(u_char *pbuf,u_char mode,u_char *ipaddr, u_char *port)
{
	xsprintf((char*)pbuf,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
	sim900a_send_cmd(pbuf);
}







//app layer
/******************************************************************/
PROCESS_THREAD(sim900a_hard_init, ev, data)
{
	PROCESS_BEGIN( );
	//gprs init
	//RST
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
	XPRINTF((12, "sim900a_hard_init\r\n"));
//	GPRS_SRST(0);
//	GPRS_STATUS(0);
//	GPRS_PWRKEY(0);
	GPRS_CON(1);
	etimer_set(&et_gprs, 5000);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_gprs));
	GPRS_CON(0);
	GPRS_EN(0);
	GPRS_ENOLD(0);
	etimer_set(&et_gprs, 5000);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_gprs));

	XPRINTF((12, "sim900a_hard_init2\r\n"));
	process_start(&sim900a_check_process, NULL);

	
//	GPRS_STATUS(1);
//	GPRS_PWRKEY(1);
	GPRS_EN(1);
	GPRS_ENOLD(1);
	XPRINTF((12, "sim900a_hard_init2\r\n"));
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,DISABLE);
	//process_start(&sim900a_check_process, NULL);

	PROCESS_END( );
}


void sim900a_send_cmd_wait_ack(const u_char *cmd, struct etimer *pet, clock_time_t waitTime)
{
	sim900a_send_cmd(cmd);
	if (waitTime > 0 && pet != NULL)
	{
		etimer_set(pet, waitTime);
	}
}


void sim900a_handler(process_event_t ev, process_data_t data)
{
	static u_char ubCheckCount = 0;
	static u_char ubSim900aState = SIM900A_CHECK_AT; //init sim900a state

	if (ev == PROCESS_EVENT_TIMER && data == &et_gprs)
	{
		//
		if (ubSim900aState == SIM900A_CHECK_AT)
		{
			sim900a_send_cmd_wait_ack("AT", &et_gprs,TIME_CHECK_GPRS_AT);
			ubCheckCount++;
			if (ubCheckCount == 10)//restart grps moduel
			{
				//process_exit(&sim900a_rev_process);
				//process_start(sim900a_process, NULL);
			}
		}
		//
		else if (ubSim900aState == SIM900A_CLOSE_ECHO)
		{
			sim900a_send_cmd_wait_ack("ATE0", &et_gprs,TIME_CLOSE_GPRS_ECHO);
		}

		//
		else if (ubSim900aState == SIM900A_CHECK_SIM)
		{

		}

	}

	//data
	if (ev == sim900_event_resp && data != NULL)
	{
		//开机启动后，发送AT指令同步波特率，同时检查模块
		if (ubSim900aState == SIM900A_CHECK_AT)
		{
			u_char *presp = NULL;
			presp = sim900a_check_cmd((const char*)data,"OK");

			if (presp == NULL)//continue
			{
				sim900a_send_cmd_wait_ack("AT", &et_gprs,TIME_CHECK_GPRS_AT);
			}
			else //gprs model normal
			{
				//关闭回显
				etimer_stop(&et_gprs);
				ubCheckCount = 0;
				ubSim900aState = SIM900A_CLOSE_ECHO;
				sim900a_send_cmd_wait_ack("ATE0", &et_gprs,TIME_CLOSE_GPRS_ECHO);
			}
		}

		//完成AT指令检查后，关闭回显
		else if (ubSim900aState == TIME_CLOSE_GPRS_ECHO)
		{
			u_char *presp = NULL;
			presp = sim900a_check_cmd((const char*)data,"OK");

			if (presp == NULL)//continue
			{
				sim900a_send_cmd_wait_ack("ATE0", &et_gprs,TIME_CLOSE_GPRS_ECHO);
			}
			else //关闭成功
			{
				//检查SIM卡是否在
				etimer_stop(&et_gprs);
				ubSim900aState = SIM900A_CHECK_SIM;
				sim900a_send_cmd_wait_ack("AT+CPIN?", &et_gprs,TIME_CHECK_GPRS_SIM);
			}
		}

		//检查SIM卡
		else if (ubSim900aState == TIME_CHECK_GPRS_SIM)
		{
			u_char *presp = NULL;
			presp = sim900a_check_cmd((const char*)data,"OK");

			if (presp == NULL)//continue
			{
				sim900a_send_cmd_wait_ack("AT+CPIN?", &et_gprs,TIME_CHECK_GPRS_SIM);
			}
			else //
			{
				//检查SIM卡是否在
				etimer_stop(&et_gprs);
				ubSim900aState = SIM900A_TCPUDP;
				sim900a_send_cmd_wait_ack("AT+CPIN?", &et_gprs,TIME_CLOSE_GPRS_ECHO);
			}
		}
	}
}



void sim900a_clear_rx(void)
{
	memset(&sim900_rx, 0, sizeof(SIM900A_MSG));
}

void sim900a_update_rx(const u_char *pcdata, u_short uwLen)
{
	sim900_rx.nLen = uwLen;
	memcpy(sim900_rx.ubamsg, pcdata, uwLen);
}


#define ATCMD_MAX_REPEAT_NUMS		5


static int check_gprs_moduel(u_char atcmdCount)
{
	if (atcmdCount == ATCMD_MAX_REPEAT_NUMS)
	{
		//process_exit(&sim900a_init_process);
		//process_start(&sim900a_rst_process, NULL);
		return 1;
	}
	return 0;
}


const char * gprs_check[]={
	"AT",
	"ATE0",
	"AT+CPIN?",
	"AT+COPS?"
};

const char * gprs_check_resp[]={
	"OK",
	"OK",
	"OK",
	"OK"
};

//common check grps and sim
PROCESS_THREAD(sim900a_check_process, ev, data)
{
	static u_char i = 0;
	static struct etimer et_check;
	u_char *presp = NULL;
	u_char *pcmd = NULL;

	PROCESS_BEGIN( );
	gprs_process = &sim900a_check_process;
	i = 0;
	XPRINTF((0, "sim900a_check_process\r\n"));

#if 1
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT");
		etimer_set(&et_check, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_check) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_check);
			MEM_DUMP(10, "AT", data, strlen(data));
			if (presp != NULL)
			{
				gprsState = SIM900A_AT_OK;
				break;
			}
		}
	}

	//close grps echo function
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("ATE0");
		etimer_set(&et_check, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_check) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_check);
			MEM_DUMP(10, "ATE0", data, strlen(data));
			if (presp != NULL)
				break;
		}
	}

	//查询SIM卡是否在位
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CPIN?");
		etimer_set(&et_check, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_check) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			pcmd = sim900a_check_cmd((const char*)data,"CPIN");
			etimer_stop(&et_check);
			MEM_DUMP(10, "CPIN", data, strlen(data));
			if (presp != NULL && pcmd != NULL)
			{
				gprsState = SIM900A_SIM_OK;
				break;
			}
		}
	}

	//查询运营商名字
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+COPS?");
		etimer_set(&et_check, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_check) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			pcmd = sim900a_check_cmd((const char*)data,"COPS");
			etimer_stop(&et_check);
			MEM_DUMP(10, "COPS", data, strlen(data));
			if (presp != NULL && pcmd != NULL)
				break;
		}
	}
	#else
	i = 0;
	etimer_set(&et_check, 5*1000); //5s
	sim900a_send_cmd(gprs_check[i]);
	while(1)
	{
		PROCESS_YIELD( );
		if (ev == PROCESS_EVENT_TIMER && data == &et_check)
		{
			etimer_set(&et_check, 5*1000); //5s
			sim900a_send_cmd(gprs_check[i]);
		}
		else if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,gprs_check_resp[i]);
			if (presp!=NULL) //next cmd
			{
				i++;
				if (i == sizeof(gprs_check)/sizeof(gprs_check[0]))
				{
					etimer_stop(&et_check);
					XPRINTF((12,"check out\n"));
					break;
				}
			}
			etimer_stop(&et_check);
			etimer_set(&et_check, 100);
		}
	}
	#endif
	gprsState = SIM900A_SIM_OK;
	process_start(&sim900a_cfggprs_process,NULL);
	process_exit(&sim900a_check_process );
	PROCESS_END();
}

//gprs model param cfg
/*
sim900a_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);	//关闭连接
sim900a_send_cmd("AT+CIPSHUT","SHUT OK",100);		//关闭移动场景
if(sim900a_send_cmd("AT+CGCLASS=\"B\"","OK",1000))return 1;				//设置GPRS移动台类别为B,支持包交换和数据交换
if(sim900a_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))return 2;//设置PDP上下文,互联网接协议,接入点等信息
if(sim900a_send_cmd("AT+CGATT=1","OK",500))return 3;					//附着GPRS业务
if(sim900a_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))return 4;	 	//设置为GPRS连接模式
if(sim900a_send_cmd("AT+CIPHEAD=1","OK",500))return 5;	 				//设置接收数据显示IP头(方便判断数据来源)
*/
//sim900a_cfggprs_process
const char * gprs_cfg[]={
	"AT+CIPCLOSE=1",						//关闭连接
	"AT+CIPSHUT=1",							//关闭移动场景
	"AT+CGCLASS=\"B\"",						//设置GPRS移动台类别为B,支持包交换和数据交换
	"AT+CGDCONT=1,\"IP\",\"CMNET\"",		//设置PDP上下文,互联网接协议,接入点等信息
	"AT+CGATT=1",							//附着GPRS业务
	"AT+CIPCSGP=1,\"CMNET\"",				//设置为GPRS连接模式
	"AT+CLPORT=\"TCP\",\"2000\""			//
};

const char * gprs_cfg_resp[]={
	"OK",
	"OK",
	"OK",
	"OK",
	"OK",
	"OK",
	"OK"
};

PROCESS_THREAD(sim900a_cfggprs_process, ev, data)
{
	static u_char i = 0;
	static struct etimer et_cfggprs;
	u_char *presp = NULL;
	PROCESS_BEGIN( );
	i = 0;
	gprs_process = &sim900a_cfggprs_process;


	//if (gprsState == SIM900A_TCPUDP_OK)
	#if 1
	{
		#if 0
		//关闭连接
		for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
		{
			sim900a_send_cmd("AT+CIPCLOSE=1");//"CLOSE OK"
			etimer_set(&et_cfggprs, 5*1000); //5s
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
			if (ev == sim900_event_resp && data != NULL)
			{
				presp = sim900a_check_cmd((const char*)data,"OK");
				etimer_stop(&et_cfggprs);
				MEM_DUMP(10, "LOSE", data, strlen(data));
				if (presp != NULL)
					break;
			}
		}

		//关闭移动场景
		for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
		{
			sim900a_send_cmd("AT+CIPSHUT=1");//"SHUT OK"
			etimer_set(&et_cfggprs, 5*1000); //5s
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
			if (ev == sim900_event_resp && data != NULL)
			{
				presp = sim900a_check_cmd((const char*)data,"OK");
				etimer_stop(&et_cfggprs);
				MEM_DUMP(10, "SHUT", data, strlen(data));
				if (presp != NULL)
				{
					gprsState = SIM900A_TCPUDP_CLOSE;
					break;
				}
			}
		}
		#endif
	}

	//设置GPRS移动台类别为B,支持包交换和数据交换
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CGCLASS=\"B\"");//"OK"
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "LASS", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}

	//设置PDP上下文,互联网接协议,接入点等信息
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"");//"OK"
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "CGDC", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}

	//附着GPRS业务
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CGATT=1");//"OK"
		etimer_set(&et_cfggprs, 20*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "CGAT", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}
//13798983973
//5989
	//设置为GPRS连接模式
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CIPCSGP=1,\"CMNET\"");//"OK"
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "CSGP", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}


	//设置接收数据显示IP头(方便判断数据来源)
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CIPHEAD=1");//"OK"
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "PHEAD", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}

	//设置短信发送参数  中英文短信的发送
	
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CMGF=0");//文本模式
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");

			MEM_DUMP(12, "CMGF", data, strlen(data));
			if (presp != NULL)
			{
				etimer_stop(&et_cfggprs);
				break;
			}
		}
	}
	/*
	//AT+CSMP=17,167,2,25
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CSMP=17,167,2,25");//文本模式参数
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "CSMP", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}
	//AT+CSCS="UCS2"
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd("AT+CSCS=\"UCS2\"");//
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"OK");
			etimer_stop(&et_cfggprs);
			MEM_DUMP(12, "CSMP", data, strlen(data));
			if (presp != NULL)
			{
				break;
			}
		}
	}
	*/
	//配置短信中心号
	//"AT+CPMS=\"SM\",\"SM\",\"SM\""

	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		u_char *pbuf = NULL;

		sim900a_send_cmd("AT+CSCA?");
		etimer_set(&et_cfggprs, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_cfggprs) || ev == sim900_event_resp);
		//MEM_DUMP(10, "CSCA", data, strlen(data));
		if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,"CSCA");

			MEM_DUMP(12, "CSCA", data, strlen(data));
			if (presp != NULL)
			{
				etimer_stop(&et_cfggprs);
				break;
			}
		}
	}
	#else
	etimer_set(&et_cfggprs, 5*1000); //5s
	sim900a_send_cmd(gprs_cfg[i]);
	while(1)
	{
		PROCESS_YIELD( );
		if (ev == PROCESS_EVENT_TIMER && data == &et_cfggprs)
		{
			etimer_set(&et_cfggprs, 5*1000); //5s
			sim900a_send_cmd(gprs_cfg[i]);
		}
		else if (ev == sim900_event_resp && data != NULL)
		{
			presp = sim900a_check_cmd((const char*)data,gprs_cfg_resp[i]);
			if (presp!=NULL) //next cmd
			{
				i++;
				if (i == sizeof(gprs_cfg)/sizeof(gprs_cfg[0]))
				{
					etimer_stop(&et_cfggprs);
					XPRINTF((10,"check out\n"));
					break;
				}
			}
			etimer_stop(&et_cfggprs);
			etimer_set(&et_cfggprs, 100);
		}
	}
	#endif

	process_start(&sim900a_tcpudp_con_process,NULL);
	process_exit(&sim900a_cfggprs_process);

	PROCESS_END( );
}

PROCESS_THREAD(sim900a_tcpudp_con_process, ev, data)
{
	static u_char i = 0;
	static u_char ubcount = 0;
	static struct etimer et_tcpudp;
	static u_char baBuf[128] = {0x00};
	u_char *presp = NULL;
    u_char *pcon_ok=NULL;
	u_char *pcon_true = NULL;
	NET_MODE *pnetMode;
	PROCESS_BEGIN( );
	gprs_process = &sim900a_tcpudp_con_process;

//	if (gprsState == SIM900A_TCPUDP_OK)
	{
		//关闭连接
		for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
		{
			sim900a_send_cmd("AT+CIPCLOSE=1");//"CLOSE OK"
			etimer_set(&et_tcpudp, 5*1000); //5s
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_tcpudp) || ev == sim900_event_resp);
			if (ev == sim900_event_resp && data != NULL)
			{
				presp = sim900a_check_cmd((const char*)data,"OK");
				etimer_stop(&et_tcpudp);
				MEM_DUMP(12, "LOSE", data, strlen(data));
				if (presp != NULL)
					break;
			}
		}

		//关闭移动场景
		for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
		{
			sim900a_send_cmd("AT+CIPSHUT=1");//"SHUT OK"
			etimer_set(&et_tcpudp, 5*1000); //5s
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_tcpudp) || ev == sim900_event_resp);
			if (ev == sim900_event_resp && data != NULL)
			{
				presp = sim900a_check_cmd((const char*)data,"OK");
				etimer_stop(&et_tcpudp);
				MEM_DUMP(12, "SHUT", data, strlen(data));
				if (presp != NULL)
				{
					gprsState = SIM900A_TCPUDP_CLOSE;
					break;
				}
			}
		}

		//query sms center num and save
		for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
		{
			u_char *pbuf = NULL;

			sim900a_send_cmd("AT+CSCA?");
			etimer_set(&et_tcpudp, 5*1000); //5s
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_tcpudp) || ev == sim900_event_resp);
			//MEM_DUMP(10, "CSCA", data, strlen(data));
			if (ev == sim900_event_resp && data != NULL)
			{
				presp = sim900a_check_cmd((const char*)data,"CSCA");

				MEM_DUMP(12, "CSCA", data, strlen(data));
				if (presp != NULL)
				{
					etimer_stop(&et_tcpudp);
					break;
				}
			}
		}	
	}
	//	sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
	//	if(sim900a_send_cmd(p,"OK",500))return;		//发起连接
	//AT+CIPSTART="TCP","180.120.52.222","8086"
	//connect serverip
	xsprintf(baBuf, "AT+CIPSTART=\"%s\",\"%s\",\"%s\"", modetbl[1],serverIp, serverPort);

	/*
	for (i = 0; i < ATCMD_MAX_REPEAT_NUMS; i++)
	{
		sim900a_send_cmd(baBuf);
		etimer_set(&et_tcpudp, 5*1000); //5s
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_tcpudp) || ev == sim900_event_resp);
		presp = sim900a_check_cmd((const char*)data,"OK");
		pcon_true = sim900a_check_cmd((const char*)data,"ALREADY");
		pcon_ok = sim900a_check_cmd((const char*)data,"CONNECT OK");
		MEM_DUMP(10, "IPST", data, strlen(data));
		if (pcon_ok != NULL)
		{
			gprsState = SIM900A_TCPUDP_CONNECT;
			break;
		}

		if (pcon_true != NULL)
		{
			gprsState = SIM900A_TCPUDP_CONNECT;
			break;
		}

	}
	*/

	sim900a_send_cmd(baBuf);
	ubcount = 0;
	etimer_set(&et_tcpudp, 10*1000); //5s
	while(1)
	{
		PROCESS_YIELD( );
		if (ev == PROCESS_EVENT_TIMER && data == &et_tcpudp)
		{
			sim900a_send_cmd(baBuf);
			etimer_set(&et_tcpudp, 10*1000); //5s
		}
		else if (ev == sim900_event_resp && data != NULL)
		{
			pcon_ok = sim900a_check_cmd((const char*)data,"CONNECT OK");
			//pcon_true = sim900a_check_cmd((const char*)data,"ALREADY CONNECT");
			if (pcon_ok != NULL)
			{
				gprsState = SIM900A_TCPUDP_CONNECT;
				pnetMode = (NET_MODE *)netModeGet( );
				if (pnetMode->netMode == NET_CONNECT_NONE)
				{
					netModeSet(NET_CONNECT_SIM900);
					XPRINTF((10, "-----------------------NETSETMODE\n"));
				}
				setNetState(NET_CONNECT_SIM900);
				gprs_process = &sim900a_app_process;
				etimer_stop(&et_tcpudp);
				process_post(&sim900a_app_process, sim900_event_heart, NULL);
				MEM_DUMP(12, "IPST", data, strlen(data));
				break;
			}
		}
		if (ubcount == 10)
		{
			ubcount= 0;
			process_start(&sim900a_hard_init, NULL);
			gprs_process = &sim900a_hard_init;
			process_exit(&sim900a_tcpudp_con_process);
			XPRINTF((10, "connect failed\r\n"));
		}
		ubcount ++;
	}
	//gprs_process = &sim900a_app_process;
	XPRINTF((12, "sim900a_tcpudp_con_process exit\r\n"));
	process_exit(&sim900a_tcpudp_con_process);
	PROCESS_END( );
}









void gprsSendFireMacSync(u_char macSync, u_char uwSeq)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	int nFramL = -1;
	SIM900A_MSG *ptxMsg = &sim900_tx;

	int nFrameL = -1;
	
	if (macSync)
	{
		return;
	}
	else
	{
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_REQUST_SYNC, uwSeq, paddrInfo->ubaNodeAddr, NULL, 0);
		

		if (nFramL > 0)
		{
			ptxMsg->nLen = nFramL;
			
			nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)ptxMsg->ubamsg,ptxMsg->nLen);
			sim900_tx_tran.nLen = nFrameL;
			//process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
			process_post(&sim900a_send_process, sim900_event_send_data,&sim900_tx_tran);
		
			MEM_DUMP(7, "SYNC", ptxMsg->ubamsg, ptxMsg->nLen);
		}		
	}
}



typedef struct {
 u_char macSync;
 u_short sendSeq;
}SEQ;

static void ctMacSyncCallback(void *ptr)
{
	SEQ * pSeq = (SEQ *)ptr;
	gprsSendFireMacSync(pSeq->macSync, pSeq->sendSeq);
}

static void ctSyncAckCallback(void *ptr)
{
	u_short seq = *(u_short *)ptr;
	SIM900A_MSG *ptxMsg = &sim900_tx;
	NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	int nFramL = -1;
	int nFrameL = -1;
	nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_ACK, seq, paddrInfo->ubaNodeAddr, NULL, 0);
	if (nFramL > 0 && gprsState == SIM900A_TCPUDP_CONNECT)
	{
		ptxMsg->nLen = nFramL;
		XPRINTF((8, "S YNC ACK\r\n"));
		
		nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)ptxMsg->ubamsg,ptxMsg->nLen);
		sim900_tx_tran.nLen = nFrameL;
		process_post(&sim900a_send_process, sim900_event_send_data,&sim900_tx_tran);
		//process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
	}	
}


#define GPRS_ACK_WAIT_TIME	1*1000

bool checkPhoneNum(const GPRS_WARN_PHONE *pcPhone)
{
	int i = 0;
	const u_char *pphone = (const u_char *)pcPhone;
	for (i = 0; i < sizeof(*pcPhone); i++)
	{
		if (pphone[i] != 0x00)
		{
			return true;
		}
	}
	return false;
}

void gprsProtocolRxProcess(const u_char *pcFrame, u_short uwSendSeq , struct etimer *petwait)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	const GPRS_PROTOCOL * pGprs = (const GPRS_PROTOCOL *)pcFrame;
	u_short uwSeq = pGprs->ubSeqL | (pGprs->ubSeqH << 8);
	u_short uwLen = pGprs->ubDataLenL | (pGprs->ubDataLenH<<8);
	static GPRS_WARN_PHONE stWarnPhone;
	static u_char macSync = 0;
	static struct ctimer ctimerMacSync;
	static struct ctimer ctimerSyncAck;
	static u_short uwSeqSyncAck = 0;
	static SEQ stSeq;
	
	ubWaitAckCount = 0;
	
	if (pGprs->ubCmd == GPRS_F_CMD_ACK)
	{
		XPRINTF((8 , "uwSendSeq = %04x  uwSeq = %04x\n", uwSendSeq, uwSeq ));

		if (uwSendSeq == uwSeq)
		{
			etimer_stop(petwait);
			stSeq.macSync = macSync;
			stSeq.sendSeq = uwSeq;
			//gprsSendFireMacSync(macSync, uwSeq);
			ctimer_set(&ctimerMacSync, GPRS_ACK_WAIT_TIME, ctMacSyncCallback, &stSeq);
		}
	}
	else if (pGprs->ubCmd == GPRS_F_CMD_DATA_SYNC)
	{
		SIM900A_MSG *ptxMsg = &sim900_tx;
		static FIRE_NODE_INFO stFireNode;
		const FIRE_NODE_INFO *pFireNodeInfo;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;

		//now we sync fire mac addr
		if (macSync == 0)
		{
			macSync = 1;
		}
		ubCount = 0;
		memset(&stFireNode, 0, sizeof(FIRE_NODE_INFO));

		if (uwLen > GPRS_F_MAC_LEN)
		{
			//memcpy()
			pFireNodeInfo = (const FIRE_NODE_INFO *)pGprs->ubaData;
			if (pFireNodeInfo->node_num > 0)
			{
				stFireNode.node_num = pFireNodeInfo->node_num;
				memcpy(stFireNode.nodeArray, pFireNodeInfo->nodeArray, stFireNode.node_num*4);
				extgdbdevSetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO, 0, (const void *)&stFireNode, sizeof(FIRE_NODE_INFO));
			}

			/*
			nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_ACK, uwSeq, paddrInfo->ubaNodeAddr, NULL, 0);
			if (nFramL > 0 && gprsState == SIM900A_TCPUDP_CONNECT)
			{
				ptxMsg->nLen = nFramL;
				process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
			}
			*/
			
		}
		uwSeqSyncAck  = uwSeq;
		ctimer_set(&ctimerSyncAck, GPRS_ACK_WAIT_TIME, ctSyncAckCallback, &uwSeqSyncAck);
	}
	else if (pGprs->ubCmd == GPRS_F_CMD_WARN_ACK)
	{

		if (uwSendSeq == uwSeq)
		{
			etimer_stop(petwait);
		}
		//clean warn phohe data
		memset(&stWarnPhone, 0, sizeof(GPRS_WARN_PHONE));
		if (uwLen-GPRS_F_MAC_LEN == 40)
		{
			memcpy(&stWarnPhone, pGprs->ubaData, 40);
			if (checkPhoneNum((const GPRS_WARN_PHONE*) &stWarnPhone))
			{
				process_post(&sim900a_app_process, sim900_event_start_sms_phone, &stWarnPhone);
			}
		}
	}
}



static bool nodeIsNotInList(const u_char * pcMac)
{
	NODE_INFO *pnode = NULL;
	
	for(pnode = (NODE_INFO *)endNodeListHead(); pnode != NULL; pnode = (NODE_INFO *)endNodeListNext(pnode))
	{
		if (mem_cmp(pnode->ubaHWGGMacAddr, pcMac, HWGG_NODE_MAC_LEN) == 0)
		{
			return false;
		}
	}

	return true;
}


void fillNotNetNodeInfo(FIRE_NODE_INFO *pFireInfo)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	//const FIRE_NODE_INFO *pFireNode = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);	
	int i = 0;
	NODE_INFO *pnode = NULL;
	int j = 0; 


	//clear fire node info
	if (pFireInfo != NULL)
	{
		memset(pFireInfo, 0, sizeof(FIRE_NODE_INFO));
	}

	if (pfireNodeInfo->node_num == 0)
	{
		return;
	}

	for(pnode = (NODE_INFO *)endNodeListHead(); pnode != NULL; pnode = (NODE_INFO *)endNodeListNext(pnode))
	{
		if (pnode->nodeNetState == HWGG_NODE_OUT_NET)
		{
			memcpy(pFireInfo->nodeArray[i++].ubaNodeAddr, pnode->ubaHWGGMacAddr, HWGG_NODE_MAC_LEN);
			XPRINTF((10, "i1 = %d\n", i));
			MEM_DUMP(10, "OUT1", pnode->ubaHWGGMacAddr, HWGG_NODE_MAC_LEN);
		}
	}

	/*node is in flash but not in ram list, is not in net*/
	for (j = 0; j < pfireNodeInfo->node_num; j++)
	{
		if ( nodeIsNotInList((const u_char*)pfireNodeInfo->nodeArray[j].ubaNodeAddr))
		{
			memcpy(pFireInfo->nodeArray[i++].ubaNodeAddr, pfireNodeInfo->nodeArray[j].ubaNodeAddr, HWGG_NODE_MAC_LEN);
			XPRINTF((10, "i = %d\n", i));
			MEM_DUMP(10, "OUT", pfireNodeInfo->nodeArray[j].ubaNodeAddr, HWGG_NODE_MAC_LEN);
		}
	}

	pFireInfo->node_num = i;
}




void sim900a_reconnect(struct etimer *petHeart)
{
	//gprs_process = &sim900a_check_process;
	gprs_process = &sim900a_hard_init;
	gprsState = SIM900A_TCPUDP_CLOSE_T;
	setNetState(NET_CONNECT_NONE);
	netModeSet(NET_CONNECT_NONE);
	//sim900a_send_cmd("AT+CPOWD=1");
	process_start(&sim900a_hard_init, NULL);
	XPRINTF((8, "ERROR\n"));
	etimer_stop(petHeart);
}




void sim900a_app_handler(process_event_t ev, process_data_t data)
{
	static u_char ubGprsRespType = SIM900A_RESP_STATUS;
	static u_char ubaFireWarnBuf[32] = {0x00};
	static u_char ubSendCmd;
	static u_char ubSendState = SIM900A_SEND_NONE;
	static struct etimer et_heart;
	static struct etimer et_wait_ack;
	static FIRE_NODE_INFO stFireNodeInfo;
	static u_short uwCurrentSeq = 0;
	
	
	//XPRINTF((10, "grps app\r\n"));

	//revceive data process
	if (ev == sim900_event_resp && data != NULL)
	{
		//tcp udp closed, connect again
		if(sim900a_check_cmd((const char*)data,"CLOSED"))
		{
			sim900a_reconnect(&et_heart);
			XPRINTF((10, "closed rec\n"));
		}
		//send error, connect again
		else if (sim900a_check_cmd((const char*)data,"ERROR"))
		{
			sim900a_reconnect(&et_heart);
			XPRINTF((10, "error rec\n"));
		}
		//send error, connect again
		else if (sim900a_check_cmd(((const char*)data),"SEND FAIL"))
		{
			sim900a_reconnect(&et_heart);
			XPRINTF((10, "send fail rec\n"));
		}
		//have call. connect again
		else if (sim900a_check_cmd(((const char*)data),"RING"))
		{
			sim900a_send_cmd("ATH");
			sim900a_reconnect(&et_heart);
			XPRINTF((10, "RING rec\n"));
		}
		//> send data start flag
		else
		{
			
			const u_char *pHead = NULL;
			SIM900A_MSG *pMsg = (SIM900A_MSG *)((u_char *)data - 4);
			pHead = (const u_char *)gprsProtocolFindHead((const u_char *)data, pMsg->nLen);
			if (pHead != NULL)
			{
				if (gprsProtocolCheck(pHead))
				{
					const GPRS_PROTOCOL *pGprs = (const GPRS_PROTOCOL *)pHead;
					u_short uwLen = (pGprs->ubDataLenL | (pGprs->ubDataLenH<<8)) + 10;
					memset(&sim900_app, 0, sizeof(SIM900A_MSG));
					sim900_app.nLen = uwLen;
					memcpy(sim900_app.ubamsg, pHead, uwLen);
					MEM_DUMP(7, "<-Rx", sim900_app.ubamsg, uwLen);
					gprsProtocolRxProcess((const u_char *) sim900_app.ubamsg,uwCurrentSeq, &et_wait_ack);
				}
			}
			
		}	
		
		if (sim900a_check_cmd((const char*)data,"\r\n>"))
		{
			const u_char sendFlag[] = "\r\n>";
			process_post(&sim900a_send_process, sim900_event_send_data_wait, (void *)sendFlag);
		}

		/*
		if (sim900a_check_cmd((const char*)data,"+IPD"))
		{
			u_char *pDataStar = NULL;
			u_short ubDataLen = 0;
			u_char ubassicNum[8] = {0x00};
			u_char *pdata = sim900a_check_cmd((const char*)data,"+IPD,");
			u_char *pdataEnd = sim900a_check_cmd((const char*)data,":");
			const u_char *pHead = NULL;

			SIM900A_MSG *pMsg = (SIM900A_MSG *)((u_char *)data - 4);
			MEM_DUMP(10, "RMSG", pMsg->ubamsg, pMsg->nLen);


			#if 0
			pDataStar = pdata + strlen("+IPD,");
			memcpy(ubassicNum, pDataStar, pdataEnd-pDataStar);
			//MEM_DUMP(10, "LEN", ubassicNum, pdataEnd-pDataStar);
			ubDataLen = Asc2Int((const u_char *)ubassicNum);
			XPRINTF((12, "len = %d\r\n", ubDataLen));
			memset(&sim900_app, 0, sizeof(SIM900A_MSG));
			sim900_app.nLen = ubDataLen;
			memcpy(sim900_app.ubamsg, pdataEnd+1, ubDataLen);
			//MEM_DUMP(7, "<-rx", sim900_app.ubamsg, ubDataLen);
			if (gprsProtocolCheck((const u_char *) sim900_app.ubamsg))
			{
				MEM_DUMP(7, "<-rx", sim900_app.ubamsg, ubDataLen);
				gprsProtocolRxProcess((const u_char *) sim900_app.ubamsg,uwCurrentSeq, &et_wait_ack);
			}
			#else
			pHead = (const u_char *)gprsProtocolFindHead((const u_char *)data, pMsg->nLen);
			if (pHead != NULL)
			{
				if (gprsProtocolCheck(pHead))
				{
					const GPRS_PROTOCOL *pGprs = (const GPRS_PROTOCOL *)pHead;
					u_short uwLen = (pGprs->ubDataLenL | (pGprs->ubDataLenH<<8)) + 10;
					memset(&sim900_app, 0, sizeof(SIM900A_MSG));
					sim900_app.nLen = uwLen;
					memcpy(sim900_app.ubamsg, pHead, uwLen);
					MEM_DUMP(7, "<-Rx", sim900_app.ubamsg, uwLen);
					gprsProtocolRxProcess((const u_char *) sim900_app.ubamsg,uwCurrentSeq, &et_wait_ack);
				}
			}			
			#endif
		}
		*/

		//finish send data
		//if (sim900a_check_cmd((const char*)data,"SEND OK") || sim900a_check_cmd((const char*)data,"OK"))
		if (sim900a_check_cmd((const char*)data,"OK"))
		{
			process_post(&sim900a_send_process, sim900_event_send_data_wait, data);
			ubSendState = SIM900A_SEND_FINISH;
		}
	}

	else if (ev == sim900_event_heart)
	{
		SIM900A_MSG *ptxMsg = &sim900_tx;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;
		//XPRINTF((10, "heart1"));
		fillNotNetNodeInfo(&stFireNodeInfo);
		if (stFireNodeInfo.node_num > 0 )
		{

			nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_HEART, uwSeq, paddrInfo->ubaNodeAddr, (const u_char *)&stFireNodeInfo, stFireNodeInfo.node_num*4+2);
		}
		else
		{
			nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_HEART, uwSeq, paddrInfo->ubaNodeAddr, NULL, 0);
		}
		if (nFramL > 0 && gprsState == SIM900A_TCPUDP_CONNECT)
		{
			int nFrameL = -1;
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwSeq;
			ubSendCmd = GPRS_F_CMD_HEART;
			//process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
			MEM_DUMP(6,"SRC",ptxMsg->ubamsg, ptxMsg->nLen);
			nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)ptxMsg->ubamsg,ptxMsg->nLen);
			sim900_tx_tran.nLen = nFrameL;
			process_post(&sim900a_send_process, sim900_event_send_data,&sim900_tx_tran);
			
			etimer_set(&et_wait_ack, 10*1000);
			ubSendState = SIM900A_SEND_START;
		}
		//1 send heart frame
		//2 reset etimer
		ubCount++;
		etimer_set(&et_heart, 30*1000);
	}

	//send fire warn data to pc
	else if (ev == sim900_event_fire_warn && data != NULL)
	{
		SIM900A_MSG *ptxMsg = &sim900_tx;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		u_char ubaWarn[5] = 0;
		int nFramL = -1;
		const FIRE_NODE * pFireNode = (const FIRE_NODE *)data;
		if (((u_long)ubaFireWarnBuf)!=((u_long)data))
		{
			MEM_DUMP(10, "warn", data, pFireNode->ubLen);
			memcpy(ubaFireWarnBuf, data, pFireNode->ubLen);
		}
		memcpy(ubaWarn, pFireNode->ubaSrcMac, HWGG_HEAD_END_CRC_LEN);
		ubaWarn[4] = pFireNode->ubCmd;
		uwSeq++;
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_WARN, uwSeq, paddrInfo->ubaNodeAddr, ubaWarn, 5);
		if (nFramL > 0 && gprsState == SIM900A_TCPUDP_CONNECT)
		{
			int nFrameL = -1;
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwSeq;
			ubSendCmd = GPRS_F_CMD_WARN;
			
			nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)ptxMsg->ubamsg,ptxMsg->nLen);
			sim900_tx_tran.nLen = nFrameL;
			process_post(&sim900a_send_process, sim900_event_send_data,&sim900_tx_tran);
			
			//process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
			etimer_set(&et_wait_ack, 10*1000);
			ubSendState = SIM900A_SEND_START;
		}
		//ALARM_LED(0);
		FAULT_LED(0);
		BUZZER(0);
		swEnable( );
	}

	else if (ev == sim900_event_fire_tran && data != NULL)
	{
		SIM900A_MSG *ptxMsg = &sim900_tx;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;
		const APP_485_DATA *p485 = (const APP_485_DATA *)data;
		MEM_DUMP(7, "TRAN", p485->ubaData, p485->ubLen);
		uwSeq++;
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_TRAN, uwSeq, paddrInfo->ubaNodeAddr, (const u_char *)p485->ubaData, p485->ubLen);
		
		if (nFramL > 0 && gprsState == SIM900A_TCPUDP_CONNECT)
		{
			int nFrameL = -1;
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwSeq;
			ubSendCmd = GPRS_F_CMD_TRAN;

			nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)ptxMsg->ubamsg,ptxMsg->nLen);
			sim900_tx_tran.nLen = nFrameL;
			process_post(&sim900a_send_process, sim900_event_send_data,&sim900_tx_tran);
			
			//process_post(&sim900a_send_process, sim900_event_send_data,ptxMsg);
			etimer_set(&et_wait_ack, 10*1000);
			ubSendState = SIM900A_SEND_START;
		}
	}

	else if (ev == sim900_event_start_sms_phone && data != NULL)
	{
		etimer_stop(&et_heart);
		gprs_process = &gprs_sms_phone_process;
		gprsState = SIM900A_TCPUDP_CLOSE_T;
		netModeSet(NET_CONNECT_NONE);
		if (getNetState() != NET_CONNECT_ETH)
		{
			setNetState(NET_CONNECT_NONE);
		}
		process_post(&gprs_sms_phone_process, sim900_event_send_sms_phone, data);
	}

	else if (ev == PROCESS_EVENT_TIMER && data == &et_heart)
	{
		NET_MODE *pnetMode = (NET_MODE *)netModeGet( );
		process_post(&sim900a_app_process, sim900_event_heart, NULL);
		uwSeq++;
		if (pnetMode->netMode == NET_CONNECT_NONE)
		{
			
			XPRINTF((10, "et_heart check\n"));
			gprs_process = &sim900a_hard_init;
			gprsState = SIM900A_TCPUDP_CLOSE_T;
			setNetState(NET_CONNECT_NONE);
			//sim900a_send_cmd("AT+CPOWD=1");
			process_start(&sim900a_hard_init, NULL);
		}
	}

	else if (ev == PROCESS_EVENT_TIMER && data == &et_wait_ack)
	{
		XPRINTF((12, "ack time out\n"));
		//ubCount++;
	}

}

//sim900a_app_process
PROCESS_THREAD(sim900a_app_process, ev, data)
{
	PROCESS_BEGIN( );
	//sim900a_send_cmd_wait_ack("AT+CIPSTATUS",&et_gprs_status,500);
	while(1)
	{
		PROCESS_YIELD( );
		sim900a_app_handler(ev, data);
	}
	PROCESS_END( );
}





void gprsSmsPhoneHandler(process_event_t ev, process_data_t data)
{
	static struct etimer et_wait_close;
	static u_char ubCloseFlag = 0;
	static GPRS_WARN_PHONE stPhone;

	if (ev == sim900_event_send_sms_phone && data != NULL)
	{
		XPRINTF((12, "SEND SMS AND PHONE\r\n"));
		//clear stphone
		memset(&stPhone, 0, sizeof(GPRS_WARN_PHONE));
		stPhone = *(GPRS_WARN_PHONE*)data;
		MEM_DUMP(12, "PHON", &stPhone, sizeof(GPRS_WARN_PHONE));
		sim900a_send_cmd("AT+CIPCLOSE=1");
		sim900a_send_cmd("AT+CIPSHUT=1");
		etimer_set(&et_wait_close, 5000);
	}
	else if (ev == sim900_event_resp && data != NULL)
	{
		//make sure tcp udp closed
		if(sim900a_check_cmd((const char*)data,"CLOSE OK"))
		{
			ubCloseFlag = 1;
			XPRINTF((12, "gpr1 is close\r\n"));
			etimer_stop(&et_wait_close);
			process_post(&sim900a_send_process, sim900_event_send_sms, &stPhone);
		}
		else if (sim900a_check_cmd((const char*)data,">"))
		{
			process_post(&sim900a_send_process, sim900_event_send_sms_wait, data);
		}
		else if (sim900a_check_cmd((const char*)data,"+CMGS"))
		{
			XPRINTF((12, "SMSOK\n"));
			process_post(&sim900a_send_process, sim900_event_send_sms_wait, data);
		}
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_wait_close)
	{
		if (ubCloseFlag == 0)
		{
			sim900a_send_cmd("AT+CIPCLOSE=1");
			sim900a_send_cmd("AT+CIPSHUT=1");
		}
		process_post(&sim900a_send_process, sim900_event_send_sms, &stPhone);
	}
}

PROCESS_THREAD(gprs_sms_phone_process, ev, data)
{

	PROCESS_BEGIN( );
	//关闭连接
	while (1)
	{
		PROCESS_YIELD( );
		gprsSmsPhoneHandler(ev, data);
	}
	PROCESS_END( );
}


int asiicToUTF8(u_char *pUTF8, const u_char *pasiic)
{
	u_char ubasiicLen = strlen(pasiic);
	u_char utf8Len = 0;
	int i = 0;
	XPRINTF((10, "asiic len = %d\n", ubasiicLen));
	for (i = 0; i < ubasiicLen; i++)
	{
		pUTF8[2*i] = 0x00;
		pUTF8[2*i+1] = pasiic[i];
	}
	utf8Len = ubasiicLen<<1;
	MEM_DUMP(8, "UTF8", pUTF8, utf8Len);
	return utf8Len;
}


#define GPRS_TIME_WAIT_SMS		(20*1000)
#define GPRS_TIME_WAIT_PHONE	(50*1000)
#define GPRS_TIME_WAIT_SEND		(5*1000)
#define GPRS_TIME_WAIT_BACK_TCP	(140*1000)

#define GPRS_NO_ACK_MAX		100

void sim900a_send_handler(process_event_t ev, process_data_t data)
{
	static const SIM900A_MSG *psendMsg = NULL;
	volatile static u_char ubPhoneIndex = 0;
	static u_char ubPhone = 0;
	static u_char ubUTF8Buf[32] = {0x00};
	static u_char smsDestN[16] ={0x00};
	static u_char ubasiicBuf[64] = {0x00};
	static u_char ubcmdBuf[128] = {0x00};
	static u_char sendState = SIM900A_SEND_NONE;
	static struct etimer et_send;
	static struct etimer et_wait_sms_ok;
	static struct etimer et_phone;
	static struct etimer et_back_udp;
	
	//static u_char sendLockFlag = 0;
	
	static u_char ubSendFailed = 0;
	static GPRS_WARN_PHONE stPhone;
	static u_char ubmp3buf[16] = {0x00};
	int nmp3L = 0;

	u_char ubLen;
	int nLen = 0;

	if (ev == sim900_event_send_data && data != NULL)
	{
		
		NET_MODE *pnetMode = (NET_MODE *)netModeGet( );
		psendMsg = (const SIM900A_MSG *)data;
		
		XPRINTF((8, "START SEND gprs\n"));
		MEM_DUMP(8, "TXDA", psendMsg->ubamsg, psendMsg->nLen);		
		if (pnetMode->netMode == NET_CONNECT_SIM900)
		{
			sim900a_send_cmd("AT+CIPSEND");
			etimer_set(&et_send, GPRS_TIME_WAIT_SEND);
		}
	}
	else if (ev == sim900_event_send_data_wait && data != NULL)
	{
		if (sim900a_check_cmd((const char*)data,">"))
		{
			if (psendMsg != NULL)
			{
				int nDeL = 0;
				MEM_DUMP(6,"->gp", psendMsg->ubamsg, psendMsg->nLen);
				//while(sendLockFlag);
				//sendLockFlag = 1;
				//nFrameL = gprsCodeGetOut0xla(sim900_tx_tran.ubamsg, (const u_char*)psendMsg->ubamsg,psendMsg->nLen);
				uart4_send_bytes(psendMsg->ubamsg, psendMsg->nLen);
				//uart4_send_bytes(sim900_tx_tran.ubamsg, nFrameL);
				uart4_send_char(0x1a);
				//MEM_DUMP(6,"->EA", sim900_tx_tran.ubamsg, nFrameL);
				nDeL = gprsDecodeFrame(sim900_tx.ubamsg, (const u_char*)psendMsg->ubamsg, psendMsg->nLen);
				MEM_DUMP(6,"->1A", sim900_tx.ubamsg, nDeL);
				sendState = SIM900A_SEND_START;
			}
		}
		//if (sim900a_check_cmd((const char*)data,"SEND OK"))
		if (sim900a_check_cmd((const char*)data,"OK"))
		{
			XPRINTF((8, "SEND OK\n"));
			etimer_stop(&et_send);
			psendMsg = NULL;
			sendState = SIM900A_SEND_FINISH;
			ubSendFailed = 0;

			//sendLockFlag = 0;
			//
			if (ubWaitAckCount >= GPRS_NO_ACK_MAX)
			{
				gprs_process = &sim900a_hard_init;
				gprsState = SIM900A_TCPUDP_CLOSE_T;
				setNetState(NET_CONNECT_NONE);
				netModeSet(NET_CONNECT_NONE);
				//sim900a_send_cmd("AT+CPOWD=1");
				process_start(&sim900a_hard_init, NULL);	
				XPRINTF((8, "sim900a reconnect agian\n"));
				ubWaitAckCount = 0;
			}
			ubWaitAckCount++;
		}
	}
	//send sms
	else if (ev == sim900_event_send_sms && data != NULL)
	{
		if ((u_long)&stPhone != (u_long)data)
		{
			stPhone = *(GPRS_WARN_PHONE*)data;
		}
		ubPhoneIndex = 0;
		ubPhone = 0;
			
		//MEM_DUMP(10, "SMSH", &stPhone, sizeof(GPRS_WARN_PHONE));
		memset(ubUTF8Buf, 0, sizeof(ubUTF8Buf));
		memset(ubasiicBuf, 0, sizeof(ubUTF8Buf));
		memset(ubcmdBuf, 0, sizeof(ubcmdBuf));
		memset(smsDestN, 0, sizeof(smsDestN));

		#if 0
		ubLen = asiicToUTF8(ubUTF8Buf, stPhone.ubaPhoneA);
		nLen = hex2ascii((const uint8 *)ubUTF8Buf, ubasiicBuf, ubLen);
		XPRINTF((10, "asic nlen =%d\n", strlen(ubasiicBuf)));
		xsprintf(ubcmdBuf,"AT+CMGS=\"%s\"",ubasiicBuf);
		sim900a_send_cmd(ubcmdBuf);
		#else
		smsPnoneByteReverse(ubUTF8Buf, (const u_char*)smsCenterPhone);
		smsPnoneByteReverse(smsDestN, (const u_char *)stPhone.ubaPhoneA);
		xsprintf(ubcmdBuf,"%s%s%s%s%s%s", smsCenterPrex,ubUTF8Buf, smsDestPrex, smsDestN, smsDestCodeLen,warnMsg);
		XPRINTF((8, "warnsms=%s\n", ubcmdBuf));
		sim900a_send_cmd("AT+CMGS=27");
		#endif
		
		etimer_set(&et_wait_sms_ok, GPRS_TIME_WAIT_SMS);
		etimer_set(&et_back_udp, GPRS_TIME_WAIT_BACK_TCP);//set time come back to udp or tcp mode
		ubPhoneIndex++;

		nmp3L = mp3fillCmdOneByteParam(ubmp3buf, 0x33, 0x02);
		MEM_DUMP(10, "M0->",ubmp3buf, nmp3L);
		uart5_send_bytes(ubmp3buf, nmp3L);
	}
	else if (ev == sim900_event_send_sms_wait && data != NULL)
	{
		//start send msg
		if (sim900a_check_cmd((const char*)data,">"))
		{
			//uart4_send_bytes(warnMsg, strlen(warnMsg));
			uart4_send_bytes(ubcmdBuf, strlen(ubcmdBuf));
			uart4_send_char(0x1a);
			sendState = SIM900A_SEND_START;
		}
		//send ok
		else if (sim900a_check_cmd((const char*)data,"+CMGS"))
		{
			if (ubPhoneIndex == 1)
			{
				XPRINTF((10, "sms21-------------------------\n"));
				ubPhoneIndex++;	
				etimer_set(&et_wait_sms_ok, GPRS_TIME_WAIT_SMS);

				memset(ubUTF8Buf, 0, sizeof(ubUTF8Buf));
				memset(ubasiicBuf, 0, sizeof(ubUTF8Buf));
				memset(ubcmdBuf, 0, sizeof(ubcmdBuf));
				memset(smsDestN, 0, sizeof(smsDestN));
				
				#if 0
				ubLen = asiicToUTF8(ubUTF8Buf, stPhone.ubaPhoneB);
				nLen = hex2ascii((const uint8 *)ubUTF8Buf, ubasiicBuf, ubLen);
				XPRINTF((10, "asic nlen =%d\n", strlen(ubasiicBuf)));
				xsprintf(ubcmdBuf,"AT+CMGS=\"%s\"",ubasiicBuf);
				sim900a_send_cmd(ubcmdBuf);
				#else
				smsPnoneByteReverse(ubUTF8Buf, (const u_char*)smsCenterPhone);
				smsPnoneByteReverse(smsDestN, (const u_char *)stPhone.ubaPhoneB);
				xsprintf(ubcmdBuf,"%s%s%s%s%s%s", smsCenterPrex,ubUTF8Buf, smsDestPrex, smsDestN, smsDestCodeLen,warnMsg);
				XPRINTF((8, "warnsms=%s\n", ubcmdBuf));
				sim900a_send_cmd("AT+CMGS=27");	
				#endif
			}
			else
			{
				XPRINTF((10, "start1 phone\n"));
				XPRINTF((10, "pono11111-------------------------\n"));
				ubPhone++;
				xsprintf(ubcmdBuf,"ATD%s;",stPhone.ubaPhoneA);
				etimer_stop(&et_wait_sms_ok);
				sim900a_send_cmd(ubcmdBuf);
				etimer_set(&et_phone, GPRS_TIME_WAIT_PHONE);

				nmp3L = mp3fillCmdNoParam(ubmp3buf, 0x11);
				MEM_DUMP(10, "M1->",ubmp3buf, nmp3L);
				uart5_send_bytes(ubmp3buf, nmp3L);
			}
			
		}
	}

	else if (ev == PROCESS_EVENT_TIMER && data == &et_send)
	{
		XPRINTF((8, "UDP SEND TIME OUT\n"));
		if (ubSendFailed >= 3)
		{
			gprs_process = &sim900a_hard_init;
			gprsState = SIM900A_TCPUDP_CLOSE_T;
			process_start(&sim900a_hard_init, NULL);
			XPRINTF((10, "et_send out\n"));
			ubSendFailed = 0;
		}
		ubSendFailed++;
	}

	else if (ev == PROCESS_EVENT_TIMER && data == &et_wait_sms_ok)
	{
		//send 1 failed
		XPRINTF((10, "ubPhoneIndex = %d\n", ubPhoneIndex));
		if (ubPhoneIndex == 1)
		{
			XPRINTF((10, "ubPhoneIndex = %d\n", ubPhoneIndex));
			XPRINTF((10, "sms22-------------------------\n"));
			ubPhoneIndex++;
			
			memset(ubUTF8Buf, 0, sizeof(ubUTF8Buf));
			memset(ubasiicBuf, 0, sizeof(ubUTF8Buf));
			memset(ubcmdBuf, 0, sizeof(ubcmdBuf));
			memset(smsDestN, 0, sizeof(smsDestN));
			
			#if 0
			ubLen = asiicToUTF8(ubUTF8Buf, stPhone.ubaPhoneB);
			nLen = hex2ascii((const uint8 *)ubUTF8Buf, ubasiicBuf, ubLen);
			XPRINTF((10, "asic nlen =%d\n", strlen(ubasiicBuf)));
			xsprintf(ubcmdBuf,"AT+CMGS=\"%s\"",ubasiicBuf);
			sim900a_send_cmd(ubcmdBuf);
			#else
			smsPnoneByteReverse(ubUTF8Buf, (const u_char*)smsCenterPhone);
			smsPnoneByteReverse(smsDestN, (const u_char *)stPhone.ubaPhoneB);
			xsprintf(ubcmdBuf,"%s%s%s%s%s%s", smsCenterPrex,ubUTF8Buf, smsDestPrex, smsDestN, smsDestCodeLen,warnMsg);
			XPRINTF((8, "warnsms=%s\n", ubcmdBuf));
			sim900a_send_cmd("AT+CMGS=27");				
			#endif
			
			etimer_set(&et_wait_sms_ok, GPRS_TIME_WAIT_SMS);
		}

		//first phone failed
		else
		{
			XPRINTF((10, "pono1-------------------------\n"));
			sim900a_send_cmd("ATH");
			xsprintf(ubcmdBuf,"ATD%s;",stPhone.ubaPhoneA);
			sim900a_send_cmd(ubcmdBuf);
			etimer_set(&et_phone, GPRS_TIME_WAIT_PHONE);
			XPRINTF((10, "start2 phone\n"));
			
			nmp3L = mp3fillCmdNoParam(ubmp3buf, 0x11);
			MEM_DUMP(10, "M2->",ubmp3buf, nmp3L);
			uart5_send_bytes(ubmp3buf, nmp3L);

		}
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_back_udp)
	{
		//sim900a_tcpudp_con_process
		//gprs_process = &sim900a_hard_init;
		//gprsState = SIM900A_TCPUDP_CLOSE_T;
		//process_start(&sim900a_hard_init, NULL);
		
		gprs_process = &sim900a_tcpudp_con_process;
		gprsState = SIM900A_TCPUDP_CLOSE_T;
		process_start(&sim900a_tcpudp_con_process, NULL);
		
		ubPhoneIndex = 0;
		XPRINTF((10, "et_back_udp out\n"));

		nmp3L = mp3fillCmdNoParam(ubmp3buf, 0x1e);
		MEM_DUMP(10, "M3->",ubmp3buf, nmp3L);
		uart5_send_bytes(ubmp3buf, nmp3L);
		
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_phone)
	{
		//
		XPRINTF((10, "start phone 2\n"));
		sim900a_send_cmd("ATH");
		xsprintf(ubcmdBuf,"ATD%s;",stPhone.ubaPhoneB);
		//etimer_stop(&et_wait_sms_ok);
		sim900a_send_cmd(ubcmdBuf);
		nmp3L = mp3fillCmdNoParam(ubmp3buf, 0x11);
		MEM_DUMP(10, "M2->",ubmp3buf, nmp3L);
		uart5_send_bytes(ubmp3buf, nmp3L);		
	}
}
//PROCESS(sim900a_send_process, "sim900a_send");
PROCESS_THREAD(sim900a_send_process, ev, data)
{

	PROCESS_BEGIN( );

	while(1)
	{
		PROCESS_YIELD( );
		sim900a_send_handler(ev, data);
	}
	PROCESS_END( );
}


static struct ringbuf ringuartbuf;
static uint8_t uartbuf[128];

/*---------------------------------------------------------------------------*/
int gprs_uart_input_byte(unsigned char c)
{
	static uint8_t overflow = 0; /* Buffer overflow: ignore until END */
	if(!overflow)
	{
		/* Add character */
		if(ringbuf_put(&ringuartbuf, c) == 0)
		{
			/* Buffer overflow: ignore the rest of the line */
			overflow = 1;
		}
	}
	else
	{
		/* Buffer overflowed:
		* Only (try to) add terminator characters, otherwise skip */
		if(ringbuf_put(&ringuartbuf, c) != 0)
		{
			overflow = 0;
		}
	}
	/* Wake up consumer process */
	process_poll(&gprs_resp_process);
	//XPRINTF((10, "gprs\n"));
	return 1;
}

void gprs_uart_init(void)
{
	ringbuf_init(&ringuartbuf, uartbuf, sizeof(uartbuf));
	//process_start(&hwfs_uart_rev_process, NULL);
	Uart_GprsSetInput(gprs_uart_input_byte);
}


//sim900a_app_process
PROCESS_THREAD(gprs_resp_process, ev, data)
{
	static char buf[SIMG900A_MAX_TCPUDP_DATA_LEN];
	static struct etimer et_rev_timeout;
	static int ptr = 0;
	char *pcsca;
	int c;

	PROCESS_BEGIN();
	sim900_event_resp = process_alloc_event( );
	sim900_event_send_cmd = process_alloc_event( );
	sim900_event_send_data = process_alloc_event( );
	sim900_event_heart = process_alloc_event( );
	sim900_event_send_data_wait = process_alloc_event( );
	sim900_event_fire_warn = process_alloc_event( );
	sim900_event_start_sms_phone = process_alloc_event( );
	sim900_event_send_sms_phone = process_alloc_event( );
	sim900_event_send_sms = process_alloc_event( );
	sim900_event_send_sms_wait = process_alloc_event( );
	sim900_event_fire_tran = process_alloc_event( );
	XPRINTF((10, "sim900a_resp_process\r\n"));

	while(1)
	{
		c = ringbuf_get(&ringuartbuf);
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			XPRINTF((12, "receive finish prt= %d\r\n", ptr));
			memset(&sim900_rx, 0, sizeof(SIM900A_MSG));
			sim900_rx.nLen = ptr;
			memcpy(sim900_rx.ubamsg, buf, ptr);
			MEM_DUMP(10, "<-gp", buf, ptr);
			process_post(gprs_process, sim900_event_resp, sim900_rx.ubamsg);
			//process_post(gprs_process, sim900_event_resp, &sim900_rx);
			//sim900a_send_cmd("AT+CSCA?");
			if ((char *)strstr(buf,"SMS Ready") != NULL)
			{
				sim900a_send_cmd("AT+CSCA?");
			}
			if ((char*)strstr(buf, "CSCA") != NULL)
			{
				pcsca = (char *)strstr(buf,"+86");
				memcpy(smsCenterPhone, pcsca+3, 11);
				XPRINTF((8, "smsCenterPhone = %s\n", smsCenterPhone));
			}
			memset(buf, 0 ,sizeof(buf));
			ptr = 0;
		}

		if(c == -1)
		{
			/* Buffer empty, wait for poll */
			PROCESS_YIELD();
		}
		else
		{
			//XPRINTF((10 "c = %02x\n", c));
			//XPRINTF((9, "%02x\r\n", c));
			if (ptr == 0 && (uint8_t)c != 0x0d)
			{
				ptr = 0;
			}
			else
			{
				buf[ptr++] = (uint8_t)c;
				if (ptr < SIMG900A_MAX_TCPUDP_DATA_LEN)
				{
					etimer_set(&et_rev_timeout, 200);
				}
				else
				{
					ptr = 0;
				}
			}
		}
	}
	PROCESS_END();
}


void gfireClean(void)
{
	static FIRE_NODE_INFO stFireNode;
	memset(&stFireNode, 0, sizeof(FIRE_NODE_INFO));
	extgdbdevSetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO, 0, (const void *)&stFireNode, sizeof(FIRE_NODE_INFO));
}



void sim900a_init(void)
{
	gprs_uart_init( );
	process_start(&sim900a_hard_init, NULL);
	process_start(&gprs_resp_process, NULL);
	process_start(&sim900a_app_process, NULL);
	process_start(&sim900a_send_process, NULL);
	process_start(&gprs_sms_phone_process, NULL);
}


/*This function is used to test*/
/*
test gprs ack
*/
void testGprsAck(void)
{
	int nFrameL = -1;
	NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	nFrameL = gprsProtocolFrameFill(sim900_tx.ubamsg, GPRS_F_CMD_ACK, uwSeq, paddrInfo->ubaNodeAddr, NULL, 0);
	process_post(gprs_process, sim900_event_resp, sim900_tx.ubamsg);
}




/*
test sync from pc
*/
typedef struct node_sync{
	u_short uwNums;
	u_char ubaBuf[1000];
}NODE_SYNC;

NODE_SYNC nodeSync={
	64,
	{\
	0x00,0x01,0x02,0x03,\
	0x04,0x05,0x06,0x07,\
	0x08,0x09,0x0a,0x0b,\
	0x0c,0x0d,0x0e,0x0f,\
	0x10,0x11,0x12,0x13,\
	0x14,0x15,0x16,0x17,\
	0x18,0x19,0x1a,0x1b,\
	0x1c,0x1d,0x1e,0x1f,\
	0x20,0x21,0x22,0x23,\
	0x24,0x25,0x26,0x27,\
	0x28,0x29,0x2a,0x2b,\
	0x2c,0x2d,0x2e,0x2f,\
	0x30,0x31,0x32,0x33,\
	0x34,0x35,0x36,0x37,\
	0x38,0x39,0x3a,0x3b,\
	0x3c,0x3d,0x3e,0x3f,\
	0x40,0x41,0x42,0x43,\
	0x44,0x45,0x46,0x47,\
	0x48,0x49,0x4a,0x4b,\
	0x4c,0x4d,0x4e,0x4f,\
	0x50,0x51,0x52,0x53,\
	0x54,0x55,0x56,0x57,\
	0x58,0x59,0x5a,0x5b,\
	0x5c,0x5d,0x5e,0x5f,\
	0x60,0x61,0x62,0x63,\
	0x64,0x65,0x66,0x67,\
	0x68,0x69,0x6a,0x6b,\
	0x6c,0x6d,0x6e,0x6f,\
	0x70,0x71,0x72,0x73,\
    0x74,0x75,0x76,0x77,\
	0x78,0x79,0x7a,0x7b,\
	0x7c,0x7d,0x7e,0x7f,\
	0x80,0x81,0x82,0x83,\
	0x84,0x85,0x86,0x87,\
	0x88,0x89,0x8a,0x8b,\
	0x8c,0x8d,0x8e,0x8f,\
	0x90,0x91,0x92,0x93,\
	0x94,0x95,0x96,0x97,\
	0x98,0x99,0x9a,0x9b,\
	0x9c,0x9d,0x9e,0x9f,\
	0xa0,0xa1,0xa2,0xa3,\
	0xa4,0xa5,0xa6,0xa7,\
	0xa8,0xa9,0xaa,0xab,\
	0xac,0xad,0xae,0xaf,\
	0xb0,0xb1,0xb2,0xb3,\
	0xb4,0xb5,0xb6,0xb7,\
	0xb8,0xb9,0xba,0xbb,\
	0xbc,0xbd,0xbe,0xbf,\
	0xc0,0xc1,0xc2,0xc3,\
	0xc4,0xc5,0xc6,0xc7,\
	0xc8,0xc9,0xca,0xcb,\
	0xcc,0xcd,0xce,0xcf,\
	0xd0,0xd1,0xd2,0xd3,\
	0xd4,0xd5,0xd6,0xd7,\
	0xd8,0xd9,0xda,0xdb,\
	0xdc,0xdd,0xde,0xdf,\
	0xe0,0xe1,0xe2,0xe3,\
	0xe4,0xe5,0xe6,0xe7,\
	0xe8,0xe9,0xea,0xeb,\
	0xec,0xed,0xee,0xef,\
	0xf0,0xf1,0xf2,0xf3,\
	0xf4,0xf5,0xf6,0xf7,\
	0xf8,0xf9,0xfa,0xfb,\
	0xfc,0xfd,0xfe,0xff,\
	0x65,0x00,0x00,0x00,\
	0x66,0x00,0x00,0x00,\
	0x67,0x00,0x00,0x00,\
	0x68,0x00,0x00,0x00,\
	0x69,0x00,0x00,0x00,\
	0x60,0x00,0x00,0x00,\
	0x71,0x00,0x00,0x00,\
	0x72,0x00,0x00,0x00,\
	0x73,0x00,0x00,0x00,\
	0x74,0x00,0x00,0x00,\
	0x75,0x00,0x00,0x00,\
	0x76,0x00,0x00,0x00,\
	0x77,0x00,0x00,0x00,\
	0x78,0x00,0x00,0x00,\
	0x79,0x00,0x00,0x00,\
	0x80,0x00,0x00,0x00,\
	0x81,0x00,0x00,0x00,\
	0x82,0x00,0x00,0x00,\
	0x83,0x00,0x00,0x00,\
	0x84,0x00,0x00,0x00,\
	0x85,0x00,0x00,0x00,\
	0x86,0x00,0x00,0x00,\
	0x87,0x00,0x00,0x00,\
	0x88,0x00,0x00,0x00,\
	0x89,0x00,0x00,0x00,\
	0x90,0x00,0x00,0x00,\
	0x91,0x00,0x00,0x00,\
	0x92,0x00,0x00,0x00,\
	0x93,0x00,0x00,0x00,\
	0x94,0x00,0x00,0x00,\
	0x95,0x00,0x00,0x00,\
	0x96,0x00,0x00,0x00,\
	0x97,0x00,0x00,0x00,\
	0x98,0x00,0x00,0x00,\
	0x99,0x00,0x00,0x00,\
	0xa0,0x00,0x00,0x00,\
	0x00,0x01,0x00,0x00,\
	0x00,0x02,0x00,0x00,\
	0x00,0x03,0x00,0x00,\
	0x00,0x04,0x00,0x00,\
	0x00,0x05,0x00,0x00,\
	0x00,0x06,0x00,0x00,\
	0x00,0x07,0x00,0x00,\
	0x00,0x08,0x00,0x00,\
	0x00,0x09,0x00,0x00,\
	0x00,0x10,0x00,0x00,\
	0x00,0x11,0x00,0x00,\
	0x00,0x12,0x00,0x00,\
	0x00,0x13,0x00,0x00,\
	0x00,0x14,0x00,0x00,\
	0x00,0x15,0x00,0x00,\
	0x00,0x16,0x00,0x00,\
	0x00,0x17,0x00,0x00,\
	0x00,0x18,0x00,0x00,\
	0x00,0x19,0x00,0x00,\
	0x00,0x20,0x00,0x00,\
	0x00,0x21,0x00,0x00,\
	0x00,0x22,0x00,0x00,\
	0x00,0x23,0x00,0x00,\
	0x00,0x24,0x00,0x00,\
	0x00,0x25,0x00,0x00,\
	0x00,0x26,0x00,0x00,\
	0x00,0x27,0x00,0x00,\
	0x00,0x28,0x00,0x00,\
	0x00,0x29,0x00,0x00,\
    0x00,0x30,0x00,0x00,\
	0x00,0x31,0x00,0x00,\
	0x00,0x32,0x00,0x00,\
	0x00,0x33,0x00,0x00,\
	0x00,0x34,0x00,0x00,\
	0x00,0x35,0x00,0x00,\
	0x00,0x36,0x00,0x00,\
	0x00,0x37,0x00,0x00,\
	0x00,0x38,0x00,0x00,\
	0x00,0x39,0x00,0x00,\
	0x00,0x40,0x00,0x00,\
	0x00,0x41,0x00,0x00,\
	0x00,0x42,0x00,0x00,\
	0x00,0x43,0x00,0x00,\
	0x00,0x44,0x00,0x00,\
	0x00,0x45,0x00,0x00,\
	0x00,0x46,0x00,0x00,\
	0x00,0x47,0x00,0x00,\
	0x00,0x48,0x00,0x00,\
	0x00,0x49,0x00,0x00,\
	0x00,0x50,0x00,0x00,\
	0x00,0x51,0x00,0x00,\
	0x00,0x52,0x00,0x00,\
	0x00,0x53,0x00,0x00,\
	0x00,0x54,0x00,0x00,\
	0x00,0x55,0x00,0x00,\
	0x00,0x56,0x00,0x00,\
	0x00,0x57,0x00,0x00,\
	0x00,0x58,0x00,0x00,\
	0x00,0x59,0x00,0x00,\
	0x00,0x60,0x00,0x00,\
	0x00,0x61,0x00,0x00,\
	0x00,0x62,0x00,0x00,\
	0x00,0x63,0x00,0x00,\
	0x00,0x64,0x00,0x00,\
	0x00,0x65,0x00,0x00,\
	0x00,0x66,0x00,0x00,\
	0x00,0x67,0x00,0x00,\
	0x00,0x68,0x00,0x00,\
	0x00,0x69,0x00,0x00,\
	0x00,0x60,0x00,0x00,\
	0x00,0x71,0x00,0x00,\
	0x00,0x72,0x00,0x00,\
	0x00,0x73,0x00,0x00,\
	0x00,0x74,0x00,0x00,\
	0x00,0x75,0x00,0x00,\
	0x00,0x76,0x00,0x00,\
	0x00,0x77,0x00,0x00,\
	0x00,0x78,0x00,0x00,\
	0x00,0x79,0x00,0x00,\
	0x00,0x80,0x00,0x00,\
	0x00,0x81,0x00,0x00,\
	0x00,0x82,0x00,0x00,\
	0x00,0x83,0x00,0x00,\
	0x00,0x84,0x00,0x00,\
	0x00,0x85,0x00,0x00,\
	0x00,0x86,0x00,0x00,\
	0x00,0x87,0x00,0x00,\
	0x00,0x88,0x00,0x00,\
	0x00,0x89,0x00,0x00,\
	0x00,0x90,0x00,0x00,\
	0x00,0x91,0x00,0x00,\
	0x00,0x92,0x00,0x00,\
	0x00,0x93,0x00,0x00,\
	0x00,0x94,0x00,0x00,\
	0x00,0x95,0x00,0x00,\
	0x00,0x96,0x00,0x00,\
	0x00,0x97,0x00,0x00,\
	0x00,0x98,0x00,0x00,\
	0x00,0x99,0x00,0x00,\
	0x00,0xa0,0x00,0x00
	}
};

void testGprsSync(void)
{
	int nFrameL = -1;
	NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	nFrameL = gprsProtocolFrameFill(sim900_tx.ubamsg, GPRS_F_CMD_DATA_SYNC, uwSeq, paddrInfo->ubaNodeAddr, (const u_char *)&nodeSync, nodeSync.uwNums*4+2);
	process_post(gprs_process, sim900_event_resp, sim900_tx.ubamsg);
}


void testNodeInfo(void)
{
	const FIRE_NODE_INFO *pFireNode = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	MEM_DUMP(0, "fnod", pFireNode, sizeof(FIRE_NODE_INFO));
}

void testGprsSmsPhone(void)
{
	const u_char phoneNum[]="18820109265";
	static GPRS_WARN_PHONE stPhone = {0};
	memcpy(stPhone.ubaPhoneA, phoneNum, sizeof(phoneNum));
	memcpy(stPhone.ubaPhoneB, phoneNum, sizeof(phoneNum));

	MEM_DUMP(10, "phone", &stPhone, sizeof(GPRS_WARN_PHONE));

	process_post(&sim900a_app_process, sim900_event_start_sms_phone, &stPhone);
}

