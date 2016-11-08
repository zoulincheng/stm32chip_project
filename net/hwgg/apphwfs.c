#include "contiki.h"
#include "basictype.h"
#include "hwgg.h"
#include "hwfs.h"
#include "sysprintf.h"
#include "common.h"
#include <string.h>
#include "apphwgg.h"
#include "dev_info.h"
#include "lib/ringbuf.h"


static u_char ubaHwfsTXBuf[32];
static u_char ubaHwfsTXBufAssic[32];
PROCESS(hwfs_uart_rev_process, "app_hwfs_uart");
PROCESS(hwfs_frame_process, "hwfs_frame");
PROCESS(app_hwfs_process, "app_hwfs");

process_event_t app_hwfs_event;

static u_short hwfsExtractPacketCrc(const u_char* pciFrame, u_char ubFrameL)
{
	u_short pktCrc = 0;
	pktCrc = (pciFrame[ubFrameL-3]<<8)|(pciFrame[ubFrameL-2]);
	return pktCrc;
}

static bool hwfsPacketCrcRight(const u_char* pciFrame, u_char ubFrameL)
{
	const HWFS_FRAME *pcHwfs = NULL;
	u_short uwCrc = 0;
	u_short uwPacketCrc = 0;

	if (pciFrame == NULL)
		return false;
	pcHwfs = (const HWFS_FRAME *)pciFrame;
	uwPacketCrc = extractPacketCrc(pciFrame);
	uwCrc = cyg_crc16((const unsigned char*)&pcHwfs->ubaNetID, ubFrameL-HWFS_FRAME_HEAD_CRC_END_BYTES);

	if (uwCrc == uwPacketCrc)
		return true;
	return false;
}


static bool hwfsPacketIsMy(const u_char* pciFrame)
{
	const HWFS_FRAME *pcHwfs = (const HWFS_FRAME *)pciFrame;
	const RF_NODE_PARAM_CONFIG *pnodeCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	//MEM_DUMP(6, "info", pnodeCfg, sizeof(RF_NODE_PARAM_CONFIG));
	if (mem_cmp(pcHwfs->ubaNetID, pnodeCfg->ubaHwfsNetId, HWGG_NETADDR_LEN) == 0)
		return true;
	return false;
}


/*
note This function process HWFS_CMD_CFG_NET only is router
*/
static int hwfs_cmd_cfg_net_process(u_char *pioFrame, const u_char* pciFrame, u_char ubFrameL)
{
	const HWFS_FRAME *pHwfs = (const HWFS_FRAME *)pciFrame;
	const HWFS_CFG_NET_DATA *pNetData = (const HWFS_CFG_NET_DATA *)pHwfs->ubaData;
	int nFrameL = -1;
	u_char ubLen = 0;
	const RF_NODE_PARAM_CONFIG *pNodeParam = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	RF_NODE_PARAM_CONFIG stNodeParam = *pNodeParam;
	HWFS_CFG_NET_DATA stNetData = {0x00};
	//save net param

	stNodeParam.ubRFChannel = pNetData->ubchannel;
	stNodeParam.ubHwfsPanId = pNetData->ubPanID;

	extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stNodeParam, sizeof(RF_NODE_PARAM_CONFIG));
	stNetData.ubchannel = pNodeParam->ubRFChannel;
	stNetData.ubPanID = pNodeParam->ubHwfsPanId;
	ubLen = (u_char *)&stNetData.ubPanID - (u_char *)&stNetData + 1;
	nFrameL = hwfsFillToHostFrame(pioFrame, pNodeParam->ubaHwfsNetId, pNodeParam->ubHwfsDevId, 0x33, HWFS_CMD_CFG_NET, (const u_char*)&stNetData, ubLen);
	return nFrameL;
}


static int hwfs_cmd_get_net_param_process(u_char *pioFrame, const u_char* pciFrame, u_char ubFrameL)
{
	const HWFS_FRAME *pHwfs = (const HWFS_FRAME *)pciFrame;
	int nFrameL = -1;
	u_char ubLen = 0;
	const RF_NODE_PARAM_CONFIG *pNodeParam = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	HWFS_CFG_NET_DATA stNetData = {0x00};

	stNetData.ubchannel = pNodeParam->ubRFChannel;
	stNetData.ubPanID = pNodeParam->ubHwfsPanId;
	ubLen = (u_char *)&stNetData.ubPanID - (u_char *)&stNetData + 1;
	nFrameL = hwfsFillToHostFrame(pioFrame, pNodeParam->ubaHwfsNetId, pNodeParam->ubHwfsDevId, 0x33, HWFS_CMD_GET_NET_PARAM, (const u_char*)&stNetData, ubLen);
	return nFrameL;
}


