#include "contiki.h"

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"

#include "net/ip/simple-udp.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "sysprintf.h"


#include "basictype.h"
#include "iodef.h"
#include "ip/uip.h"
#include "sim900a.h"
#include "gprsProtocol.h"
#include "dev_info.h"
#include "hwgg.h"
#include "app485.h"

#define UDP_PORT_C	4567
//#define UDP_PORT_C	4570


PROCESS_NAME(sim900a_app_process);
extern process_event_t sim900_event_start_sms_phone;


const uip_ip4addr_t stip4Addr={192,168,4,212};
const uip_ip4addr_t stip4MaskAddr={192,168,4,1};

//remote server ip addr
//const uip_ip4addr_t serverHostIp4Addr={119,29,155,148};
const uip_ip4addr_t serverHostIp4Addr={119,29,224,28};
//const u_char serverIp[]="119.29.224.28";
//const uip_ip4addr_t serverHostIp4Addr={192,168,3,6};
//remote server ip port
static const u_short serverHostPort = UDP_PORT_C;

extern uip_lladdr_t uip_lladdr;

static struct simple_udp_connection udp_connection;


static SIM900A_MSG stEncRxMsg;
static SIM900A_MSG stEncTxMsg;
static SIM900A_MSG stEncTxMsgTran;
volatile static NET_MODE stnetMode={NET_CONNECT_NONE, 0};
process_event_t event_ip_msg;
process_event_t event_ip_heart;
process_event_t event_ip_send_data;
process_event_t event_ip_warn;
process_event_t event_ip_tran;

#define IP_NOT_NET_WAIT_TIME	120		//second




/*---------------------------------------------------------------------------*/
PROCESS(ip_receive_process, "ip_receive");
PROCESS(ip_data_process, "ip_data");


void netModeSet(u_char mode)
{
	//if (mode == NET_CONNECT_ETH || mode == NET_CONNECT_NONE || mode == NET_CONNECT_SIM900)
	stnetMode.netMode = mode;
}


NET_MODE *netModeGet(void)
{
	return (NET_MODE*)&stnetMode;
}


static void updateNetMode(u_char mode)
{
	netModeSet(mode);
	stnetMode.lastRxTime = clock_seconds( );
}

static void checkNetMode(void)
{
	//not update time
	if ((stnetMode.lastRxTime + IP_NOT_NET_WAIT_TIME) < clock_seconds( ))
	{
		updateNetMode(NET_CONNECT_NONE);
	}
}



static void appSaveIpData(u_char *pdataBuf, const uint8_t *data, u_short dataLen)
{
	if (pdataBuf != NULL)
	{
		SIM900A_MSG *pMSG = (SIM900A_MSG *)pdataBuf;
		pMSG->nLen = dataLen;
		memcpy(pMSG->ubamsg, data, dataLen);
		process_post(&ip_data_process, event_ip_msg, pdataBuf);
		updateNetMode(NET_CONNECT_ETH);
	}
}





/*---------------------------------------------------------------------------*/
static void receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	XPRINTF((10,"Data received from "));
	XPRINTF((10," on port %d from port %d with length %d: \n",receiver_port, sender_port, datalen));
	MEM_DUMP(10 , "<-w", data, datalen);
	appSaveIpData((u_char *)&stEncRxMsg, data, datalen);
}

/*---------------------------------------------------------------------------*/
static uint8_t should_blink = 1;
static void route_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr, int num_routes)
{
  if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    should_blink = 0;
  } else if(event == UIP_DS6_NOTIFICATION_DEFRT_RM) {
    should_blink = 1;
  }
}

const u_char udp_testdata[]={0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x55,0x33,0x89};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ip_receive_process, ev, data)
{
  static struct etimer et;
  static uip_ip6addr_t serverHostIp6Addr;
  static struct uip_ds6_notification n;


  PROCESS_BEGIN();
  ip64_addr_4to6(&serverHostIp4Addr, &serverHostIp6Addr);
  uip_ds6_notification_add(&n, route_callback);
  simple_udp_register(&udp_connection, 2000, &serverHostIp6Addr, UDP_PORT_C, receiver);

 // etimer_set(&et, CLOCK_SECOND*120);
  while(1) {
    PROCESS_YIELD( );
    //etimer_set(&et, 30*CLOCK_SECOND);
    //PROCESS_WAIT_UNTIL(etimer_expired(&et));
    //simple_udp_send(&udp_connection, udp_testdata, sizeof(udp_testdata));
    /*
    if (ev == tcpip_event)
    {
    	if(uip_newdata())
    	{
			MEM_DUMP(6 , "<-w", uip_appdata, uip_datalen());
		}
    }
    */
  }
  PROCESS_END();
}



void closeGprs(void)
{

}

void openGprs(void)
{
	
}


#define IP_HEART_TIME  (30*CLOCK_SECOND)
#define IP_ACK_WAIT_TIME (10*CLOCK_SECOND)


