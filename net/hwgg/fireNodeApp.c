#include "contiki.h"
#include "basictype.h"
#include "dev_info.h"
#include "hwgg.h"
#include "lib/ringbuf.h"
#include "sysprintf.h"
#include <string.h>
#include "end_node_list.h"

static struct ctimer period_nodeaddr_timer;
PROCESS(fire_node_uart, "fire_uart");
PROCESS(fire_app_process, "fire_app");
PROCESS(fire_read_process,"frie_read");


PROCESS_NAME(sim900a_app_process);

extern process_event_t sim900_event_fire_warn;
extern process_event_t sim900_event_fire_tran;

PROCESS_NAME(ip_data_process);
extern process_event_t event_ip_warn;
extern process_event_t event_ip_tran;

static process_event_t fire_event_rev;


static FIRE_NODE_DATA stFireData;

static bool nodeBelognToMeByMac(const u_char *pcMac)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	int i = 0;

	//if node information docment is save in flash, all node need to save to list
	if (pfireNodeInfo->node_num == 0)
		return true;

	for (i = 0; i < pfireNodeInfo->node_num; i++)
	{
		if (mem_cmp(pcMac, pfireNodeInfo->nodeArray[i].ubaNodeAddr, HWGG_NODE_MAC_LEN) == 0)
		{
			return true;
		}
	}

	return false;
}

static bool checkWsFrameCrc(const u_char *pcdata, u_char len)
{
	u_short uwCrc = 0;
	return true;
}


static bool fireAppCheckFrameLength(const HWGG_FRAME *pFrame, const FIRE_NODE *pFireNode)
{
	//MEM_DUMP(10, "nodf", pFrame, pFrame->ubLen + 4);
	if ((pFrame->ubLen - 12) != pFireNode->ubLen)
	{
		//XPRINTF((10, "pFireNode->ubLen = %d pFrame->ubLen = %d\n",pFireNode->ubLen ,pFrame->ubLen));
		return false;
	}
	return true;
}


static bool fireAppFilterNodeAddr(const u_char *pcFrame)
{
	const HWGG_FRAME *pFrame = NULL;
	const FIRE_NODE *pFireNode = NULL;
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	NODE_INFO *pnode = NULL;
	
	if (pcFrame == NULL)
		return false;

	pFrame = (const HWGG_FRAME *)pcFrame;
	pFireNode = (const FIRE_NODE *)pFrame->ubaData;

	if (!fireAppCheckFrameLength(pFrame, pFireNode))
		return false;

	if (pfireNodeInfo->node_num == 0)
		return true;

	MEM_DUMP(10, "find", pFrame, pFrame->ubLen+4);
	MEM_DUMP(10, "addr", pFireNode->ubaSrcMac, 4);
	//pnode = (NODE_INFO *)endNodeInListByMac(pFireNode->ubaSrcMac);
	if (nodeBelognToMeByMac((const u_char*) pFireNode->ubaSrcMac))
	{
		MEM_DUMP(10, "eddr", pnode->ubaHWGGMacAddr, 4);
		return true;
	}
	return false;
}



static void addFireNodeToList(const u_char *pcFrame)
{
	NODE_INFO stnodeInfo;
	const HWGG_FRAME *pFrame = NULL;
	const FIRE_NODE *pFireNode = NULL;	

	if (pcFrame == NULL)
		return;

	pFrame = (const HWGG_FRAME *)pcFrame;
	pFireNode = (const FIRE_NODE *)pFrame->ubaData;

	stnodeInfo.lastRevPacketTime = clock_seconds( );
	stnodeInfo.next = NULL;
	stnodeInfo.nodeNetState = HWGG_NODE_IN_NET;
	memcpy(stnodeInfo.ubaHWGGMacAddr, pFireNode->ubaSrcMac, HWGG_NODE_MAC_LEN);
	
	endNodeListadd((const NODE_INFO *)&stnodeInfo);
}

static bool nodeBelognToMe(const NODE_INFO *pcNode)
{
	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	int i = 0;

	//if node information docment is save in flash, all node need to save to list
	if (pfireNodeInfo->node_num == 0)
		return true;

	for (i = 0; i < pfireNodeInfo->node_num; i++)
	{
		if (mem_cmp(pcNode->ubaHWGGMacAddr, pfireNodeInfo->nodeArray[i].ubaNodeAddr, HWGG_NODE_MAC_LEN) == 0)
		{
			return true;
		}
	}

	return false;
}

static void removeNodeNotBelongToMe(void)
{
	NODE_INFO *pnode = NULL;
	bool isTrue = false;

	const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);

	//not have node info docment
	if (pfireNodeInfo->node_num == 0)
		return;

	for(pnode = (NODE_INFO *)endNodeListHead(); pnode != NULL; pnode = (NODE_INFO *)endNodeListNext(pnode)) 
	{
		isTrue = nodeBelognToMe((const NODE_INFO *)pnode);
		if (!isTrue)
		{
			endNodeListRemove(pnode);
		}
	}
}


