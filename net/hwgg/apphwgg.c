#include "contiki.h"
#include "basictype.h"
#include "hwgg.h"
#include "hwfs.h"
#include "sysprintf.h"
#include "common.h"
#include <string.h>
#include "apphwgg.h"

const u_char broadcastAddrFE[] = {0xfe, 0xff};
const u_char broadcastAddrFF[] = {0xff, 0xff};
const u_char routerAddr[2] = {0x00};
const u_char ubPanID = 0x00;
static u_char ubaHwggBuf[32] = {0x00};
static u_char ubaHwggTx[128] = {0x00};
static u_char ubaHwfsToHostHexBuf[32] = {0x00};
static u_char ubaHwfsToHostAssicBuf[64] = {0x00};

#define APP_HWGG_RSSI_INDEX		31

process_event_t app_hwgg_event;

PROCESS(app_hwgg_process, "app_hwgg");

static u_short extractPacketCrc(const u_char* pciFrame)
{
	u_short pktCrc = 0;
	const HWGG_FRAME *pcHwgg =(const HWGG_FRAME *) pciFrame;
	u_char ubTotalLen = pcHwgg->ubLen + HWGG_FRAME_HEAD_CRC_END_BYTES;

	pktCrc = (pciFrame[ubTotalLen-3]<<8)|(pciFrame[ubTotalLen-2]);

	return pktCrc;
}

static bool packetCrcRight(const u_char* pciFrame)
{
	const HWGG_FRAME *pcHwgg = NULL;
	u_short uwCrc = 0;
	u_short uwPacketCrc = 0;

	if (pciFrame == NULL)
		return false;
	pcHwgg = (const HWGG_FRAME *)pciFrame;
	uwPacketCrc = extractPacketCrc(pciFrame);
	uwCrc = cyg_crc16((const unsigned char*)&pcHwgg->ubLen, pcHwgg->ubLen);

	if (uwCrc == uwPacketCrc)
		return true;
	return false;
}

static bool packetDstAddrBelongToMe(const u_char* pciFrame)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME *)pciFrame;
	if (mem_cmp(pcHwgg->ubaDstAddr, routerAddr, HWGG_NETADDR_LEN) == 0)
		return true;
	return false;
}

bool isBroadNetApplyPacket(const u_char* pciFrame)
{
	const HWGG_FRAME * pcHwgg = NULL;
	if (pciFrame == NULL)
		return false;
	pcHwgg = (const HWGG_FRAME*)pciFrame;

	if (mem_cmp(pcHwgg->ubaDstAddr, broadcastAddrFE,HWGG_NETADDR_LEN) == 0 || mem_cmp(pcHwgg, broadcastAddrFF, HWGG_NETADDR_LEN) == 0)
		return true;
	return false;
}


bool isMyPacket(const u_char * pciFrame)
{
	bool pktCrc = packetCrcRight(pciFrame);
	bool isBroadcast = isBroadNetApplyPacket(pciFrame);
	bool isMyData = packetDstAddrBelongToMe(pciFrame);

	if (pktCrc && (isBroadcast || isMyData))
		return true;
	return false;
}


static int hwgg_cmd_antcollision_process(u_char *pioFrame, const u_char * pciHwgg)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME*)pciHwgg;
	int nFrameL = -1;
	u_char ubSeq = pcHwgg->ubSeq;
	nFrameL = hwggFillFrame(pioFrame, ubPanID, pcHwgg->ubaSrcAddr, routerAddr, ubSeq++, HWGG_CMD_ANTCOLLISION_ACK, NULL, 0);
	return nFrameL;
}

static int hwgg_cmd_net_in_process(u_char *pioFrame, const u_char *pciHwgg)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME*)pciHwgg;
	int nFrameL = -1;
	u_char ubaLayer[1] = {0x00};
	u_char ubSeq = pcHwgg->ubSeq;
	nFrameL = hwggFillFrame(pioFrame, ubPanID, pcHwgg->ubaSrcAddr, routerAddr, ubSeq, HWGG_CMD_NET_IN_ACK, (const u_char*)ubaLayer, 1);
	return nFrameL;
}