static void hwfs_uart_burad_cfg(u_char ubBuardIndex)
{
	uart2_cfg(ubBuardIndex);
}


static int hwfs_cmd_modify_uart_buard_process(u_char *pioFrame, const u_char* pciFrame, u_char ubFrameL)
{
	const HWFS_FRAME *pHwfs = (const HWFS_FRAME *)pciFrame;
	const RF_NODE_PARAM_CONFIG *pNode = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	int nFrameL = -1;
	u_char ubUartBuard = pHwfs->ubaData[0];

	hwfs_uart_burad_cfg(ubUartBuard);

	nFrameL = hwfsFillToHostFrame(pioFrame, pNode->ubaHwfsNetId, pNode->ubHwfsDevId, 0x33, HWFS_CMD_MODIFY_UART_BUARD, pHwfs->ubaData, 1);
	return nFrameL;
}

static int routerNodeProcessPacketAndAckHost(u_char *pioBuf,const HWFS_FRAME *pcHwfs, u_char ubLen)
{
	int nFrameL = -1;
	switch(pcHwfs->ubCMD)
	{
		case HWFS_CMD_CFG_NET:
			XPRINTF((10, "HWFS_CMD_CFG_NET router\r\n"));
			nFrameL = hwfs_cmd_cfg_net_process(pioBuf, (const u_char*)pcHwfs, ubLen);
			break;
		case HWFS_CMD_QUERY_ROUTER_ADDR:
			XPRINTF((10, "HWFS_CMD_QUERY_ROUTER_ADDR router\r\n"));
			break;
		case HWFS_CMD_FORBID_END_NODE:
			XPRINTF((10, "HWFS_CMD_FORBID_END_NODE router\r\n"));
			break;
		case HWFS_CMD_ALLOW_END_NODE:
			XPRINTF((10, "HWFS_CMD_ALLOW_END_NODE router\r\n"));
			break;
		case HWFS_CMD_SEND_MAC:
			XPRINTF((10, "HWFS_CMD_SEND_MAC router\r\n"));
			break;
		case HWFS_CMD_CFG_WIRELESS:
			XPRINTF((10, "HWFS_CMD_CFG_WIRELESS router\r\n"));
			break;
		case HWFS_CMD_GET_NET_PARAM:
			XPRINTF((10, "HWFS_CMD_GET_NET_PARAM router\r\n"));
			nFrameL = hwfs_cmd_get_net_param_process(pioBuf, (const u_char*)pcHwfs, ubLen);
			break;
		case HWFS_CMD_DEV_TYPE_CHANGE:
			XPRINTF((10, "HWFS_CMD_DEV_TYPE_CHANGE router\r\n"));
			break;
		case HWFS_CMD_SEARCH_NETID_BY_DEVID:
			XPRINTF((10, "HWFS_CMD_SEARCH_NETID_BY_DEVID router\r\n"));
			break;
		case HWFS_CMD_MODIFY_UART_BUARD:
			XPRINTF((10, "HWFS_CMD_MODIFY_UART_BUARD router\r\n"));
			nFrameL = hwfs_cmd_modify_uart_buard_process(pioBuf, (const u_char*)pcHwfs, ubLen);
			break;
		case HWFS_CMD_TRAN_DATA:
			XPRINTF((10, "HWFS_CMD_TRAN_DATA router\r\n"));
			break;
		case HWFS_CMD_BROAD_CAST:
			XPRINTF((10, "HWFS_CMD_BROAD_CAST router\r\n"));
			break;
		case HWFS_CMD_GET_MAC_BY_SHORT_ADDDR:
			XPRINTF((10, "HWFS_CMD_GET_MAC_BY_SHORT_ADDDR router\r\n"));
			break;
		case HWFS_CMD_MODIFY_DOWN_DELAY_TIME:
			XPRINTF((10, "HWFS_CMD_MODIFY_DOWN_DELAY_TIME router\r\n"));
			break;
		default:
			break;
	}
}


static void updateDownData(NODE_INFO *pnode, const HWFS_DATA *pcHwfsData)
{
	pnode->ubLenDown = 7;
	memcpy(pnode->ubaDataDown, pcHwfsData, pnode->ubLenDown);
	MEM_DUMP(10,"hdat", pnode->ubaDataDown, pnode->ubLenDown);
}