static void nodeAddrCheckTimer(void *ptr)
{
	removeNodeNotBelongToMe( );
	endNodeListPeriodicCheck( );
	XPRINTF((10, "node check!\n"));
	ctimer_set(&period_nodeaddr_timer, 10*1000, nodeAddrCheckTimer, NULL);
}

static void setNodeAddrChecktimer(void)
{
	ctimer_set(&period_nodeaddr_timer, 30*1000, nodeAddrCheckTimer, NULL);
}





static void fireProtocolProcess(process_data_t data)
{
	const HWGG_FRAME *pFrame = NULL;
	const FIRE_NODE *pFireNode = NULL;
	//const FIRE_NODE_INFO *pfireNodeInfo = (const FIRE_NODE_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_FIRE_NODE_INFO);
	//NODE_INFO *pnode = NULL;

	pFrame = (const HWGG_FRAME *)data;
	pFireNode = (const FIRE_NODE *)pFrame->ubaData;


	MEM_DUMP(9, "rf<-", data, pFrame->ubLen + HWGG_HEAD_END_CRC_LEN);
	if (pFireNode->ubCmd == HWGG_CMD_HEART)
	{
		XPRINTF((12, "hwgg_cmd_heart\n"));
		addFireNodeToList(data);
	}
	else if (pFireNode->ubCmd == HWGG_CMD_LOW_VOLTAGE || pFireNode->ubCmd == HWGG_CMD_WARN)
	{
		XPRINTF((12, "hwgg_warn\n"));
		addFireNodeToList(data);
		process_post(&sim900a_app_process, sim900_event_fire_warn, (void*)pFireNode);
		process_post(&ip_data_process, event_ip_warn, (void*)pFireNode);
	}
	else if (pFireNode->ubCmd == HWGG_CMD_TRAN && pFireNode->ubLen > FIRE_FIX_LEN)
	{
		/*post data to */
		stFireData.ublen = pFireNode->ubLen - FIRE_FIX_LEN;
		stFireData.ubadata[0] = stFireData.ublen+5;
		stFireData.ubadata[1] = HWGG_CMD_TRAN;
		stFireData.ubadata[2] = pFireNode->ubaSrcMac[0];
		stFireData.ubadata[3] = pFireNode->ubaSrcMac[1];
		stFireData.ubadata[4] = pFireNode->ubaSrcMac[2];
		stFireData.ubadata[5] = pFireNode->ubaSrcMac[3];
		stFireData.ublen += 6;
		memcpy(&stFireData.ubadata[6], pFireNode->ubaData, stFireData.ublen);
		process_post(&sim900a_app_process, sim900_event_fire_tran, &stFireData);
		process_post(&ip_data_process, event_ip_tran, &stFireData);
	}
}



void frieAppHandler(process_event_t ev, process_data_t data)
{
	if (ev == fire_event_rev && data != NULL)
	{
		fireProtocolProcess(data);
	}
}

PROCESS_THREAD(fire_app_process, ev, data)
{
	PROCESS_BEGIN( );

	while(1)
	{
		PROCESS_YIELD( );
		frieAppHandler(ev, data);
	}
	PROCESS_END( );
}


static struct ringbuf ringuartbuf;
static uint8_t uartbuf[128];
/*----------------------------------------------------------------------*/
#define FIRE_RXBUFS	 	8
#define FIRE_BUF_LEN	128

static uint8_t fire_rxbufs[FIRE_RXBUFS][FIRE_BUF_LEN];

#if FIRE_RXBUFS > 1
static volatile int8_t fire_first = -1, fire_last = 0;
#else   
static const int8_t fire_first = 0, fire_last = 0;
#endif  

#if FIRE_RXBUFS > 1
#define CLEAN_FIRE_RXBUFS() do{fire_first = -1; fire_last = 0;}while(0)
#define FIRE_RXBUFS_EMPTY() (fire_first == -1)
static int FIRE_RXBUFS_FULL( )
{
	int8_t first_tmp = fire_first;
	return first_tmp == fire_last;
}
#else 
#define CLEAN_FIRE_RXBUFS( ) (fire_rxbufs[0][0] = 0)
#define FIRE_RXBUFS_EMPTY( ) (fire_rxbufs[0][0] == 0)
#define FIRE_RXBUFS_FULL( )  (fire_rxbufs[0][0] != 0)
#endif 