void ipSendFireMacSync(u_char macSync, u_char uwSeq)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	int nFramL = -1;
	SIM900A_MSG *ptxMsg = &stEncTxMsg;
	
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
			process_post(&ip_data_process, event_ip_send_data,ptxMsg);
		}		
	}
}


void ipProtocolRxProcess(u_char *ptxBuf, const u_char *pcFrame, u_short uwSendSeq , struct etimer *petwait)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	const GPRS_PROTOCOL * pGprs = (const GPRS_PROTOCOL *)pcFrame;
	u_short uwSeq = pGprs->ubSeqL | (pGprs->ubSeqH << 8);
	u_short uwLen = pGprs->ubDataLenL | (pGprs->ubDataLenH<<8);
	static GPRS_WARN_PHONE stWarnPhone;
	static u_char macSync = 0;
	

	/*we have fire mac addr, not need to sync fire mac addr first time
    dev power up
	*/


	if (pGprs->ubCmd == GPRS_F_CMD_ACK)
	{
		if (uwSendSeq == uwSeq)
		{
			etimer_stop(petwait);
			ipSendFireMacSync(macSync, uwSeq);
		}
	}
	else if (pGprs->ubCmd == GPRS_F_CMD_DATA_SYNC)
	{

		SIM900A_MSG *ptxMsg = (SIM900A_MSG *)ptxBuf;
		static FIRE_NODE_INFO stFireNode;
		const FIRE_NODE_INFO *pFireNodeInfo; 
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;


		//now we sync fire mac addr
		if (macSync == 0)
		{
			macSync = 1;
		}
		
		memset(&stFireNode, 0, sizeof(FIRE_NODE_INFO));
		if (uwLen > GPRS_F_MAC_LEN)
		{
			pFireNodeInfo = (const FIRE_NODE_INFO *)pGprs->ubaData;
			if (pFireNodeInfo->node_num > 0)
			{
				stFireNode.node_num = pFireNodeInfo->node_num;
				memcpy(stFireNode.nodeArray, pFireNodeInfo->nodeArray, stFireNode.node_num*4);
				extgdbdevSetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO, 0, (const void *)&stFireNode, sizeof(FIRE_NODE_INFO));
			}
		}
		
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_ACK, uwSeq, paddrInfo->ubaNodeAddr, NULL, 0);

		if (nFramL > 0)
		{
			ptxMsg->nLen = nFramL;
			process_post(&ip_data_process, event_ip_send_data,ptxMsg);
		}
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
			NET_MODE *pnetMode = (NET_MODE*)netModeGet( );
			memcpy(&stWarnPhone, pGprs->ubaData, 40);
			if (pnetMode->netMode == NET_CONNECT_ETH)
			{
				if (checkPhoneNum((const GPRS_WARN_PHONE*) &stWarnPhone))
				{
					process_post(&sim900a_app_process, sim900_event_start_sms_phone, &stWarnPhone);
				}
			}
		}
	}
}


#define ETH_NO_ACK_MAX		2