/*
cmd 
source layer 00
tran count  00
mac			4
result		1
redata		2
*/
static int hwgg_cmd_select_router_process(u_char *pioFrame, const u_char *pciHwgg, u_char ubnetFlag, const u_char * pcNetID)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME*)pciHwgg;
	NODE_INFO node_info;
	HWGG_SELECT *pHwggSelect;
	int nFrameL = -1;
	u_char ubaData[16] = {0x00};
	u_char ubDataL = 0;
	u_char ubSeq = pcHwgg->ubSeq;
	
	pHwggSelect = (HWGG_SELECT *)pcHwgg->ubaData;
	
	ubaData[ubDataL++] = 0x00;
	ubaData[ubDataL++] = 0x00;
	memcpy(&ubaData[2], pHwggSelect->ubaMAC, HWGG_NODE_MAC_LEN);
	ubDataL += HWGG_NODE_MAC_LEN;
	ubaData[ubDataL++] = ubnetFlag;
	memcpy(&ubaData[ubDataL], pcNetID, HWGG_NETADDR_LEN);
	ubDataL += HWGG_NETADDR_LEN;
	
	nFrameL = hwggFillFrame(pioFrame, ubPanID, pcHwgg->ubaSrcAddr, routerAddr, ubSeq, HWGG_CMD_SELECT_ROUTER_ACK, (const u_char *)ubaData, ubDataL);

	memset(&node_info, 0x00, sizeof(NODE_INFO));
	memcpy(node_info.ubaHWGGMacAddr, pHwggSelect->ubaMAC, HWGG_NODE_MAC_LEN);
	memcpy(node_info.ubaHWGGNetAddr, pcHwgg->ubaSrcAddr, HWGG_NETADDR_LEN);
	//node_info.ubNetState = 0;
	//node_info.ubHeartBeatCount = 1;
	node_info.lastRevPacketTime = clock_seconds( );
	
	endNodeListadd((const NODE_INFO *)&node_info);
	return nFrameL;	
}

static int hwgg_cmd_ack_node_process(u_char *pioFrame, const u_char *pciHwgg)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME*)pciHwgg;
	NODE_INFO *pnodeInfo = NULL;

	pnodeInfo = (NODE_INFO *)endNodeInListByNetId((const u_char*)pcHwgg->ubaSrcAddr);

	if (pnodeInfo != NULL)
	{
		pnodeInfo->ubNetState  = 1;
		pnodeInfo->lastRevPacketTime = clock_seconds( );
	}

	return -1;
}


//update node devid
static void updateNodeDevid(NODE_INFO *pnode, const HWFS_UP_LOAD *pHwfsUp)
{
	if (pHwfsUp->cmd == HWFS_CMD_TRAN_DATA)
	{
		if (pnode->ubHwfsDevID != pHwfsUp->devid)
		{
			pnode->ubHwfsDevID = pHwfsUp->devid;
		}
	}
}


static void updateEndNodeUpData(NODE_INFO *pnodeInfo, const HWGG_FRAME *pcHwgg)
{
	HWFS_UP_LOAD *pHwfsUp = NULL;
	u_char ubDataHexL = 0;
	u_char ubDataAssicL = 0;
	u_char ubSubL = 0;
	u_char ubDataL = 0;
	const u_char *pFrame = (const u_char *)pcHwgg;
	u_char ubRssi = pFrame[APP_HWGG_RSSI_INDEX];

	int nFrameL = -1;

	pHwfsUp = (HWFS_UP_LOAD *)pcHwgg->ubaData;
	ubDataL = pcHwgg->ubLen + HWGG_FRAME_HEAD_CRC_END_BYTES - HWGG_FRAME_FIX_LEN;
	ubSubL = (u_char *)&pHwfsUp->ubTxCount - (u_char *)pHwfsUp + 1;
	MEM_DUMP(8, "<-up", pHwfsUp, ubDataL);

	//update end node up date, receive packet time, net state, heart beat count
	pnodeInfo->ubLenUp = ubDataL-ubSubL;
	memcpy(pnodeInfo->ubaDataUp, &pHwfsUp->ubRSSI, ubDataL-ubSubL);
	pnodeInfo->lastRevPacketTime = clock_seconds( );
	pnodeInfo->ubHeartBeatCount += 1;
	updateNodeDevid(pnodeInfo, (const HWFS_UP_LOAD*)pHwfsUp);

	nFrameL = hwfsFillToHostFrame(ubaHwfsToHostHexBuf, pnodeInfo->ubaHWGGNetAddr, pHwfsUp->devid, ubRssi, pHwfsUp->cmd, (const u_char*)&pHwfsUp->ubRSSI, 7);
	ubDataAssicL = hwfsHexFrameToAscii(ubaHwfsToHostAssicBuf, (const u_char*)ubaHwfsToHostHexBuf, nFrameL-2);
	MEM_DUMP(8,"hex", ubaHwfsToHostHexBuf, nFrameL);
	MEM_DUMP(8,"asic", ubaHwfsToHostAssicBuf, ubDataAssicL);
	uart2_send_bytes(ubaHwfsToHostAssicBuf, ubDataAssicL);
}