/*---------------------------------------------------------------------------*/
static int add_to_fire_rxbuf(uint8_t *src)
{
	HWGG_FRAME *pHwgg = NULL;
	u_char ubLen = 0;
	if(FIRE_RXBUFS_FULL()) 
	{
		return 0;
	}

	//TODO:need to know frame length
	pHwgg = (HWGG_FRAME *)src;
	ubLen = pHwgg->ubLen + HWGG_HEAD_END_CRC_LEN;
	memcpy(fire_rxbufs[fire_last], src, ubLen);

	#if FIRE_RXBUFS > 1
	fire_last = (fire_last + 1) % FIRE_RXBUFS;
	if(fire_first == -1) 
	{
		fire_first = 0;
	}
	#endif

	//TODO:need to know frame length
	memset(src, 0, ubLen);//clear buf
	return 1;
}

/*---------------------------------------------------------------------------*/
static int read_from_fire_rxbuf(void *dest, unsigned short len)
{
	HWGG_FRAME *pHwgg = NULL;
	u_char ubLen = 0;

	if(FIRE_RXBUFS_EMPTY()) 
	{          
		return 0;
	}

	pHwgg = (HWGG_FRAME *)fire_rxbufs[fire_first];
	ubLen = pHwgg->ubLen + HWGG_HEAD_END_CRC_LEN;
	if (ubLen > len)
	{   
		len = 0;
	} 
	else 
	{
		len = ubLen;  //frame length
		memcpy(dest, (uint8_t*)&fire_rxbufs[fire_first][0], len);
		memset((uint8_t*)&fire_rxbufs[fire_first][0], 0, len);
	}

	#if FIRE_RXBUFS > 1
	{
		int first_tmp;
		fire_first = (fire_first + 1) % FIRE_RXBUFS;
		first_tmp = fire_first;
		if(first_tmp == fire_last) 
		{
			CLEAN_FIRE_RXBUFS();
		}
	}
	#else
	CLEAN_FIRE_RXBUFS();
	#endif
	return len;
}



PROCESS_THREAD(fire_node_uart, ev, data)
{
	static u_char buf[128];
	static struct etimer et_rev_timeout;
	static int ptr;
	HWGG_FRAME *pHwgg;
	int c;
	
	PROCESS_BEGIN();
	XPRINTF((12, "fire_node_uart\r\n"));
	fire_event_rev = process_alloc_event( );
	
	while(1) 
	{
		c = ringbuf_get(&ringuartbuf);
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			XPRINTF((8, "T_O\r\n"));
			ptr = 0;
			buf[0] = 0;
		}

		if(c == -1) 
		{
			PROCESS_YIELD();
		} 
		else 
		{
			if (ptr <= 128)
			{
				buf[ptr++] = (uint8_t)c;
				if (ptr==1 && buf[0] == HWGG_HEAD)
				{
					//set timeout 
					//Frame start
					XPRINTF((12, "start\r\n"));
					etimer_set(&et_rev_timeout, 500);					
				}

				//head error
				if (buf[0] != HWGG_HEAD)
				{
					ptr = 0;
					buf[0] = 0;
				}
				else
				{
					if (ptr >= HWGG_FRAME_FIX_LEN)
					{
						pHwgg = (HWGG_FRAME *)buf;
						if ((c == HWGG_END)&& (pHwgg->ubLen + HWGG_HEAD_END_CRC_LEN) == ptr)
						{
							//MEM_DUMP(10,"ra<-", buf, ptr);
							if (fireAppFilterNodeAddr((const u_char *)buf))
							{
								//MEM_DUMP(10,"filt", buf, ptr);
								add_to_fire_rxbuf(buf);
								process_poll(&fire_read_process);
							}
							etimer_stop(&et_rev_timeout);
							ptr = 0;
							buf[0] = 0;
						}
					}
				}
			}
			else
			{
				ptr = 0;
				buf[0] = 0;
			}
		}
	}
	PROCESS_END();
}




/******************************************************************/
PROCESS_THREAD(fire_read_process, ev, data)
{
	int len;
	static u_char ubBuf[128];
	PROCESS_BEGIN();
	XPRINTF((12,"rf_radio_process: started\r\n"));

	
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

		len = read_from_fire_rxbuf(ubBuf, FIRE_BUF_LEN);
		if(len > 0) 
		{
			//MEM_DUMP(10, "READ", ubBuf, len);
			process_post(&fire_app_process, fire_event_rev, ubBuf);
		}
		if(!FIRE_RXBUFS_EMPTY()) 
		{
			/*
			* Some data packet still in rx buffer (this happens because process_poll
			* doesn't queue requests), so stm32w_radio_process needs to be called
			* again.
			*/
			process_poll(&fire_read_process);
		}
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
static int fire_uart_input_byte(unsigned char c)
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
	process_poll(&fire_node_uart);	
	return 1;
}

static void fire_uart_init(void)
{
	ringbuf_init(&ringuartbuf, uartbuf, sizeof(uartbuf));
	process_start(&fire_node_uart, NULL);
	Uart_RfSetInput(fire_uart_input_byte);
}

void fireAppInit(void)
{
	fire_uart_init( );
	setNodeAddrChecktimer( );
	process_start(&fire_app_process, NULL);
	process_start(&fire_read_process, NULL);
}