/*
if cmd is braodcast, update all node updown data.
*/
static void updateEndNodeDownDataBraodcastCmd(const HWFS_DATA *pcHwfsData)
{
	NODE_INFO *pnode = (NODE_INFO *)endNodeListHead();

	for (;pnode; pnode = (NODE_INFO *)endNodeListNext(pnode))
	{
		updateDownData(pnode, pcHwfsData);
	}
}

static void updateEndNodeDownDataByNetid(NODE_INFO *pnode, const HWFS_DATA *pcHwfsData)
{
	updateDownData(pnode, pcHwfsData);
}


static void routerNodeUpdateEndNodeDownData(const HWFS_FRAME *pcHwfs, u_char ubLen)
{
	NODE_INFO *pnode = (NODE_INFO *)endNodeInListByNetId(pcHwfs->ubaNetID);
	HWFS_DATA stHwfsData = {0x00};
	const RF_NODE_PARAM_CONFIG *prfNode = extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	 
	if (pnode != NULL)
	{
		stHwfsData.ubRSSI = 0xff;
		stHwfsData.devid = pcHwfs->ubDevID;
		stHwfsData.cmd = pcHwfs->ubCMD;
		switch(pcHwfs->ubCMD)
		{
			case HWFS_CMD_CFG_NET:
			{
				const HWFS_CFG_NET_DATA * pNetCfg = (const HWFS_CFG_NET_DATA *)pcHwfs->ubaData;
				//fill data
				XPRINTF((10, "HWFS_CMD_CFG_NET node\r\n"));
				stHwfsData.ubaData[0] = pNetCfg->ubchannel;
				stHwfsData.ubaData[1] = 0x00;
				stHwfsData.ubaData[2] = prfNode->ubHwfsPanId;
				stHwfsData.ubaData[3] = pcHwfs->ubDevID;
				updateEndNodeDownDataByNetid(pnode, (const HWFS_DATA *)&stHwfsData);
				break;
			}
			case HWFS_CMD_QUERY_ROUTER_ADDR:
			case HWFS_CMD_GET_NET_PARAM:
			case HWFS_CMD_FORBID_END_NODE:
			case HWFS_CMD_ALLOW_END_NODE:
				XPRINTF((10, "HWFS_CMD_QUERY_ROUTER_ADDR node\r\n"));
				stHwfsData.ubaData[0] = 0x00;
				stHwfsData.ubaData[1] = 0x00;
				stHwfsData.ubaData[2] = 0xff;
				stHwfsData.ubaData[3] = 0xff;
				updateEndNodeDownDataByNetid(pnode, (const HWFS_DATA *)&stHwfsData);
				break;
			case HWFS_CMD_SEND_MAC:
				XPRINTF((10, "HWFS_CMD_SEND_MAC node\r\n"));
				break;
			case HWFS_CMD_CFG_WIRELESS:
				XPRINTF((10, "HWFS_CMD_CFG_WIRELESS node\r\n"));
				break;
			
			case HWFS_CMD_DEV_TYPE_CHANGE:
				XPRINTF((10, "HWFS_CMD_DEV_TYPE_CHANGE node\r\n"));
				break;
			case HWFS_CMD_SEARCH_NETID_BY_DEVID:
				XPRINTF((10, "HWFS_CMD_SEARCH_NETID_BY_DEVID node\r\n"));
				stHwfsData.ubaData[0] = pcHwfs->ubDevID;
				stHwfsData.ubaData[1] = 0xff;
				stHwfsData.ubaData[2] = 0xff;
				stHwfsData.ubaData[3] = 0xff;
				updateEndNodeDownDataByNetid(pnode, (const HWFS_DATA *)&stHwfsData);
				break;
			case HWFS_CMD_MODIFY_UART_BUARD:
				XPRINTF((10, "HWFS_CMD_MODIFY_UART_BUARD node\r\n"));
				break;
			case HWFS_CMD_TRAN_DATA:
				XPRINTF((10, "HWFS_CMD_TRAN_DATA node\r\n"));
				memcpy(stHwfsData.ubaData, pcHwfs->ubaData, 4);
				updateEndNodeDownDataByNetid(pnode, (const HWFS_DATA *)&stHwfsData);
				break;			
			case HWFS_CMD_BROAD_CAST:
				XPRINTF((10, "HWFS_CMD_BROAD_CAST node\r\n"));
				memcpy(stHwfsData.ubaData, pcHwfs->ubaData, 4);
				updateEndNodeDownDataBraodcastCmd((const HWFS_DATA *)&stHwfsData);
				break;
			case HWFS_CMD_GET_MAC_BY_SHORT_ADDDR:
				XPRINTF((10, "HWFS_CMD_GET_MAC_BY_SHORT_ADDDR node\r\n"));
				break;
			case HWFS_CMD_MODIFY_DOWN_DELAY_TIME:
				XPRINTF((10, "HWFS_CMD_MODIFY_DOWN_DELAY_TIME node\r\n"));
				break;
			default:
				break;
		}
	}
}