void ipDataHandler(process_event_t ev, process_data_t data)
{
	static struct etimer et_ip_heart;
	static struct etimer et_ip_ack_wait;
	static u_short uwipSeq = 0;
	static u_short uwCurrentSeq = 0;
	static FIRE_NODE_INFO stFireNodeInfo;
	static u_char ubaFireWarnBuf[32] = {0x00};
	static u_char ubNoAckCount = 0;


	//receive packet from server
	if (ev == event_ip_msg && data != NULL)
	{
		SIM900A_MSG * pMsg = (SIM900A_MSG *)data;
		ubNoAckCount = 0;
		if (gprsProtocolCheck((const u_char *) pMsg->ubamsg))
		{
			const GPRS_PROTOCOL * pGprs = (const GPRS_PROTOCOL *)pMsg->ubamsg;
			u_short uwSeq = pGprs->ubSeqL | (pGprs->ubSeqH << 8);
			u_short uwLen = pGprs->ubDataLenL | (pGprs->ubDataLenH<<8);

			MEM_DUMP(7, "<-ip", pMsg->ubamsg, pMsg->nLen);
			ipProtocolRxProcess((u_char *)&stEncTxMsg,(const u_char *) pMsg->ubamsg, uwCurrentSeq, &et_ip_ack_wait);
			if (stnetMode.netMode != NET_CONNECT_ETH)
			{
				netModeSet(NET_CONNECT_ETH);
			}
		}	


	}
	else if (ev == event_ip_send_data && data != NULL)
	{
		SIM900A_MSG * pMsg = (SIM900A_MSG *)data;
		if (pMsg->nLen > 0)
		{
			int nFrameL = 0;
			nFrameL = gprsCodeGetOut0xla(stEncTxMsgTran.ubamsg, (const u_char*)pMsg->ubamsg, pMsg->nLen);
			
			//simple_udp_send(&udp_connection, pMsg->ubamsg, pMsg->nLen);
			simple_udp_send(&udp_connection, stEncTxMsgTran.ubamsg, nFrameL);
			etimer_set(&et_ip_ack_wait, IP_ACK_WAIT_TIME);
			MEM_DUMP(7, "IP->",  stEncTxMsgTran.ubamsg, nFrameL);
		}
	}
	//heart packet
	else if (ev == event_ip_heart && data == NULL)
	{
		etimer_set(&et_ip_heart, IP_HEART_TIME);
		XPRINTF((12, "IPHeart\n"));
	}
	//fire warn
	else if (ev == event_ip_warn && data != NULL)
	{
		SIM900A_MSG *ptxMsg = &stEncTxMsg;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		u_char ubaWarn[5] = 0;
		int nFramL = -1;
		const FIRE_NODE * pFireNode = (const FIRE_NODE *)data;
		
		if (((u_long)ubaFireWarnBuf)!=((u_long)data))
		{
			//MEM_DUMP(7, "IPWA", data, pFireNode->ubLen);
			memcpy(ubaFireWarnBuf, data, pFireNode->ubLen);
		}
		memcpy(ubaWarn, pFireNode->ubaSrcMac, HWGG_NODE_MAC_LEN);
		ubaWarn[4] = pFireNode->ubCmd;
		uwipSeq++;
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_WARN, uwipSeq, paddrInfo->ubaNodeAddr, ubaWarn, 5);
		if (nFramL > 0 )
		{
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwipSeq;
			process_post(&ip_data_process, event_ip_send_data, ptxMsg);
			etimer_set(&et_ip_ack_wait, IP_ACK_WAIT_TIME);
		}
		//ALARM_LED(0);
		FAULT_LED(0);
		BUZZER(0);
	}
	else if (ev == event_ip_tran && data != NULL)
	{
		SIM900A_MSG *ptxMsg = &stEncTxMsg;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;
		const APP_485_DATA *p485 = (const APP_485_DATA *)data;
		MEM_DUMP(7, "TRAN", p485->ubaData, p485->ubLen);

		uwipSeq++;
		nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_TRAN, uwipSeq, paddrInfo->ubaNodeAddr,(const u_char *)p485->ubaData, p485->ubLen);
		if (nFramL > 0 )
		{
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwipSeq;
			process_post(&ip_data_process, event_ip_send_data, ptxMsg);
			etimer_set(&et_ip_ack_wait, IP_ACK_WAIT_TIME);
		}
		
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_ip_heart)
	{
		SIM900A_MSG *ptxMsg = &stEncTxMsg;
		NODE_ADDR_INFO *paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
		int nFramL = -1;
		fillNotNetNodeInfo(&stFireNodeInfo);
		if (stFireNodeInfo.node_num > 0)
		{
			nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_HEART, uwipSeq, paddrInfo->ubaNodeAddr, (const u_char *)&stFireNodeInfo, stFireNodeInfo.node_num*4+2);
		}
		else
		{
			nFramL = gprsProtocolFrameFill(ptxMsg->ubamsg, GPRS_F_CMD_HEART, uwipSeq, paddrInfo->ubaNodeAddr, NULL, 0);
		}
		if (nFramL > 0)
		{
			ptxMsg->nLen = nFramL;
			uwCurrentSeq = uwipSeq;
			process_post(&ip_data_process, event_ip_send_data,ptxMsg);
			etimer_set(&et_ip_ack_wait, IP_ACK_WAIT_TIME);
			uwipSeq++; // every send a ip frame, uwipSeq + 1;
		}
		
		etimer_set(&et_ip_heart, IP_HEART_TIME);

		//checkNetMode( );
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_ip_ack_wait)
	{
		XPRINTF((12, "IP ACK TIME OUT\n"));
		if (ubNoAckCount >= ETH_NO_ACK_MAX)
		{
			if (stnetMode.netMode == NET_CONNECT_ETH)
			{
				XPRINTF((10, "-----------------------------set mode\n"));
				netModeSet(NET_CONNECT_NONE);
			}
			ubNoAckCount = 0;
		}
		ubNoAckCount++;
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ip_data_process, ev, data)
{
  PROCESS_BEGIN();
  event_ip_msg = process_alloc_event( );
  event_ip_heart = process_alloc_event( );
  event_ip_send_data = process_alloc_event( );
  event_ip_warn = process_alloc_event( );
  event_ip_tran = process_alloc_event( );
  XPRINTF((12, "ip_data_process\n"));
  
  while(1) {
    PROCESS_YIELD( );
    ipDataHandler(ev, data);
  }
  PROCESS_END();
}



void  appUpdateNetMode(void)
{
	updateNetMode(NET_CONNECT_ETH);
	//process_post(&ip_data_process, event_ip_heart, NULL);
}


void app_enc28j60_init(void)
{
	SPI_Config( );
	rpl_dag_root_init_dag();
	//ip64_set_ipv4_address(&stip4Addr, &stip4MaskAddr);
	//dhcpc_init(uip_lladdr.addr, sizeof(uip_lladdr.addr));
  	/* Initialize the IP64 module so we'll start translating packets */
	ip64_init();

	process_start(&ip_receive_process, NULL);
	process_start(&ip_data_process, NULL);

	process_post(&ip_data_process, event_ip_heart, NULL);
}