static int hwgg_cmd_heart_data_up_process(u_char *pioFrame, const u_char *pciHwgg)
{
	const HWGG_FRAME *pcHwgg = (const HWGG_FRAME*)pciHwgg;
	NODE_INFO *pnodeInfo = NULL;
	int nFrameL = -1;

	pnodeInfo =(NODE_INFO *)endNodeInListByNetId(pcHwgg->ubaSrcAddr);

	if (pnodeInfo != NULL)
	{
		if (pnodeInfo->ubLenDown)
		{
			const u_char *pData = pnodeInfo->ubaDataDown;
			HWFS_UP_LOAD stDownData={0x00};
			u_char ubLen = pnodeInfo->ubLenDown;
			//MEM_DUMP(10, "->n", pnodeInfo->ubaDataDown, pnodeInfo->ubLenDown);
			memcpy(&stDownData.ubRSSI, pData, ubLen);
			MEM_DUMP(10, "->d", &stDownData, ubLen+2);
			nFrameL = hwggFillFrame(pioFrame, ubPanID, pcHwgg->ubaSrcAddr, routerAddr, pcHwgg->ubSeq, HWGG_CMD_DATA_DOWN, (u_char const*)&stDownData, ubLen+2);
			memset(pnodeInfo->ubaDataDown, 0x00, pnodeInfo->ubLenDown);
			pnodeInfo->ubLenDown = 0;
		}
		else
		{
			nFrameL = hwggFillFrame(pioFrame, ubPanID, pcHwgg->ubaSrcAddr, routerAddr, pcHwgg->ubSeq, HWGG_CMD_ACK_ROUTER, NULL, 0);
		}
		//update end node up data
		updateEndNodeUpData(pnodeInfo, pcHwgg);
	}
	return nFrameL;
}

void appHwggPacketHandler(process_event_t ev, process_data_t data)
{
	HWGG_FRAME * pHwgg = NULL;
	pHwgg = (HWGG_FRAME *)data;
	int nFrameL = -1;

	switch(pHwgg->ubCmd)
	{
		case HWGG_CMD_ANTCOLLISION:
			
			break;
		case HWGG_CMD_ANTCOLLISION_ACK:
			XPRINTF((8, "HWGG_CMD_ANTCOLLISION_ACK\r\n"));
			nFrameL = hwgg_cmd_antcollision_process(ubaHwggBuf, (const u_char*)data);
			break;
		case HWGG_CMD_NET_IN:
			XPRINTF((8, "HWGG_CMD_NET_IN\r\n"));
			nFrameL = hwgg_cmd_net_in_process(ubaHwggBuf, (const u_char*)data);
			break;
		case HWGG_CMD_NET_IN_ACK:
			break;
		case HWGG_CMD_SELECT_ROUTER:
		case HWGG_CMD_SELECT_ROUTER_S:
		{
			u_char ubnetFlag = HWGG_FLAG_NET_SUCCESS;
			u_char ubaNetId[2] = {0x00,0x00};
			nFrameL = hwgg_cmd_select_router_process(ubaHwggBuf, (const u_char*)data, ubnetFlag, (const u_char*)ubaNetId);
			XPRINTF((8, "HWGG_CMD_SELECT_ROUTER\r\n"));
			break;
		}
		case HWGG_CMD_SELECT_ROUTER_ACK:
			break;
		case HWGG_CMD_HEART_DATA_UP:
		case HWGG_CMD_HEART_DATA_UP_S:
			XPRINTF((8, "HWGG_CMD_HEART_DATA_UP\r\n"));
			nFrameL = hwgg_cmd_heart_data_up_process(ubaHwggBuf, (const u_char*)data);
			break;
		case HWGG_CMD_DATA_DOWN:
			break;
		case HWGG_CMD_BROAD_DATA:
			break;
		case HWGG_CMD_ACK_ROUTER:
			XPRINTF((8, "HWGG_CMD_ACK_ROUTER\r\n"));	
			nFrameL = hwgg_cmd_ack_node_process(ubaHwggBuf, (const u_char*)data);
			break;
		case HWGG_CMD_ACK_NODE:
			XPRINTF((8, "HWGG_CMD_ACK_NODE\r\n"));
			nFrameL = hwgg_cmd_ack_node_process(ubaHwggBuf, (const u_char*)data);
			break;
		default:
			break;
	}

	if (nFrameL > 0)
	{
		memcpy(&ubaHwggTx[1], ubaHwggBuf, nFrameL);
		ubaHwggTx[0] = nFrameL+1;
		si446xRadioSendFixLenDataHwgg((const u_char *)ubaHwggTx);
		MEM_DUMP(0, "->en", ubaHwggTx, ubaHwggTx[0]);
	}
}


/******************************************************************/
PROCESS_THREAD(app_hwgg_process, ev, data)
{
	PROCESS_BEGIN();
	XPRINTF((10,"app_hwgg_process: started\r\n"));
	app_hwgg_event = process_alloc_event( );
	endNodeListInit( );
	while(1) 
	{
		PROCESS_YIELD( );
		if (ev == app_hwgg_event)
		{
			appHwggPacketHandler(ev, data);
		}
	}
	PROCESS_END();
}