/*
	data is hwfs frame : first is frame length, and the second to end is hwfs frame
	[len, hwfs frame]
*/
void appHwfsPacketHandler(process_event_t ev, process_data_t data)
{
	const u_char * pdata = (const u_char *)data;
	u_char ubLen = 0;
	const HWFS_FRAME *pcHwfs = (const HWFS_FRAME *)&pdata[1];
	int nFrameL = -1;
	const RF_NODE_PARAM_CONFIG *pNodeCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	bool isMy = false;

	ubLen = pdata[0];

	if (pcHwfs != NULL)
	{
		isMy = hwfsPacketIsMy((const u_char *)&pdata[1]);
		if (isMy)
		{
			nFrameL = routerNodeProcessPacketAndAckHost(ubaHwfsTXBuf, pcHwfs, ubLen);
		}
		else 
		{
			routerNodeUpdateEndNodeDownData(pcHwfs, ubLen);
		}
	}

	//send data to host
	if (nFrameL > 0)
	{
		MEM_DUMP(10, "->ho", ubaHwfsTXBuf, nFrameL);
		ubLen = hwfsHexFrameToAscii(ubaHwfsTXBufAssic, (const u_char*)ubaHwfsTXBuf, nFrameL-2);
		uart2_send_bytes(ubaHwfsTXBufAssic, ubLen);
		MEM_DUMP(10, "->as", ubaHwfsTXBufAssic, ubLen);
	}
}


/******************************************************************/
PROCESS_THREAD(app_hwfs_process, ev, data)
{

	PROCESS_BEGIN();
	XPRINTF((10,"app_hwfs_process: started\r\n"));
	while(1) 
	{
		PROCESS_YIELD( );
		
		if (ev == app_hwfs_event)
		{
			appHwfsPacketHandler(ev, data);
		}
	}
	PROCESS_END();
}





static struct ringbuf ringuartbuf;
static uint8_t uartbuf[64];

/*----------------------------------------------------------------------*/
#define HWFS_RXBUFS	 	8
#define HWFS_BUF_LEN	32

static uint8_t hwfs_rxbufs[HWFS_RXBUFS][HWFS_BUF_LEN];

#if HWFS_RXBUFS > 1
static volatile int8_t hwfs_first = -1, hwfs_last = 0;
#else   
static const int8_t hwfs_first = 0, hwfs_last = 0;
#endif  

#if HWFS_RXBUFS > 1
#define CLEAN_HWFS_RXBUFS() do{hwfs_first = -1; hwfs_last = 0;}while(0)
#define HWFS_RXBUFS_EMPTY() (hwfs_first == -1)
static int HWFS_RXBUFS_FULL( )
{
	int8_t first_tmp = hwfs_first;
	return first_tmp == hwfs_last;
}
#else 
#define CLEAN_HWFS_RXBUFS( ) (hwfs_rxbufs[0][0] = 0)
#define HWFS_RXBUFS_EMPTY( ) (hwfs_rxbufs[0][0] == 0)
#define HWFS_RXBUFS_FULL( )  (hwfs_rxbufs[0][0] != 0)
#endif 

/*---------------------------------------------------------------------------*/
static int add_to_hwfs_rxbuf(uint8_t *src)
{
	if(HWFS_RXBUFS_FULL()) 
	{
		return 0;
	}

	memcpy(hwfs_rxbufs[hwfs_last], src, src[0]+1);//src[0] is packet lentht bufformat:[1byte:packetlength,packet]

	#if HWFS_RXBUFS > 1
	hwfs_last = (hwfs_last + 1) % HWFS_RXBUFS;
	if(hwfs_first == -1) 
	{
		hwfs_first = 0;
	}
	#endif
	
	memset(src, 0, src[0]+1);//clear buf
	return 1;
}

/*---------------------------------------------------------------------------*/
static int read_from_hwfs_rxbuf(void *dest, unsigned short len)
{
	//PRINTF("first 0   %d\r\n", first);
	u_char  packet_rssi;
	int8_t rssi;
	u_char *pdata = (u_char *)dest;
	if(HWFS_RXBUFS_EMPTY()) 
	{          
		return 0;
	}

	if(hwfs_rxbufs[hwfs_first][0] > len) 
	{   /* Too large packet for dest. */
		len = 0;
	} 
	else 
	{
		len = hwfs_rxbufs[hwfs_first][0];  //frame length
		memcpy(dest, (uint8_t*)&hwfs_rxbufs[hwfs_first][0], len+1);//copy frame to buf bufformat:[1byte:packetlength,packet]
	}

	#if HWFS_RXBUFS > 1
	{
		int first_tmp;
		hwfs_first = (hwfs_first + 1) % HWFS_RXBUFS;
		first_tmp = hwfs_first;
		if(first_tmp == hwfs_last) 
		{
			CLEAN_HWFS_RXBUFS();
		}
	}
	#else
	CLEAN_HWFS_RXBUFS();
	#endif
	return len;
}



PROCESS_THREAD(hwfs_uart_rev_process, ev, data)
{
	static char buf[64];
	static u_char hexbuf[64];
	static struct etimer et_rev_timeout;
	static int ptr;
	int c;
	
	PROCESS_BEGIN();
	XPRINTF((10, "hwfs_uart_rev_process\r\n"));
	process_start(&hwfs_frame_process, NULL);
	process_start(&app_hwfs_process, NULL);
	
	while(1) 
	{
		c = ringbuf_get(&ringuartbuf);
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			XPRINTF((6, "T_O\r\n"));
			ptr = 0;
		}

		if(c == -1) 
		{
			/* Buffer empty, wait for poll */
			PROCESS_YIELD();
		} 
		else 
		{
			if (ptr < 64)
			{
				buf[ptr++] = (uint8_t)c;
				if (ptr==1 && buf[0] == HWGG_HEAD)
				{
					//set timeout 
					//Frame start
					XPRINTF((6, "start\r\n"));
					etimer_set(&et_rev_timeout, 500);					
				}

				//head error
				if (buf[0] != HWGG_HEAD)
				{
					ptr = 0;
				}
				else
				{
					if ((c == HWGG_END))
					{
						MEM_DUMP(0,"ra<-", buf, ptr);
						hexbuf[0]=hwfsAsciiFrameToHex((u_char *)&hexbuf[1], (const u_char *)buf, ptr-2);
						//hexbuf[0] = ptr;
						MEM_DUMP(0,"ra<-", hexbuf, hexbuf[0]+1);
						//if (hwfsPacketCrcRight((const u_char*)&hexbuf[1], hexbuf[0]))
						{
							add_to_hwfs_rxbuf(hexbuf);
							process_poll(&hwfs_frame_process);
						}
						etimer_stop(&et_rev_timeout);
						ptr = 0;
					}					
				}
			}
			else
			{
				ptr = 0;
			}
		}
	}
	PROCESS_END();
}


/******************************************************************/
PROCESS_THREAD(hwfs_frame_process, ev, data)
{
	int len;
	static u_char hwfsBuf[32];

	PROCESS_BEGIN();
	XPRINTF((10,"apphwfs_process: started\r\n"));
	app_hwfs_event = process_alloc_event( );
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		len = read_from_hwfs_rxbuf(hwfsBuf, HWFS_BUF_LEN);
		if(len > 0) 
		{
			XPRINTF((6, "host data\r\n"));
			process_post_synch(&app_hwfs_process, app_hwfs_event, hwfsBuf);
		}
		if(!HWFS_RXBUFS_EMPTY()) 
		{
			process_poll(&hwfs_frame_process);
		}
	}
	PROCESS_END();
}






/*---------------------------------------------------------------------------*/
int serial_uart_input_byte(unsigned char c)
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
	process_poll(&hwfs_uart_rev_process);	
	return 1;
}

void serial_uart_init(void)
{
	ringbuf_init(&ringuartbuf, uartbuf, sizeof(uartbuf));
	process_start(&hwfs_uart_rev_process, NULL);
	Uart_485SetInput(serial_uart_input_byte);
}





