#include "em32lg_config.h"

#include "basictype.h"
#include "sysprintf.h"

#include "hwlh_light.h"
#include "hwlh_z.h"

#include "utility.h"
#include "common.h"





#include "apphwllh.h"

/*
HWLH_Z protocol data hight bit is first, low bit is second
example
u_short data = 0x1234   	-> 0x12  0x34
u_long data = 0x12345678 	-> 0x12  0x34 0x56 0x78
*/

process_event_t event_hwlh_udp;
process_event_t event_hwlh_udp_to_uart;
process_event_t event_hwlh_uart_to_udp;
process_event_t event_rev_msg;
process_event_t event_node_test_pro;
static APP_HWLLH_OBJ appHwlhObj;
static APP_HWLLH_OBJ *pAppObj;


PROCESS(hwlh_event_process_node, "hwlh_event_node");
PROCESS(hwlh_event_process_center, "hwlh_event_center");
PROCESS(hwlh_uart_process, "hwlh_uart_process");

//test leaf node
PROCESS(hwlh_leaf_heart_process, "leaf_heart_process");

static int app_process_hwlh_uart_send_bytes(u_char *pBuf, u_char ubLength);



/*This process is used to test hwlh_z protocol*/
PROCESS(hwlh_z_process, "hwlh_z_process");
PROCESS_THREAD(hwlh_z_process, ev, data)
{
	static struct etimer et;
	static const u_char ubID[3] = {0x12, 0x34, 0x00}; 
	static const u_char ubchPanid[3] = {0x0b, 0xab,0xcd};
	static u_char ubaBuf[64];
	static u_char ubahBuf[64];
	int nFrameHL;
	int nFrameAL;
	
	PROCESS_BEGIN();
	XPRINTF((10, "hwlh_z_process\r\n"));
	
	while(1) 
	{
		etimer_set(&et, 30000);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		//nFrameHL = hwlhFillRxFrame(ubaBuf, ubID, 0x06, (u_char *)ubchPanid, 0);
		nFrameHL = hwlhFillTxFrame(ubaBuf, ubID, 0x00, 0x06, (u_char *)ubchPanid, 0);
		//uart0_send_char('a');
		if (nFrameHL>0)
		{
			MEM_DUMP(0, "hwlh", ubaBuf, nFrameHL);
		}

		nFrameAL = hwlhHexFrameToAscii(ubahBuf, (const u_char *)ubaBuf, nFrameHL-HWLH_FRAME_HEAD_LEN);
		if (nFrameAL > 0)
		{
			MEM_DUMP(0, "h->a", ubahBuf, nFrameAL);
			app_process_hwlh_uart_send_bytes(ubahBuf, nFrameAL);
		}

		nFrameHL = hwlhAsciiFrameToHex(ubaBuf, (const u_char *)ubahBuf, nFrameAL-HWLH_FRAME_HEAD_LEN);
		if (nFrameHL > 0)
		{
			MEM_DUMP(0, "a->h", ubaBuf, nFrameHL);
		}
		
		memset(ubaBuf, 0 , sizeof(ubaBuf));
		memset(ubahBuf, 0, sizeof(ubahBuf));
	}
	PROCESS_END();
}

void init_hwlh_z(void)
{
	process_start(&hwlh_z_process,NULL);
}



APP_HWLLH_OBJ *getObjP(void)
{
	return &appHwlhObj;
}

static int app_process_hwlh_uart_send_bytes(u_char *pBuf, u_char ubLength)
{
	uart2_send_bytes(pBuf, ubLength);
	return 0;
}



//save rfch and panid and devid
static void app_process_hwlh_cmd_cfg(APP_HWLLH_OBJ *pioOBJ)
{
	int nResultRfch = 0;
	int nResultRfAddr = 0;
	RF_NODE_PARAM_CONFIG stRFCfg;
	NODE_ADDR_INFO stNodeAddrInfo;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	const NODE_ADDR_INFO *pcNodeAddr = NULL;
	HWLH_FRAME_HEX_RX_ST *pFrame = NULL; 

	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	pcNodeAddr = (const NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);

	if ((NULL == pioOBJ)||(NULL == pRFCfg) || (NULL == pcNodeAddr))
	{
		return ;
	}

	if (pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART)
	{
		XPRINTF((10, "HWLH<-uart\r\n"));
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pioOBJ->stRxUart.ubaBuf;
	}
	else if (pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP)
	{
		XPRINTF((10, "HWLH<-udp\r\n"));
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pioOBJ->ubaUdpDataBuf;
	}
	
	stRFCfg = *pRFCfg;
	stNodeAddrInfo = *pcNodeAddr;

	//data frame come from udp ,leaf node
	if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP) && (pioOBJ->ubNodeT == NODE_TYPE_LEAF))
	{
		XPRINTF((10, "1frame come from udp ,leaf node\r\n"));
		stRFCfg.ubRFChannel = pFrame->ubaData[0];
		stRFCfg.uwPanID = pFrame->ubaData[2]|(pFrame->ubaData[1]<<8);
		stNodeAddrInfo.ubaNodeAddr[2] = pFrame->ubaAddr[2];	//set devid

		nResultRfch = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		nResultRfAddr = extgdbdevSetDeviceSettingInfoSt(LABLE_ADDR_INFO, 0, &stNodeAddrInfo, sizeof(NODE_ADDR_INFO));

		process_post(&hwlh_event_process_node, event_hwlh_udp, pioOBJ);
	}
	//data frame come from udp, center node
	else if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP) && (pioOBJ->ubNodeT == NODE_TYPE_CENTER))
	{
		XPRINTF((10, "2frame come from udp ,center node\r\n"));
		process_post(&hwlh_event_process_center, event_hwlh_udp_to_uart, pioOBJ);
	}

	//data frame come from uart ,leaf node or center node
	else if (pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART)
	{
		//set center node panid, channel
		if ((pFrame->ubaAddr[0] == 0x00)&&(pFrame->ubaAddr[1] == 0x00)&&(pFrame->ubaAddr[2] == 0x00))
		{
			int nFrameLH = 0;
			int nFrameLA = 0;
			u_char ubDataLen = 0;

			XPRINTF((10, "2frame come from uart ,center node\r\n"));
	
			stRFCfg.ubRFChannel = pFrame->ubaData[0];
			stRFCfg.uwPanID = pFrame->ubaData[2]|(pFrame->ubaData[1]<<8);
			stNodeAddrInfo.ubaNodeAddr[2] = pFrame->ubaAddr[2];	//set devid

			nResultRfch = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
			nResultRfAddr = extgdbdevSetDeviceSettingInfoSt(LABLE_ADDR_INFO, 0, &stNodeAddrInfo, sizeof(NODE_ADDR_INFO));
			
			const u_char *pcnode = (const u_char *)pcNodeAddr->ubaNodeAddr;
			ubDataLen = pioOBJ->stRxUart.ubLen- HWLH_FRAME_HEX_COMMON_LEN;
			nFrameLH = hwlhFillTxFrame(pioOBJ->ubaFrameBuf, pcnode, 0x00, pFrame->ubCMD, pFrame->ubaData, ubDataLen);
			nFrameLA = hwlhHexFrameToAscii(pioOBJ->stRxUart.ubaBuf, (const u_char *)pioOBJ->ubaFrameBuf, nFrameLH-HWLH_TO_ASCII_SUB_LEN);

			MEM_DUMP(10, "th->",pioOBJ->ubaFrameBuf ,nFrameLH);
			if (nFrameLA > 0)
			{
				app_process_hwlh_uart_send_bytes(pioOBJ->stRxUart.ubaBuf, nFrameLA);
				MEM_DUMP(10, "ta->", pioOBJ->ubaFrameBuf, nFrameLA);
				//clear data
				memset(pioOBJ->ubaFrameBuf, 0, nFrameLH);
				memset(&pioOBJ->stRxUart, 0, sizeof(HWLH_SERIAL_RX_HEX_ST));
			}
		}
		else
		{
			if (pioOBJ->ubNodeT == NODE_TYPE_CENTER)
			{
				XPRINTF((10, "3frame come from uart ,center node\r\n"));
				process_post(&hwlh_event_process_center, event_hwlh_udp, pioOBJ);
			}
			else if (pioOBJ->ubNodeT == NODE_TYPE_LEAF)
			{
				XPRINTF((10, "4frame come from uart ,leaf node, not process\r\n"));
			}
		}
	}	
}


static void app_process_hwlh_cmd_tran_data(APP_HWLLH_OBJ *pioOBJ)
{
	//frame come from udp, leaf node
	if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP)&&(pioOBJ->ubNodeT == NODE_TYPE_LEAF))
	{
		process_post(&hwlh_event_process_node, event_hwlh_udp_to_uart, pioOBJ);
	}
	//frame come from udp, center node
	else if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP)&&(pioOBJ->ubNodeT == NODE_TYPE_CENTER))
	{
		process_post(&hwlh_event_process_center, event_hwlh_udp_to_uart, pioOBJ);
	}
	//frame come from uart, center node
	else if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART)&&(pioOBJ->ubNodeT == NODE_TYPE_CENTER))
	{
		process_post(&hwlh_event_process_center, event_hwlh_udp, pioOBJ);
	}

	//frame come from uart, leaf node
	else if ((pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART)&&(pioOBJ->ubNodeT == NODE_TYPE_LEAF))
	{
		//not process
		//clear
		pioOBJ->ubFrameSrc = APP_FRAME_COME_NO;
		pioOBJ->ubFrameType = APP_FRAME_TYPE_NO;
	}


}

static void app_process_hwlh_hl_tran_data(APP_HWLLH_OBJ *pioOBJ)
{
	process_post(&hwlh_event_process_node, event_hwlh_uart_to_udp, pioOBJ);
}






//get rfch and panid only used center node
static void app_process_hwlh_cmd_get_ch_panid(APP_HWLLH_OBJ *pioOBJ)
{
	int nFrameLH = 0;
	int nFrameLA = 0;
	u_char ubaData[3]={0x00};
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;

	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	ubaData[0] = pRFCfg->ubRFChannel;
	ubaData[1] = pRFCfg->uwPanID>>8;
	ubaData[2] = pRFCfg->uwPanID;

	nFrameLH = hwlhFillTxFrame(pioOBJ->ubaUdpDataBuf, NULL, 0x00, HWLH_CMD_GET_CH_PANID, ubaData, 0x03);
	nFrameLA = hwlhHexFrameToAscii(pioOBJ->ubaFrameBuf, (const u_char *)pioOBJ->ubaUdpDataBuf, nFrameLH-HWLH_TO_ASCII_SUB_LEN);

	if (nFrameLA> 0)
	{
		MEM_DUMP(10, "th1-", pioOBJ->ubaUdpDataBuf, nFrameLH);
		app_process_hwlh_uart_send_bytes(pioOBJ->ubaFrameBuf, nFrameLA);
		MEM_DUMP(10, "ta1-", pioOBJ->ubaFrameBuf, nFrameLA);
		//clear data
		memset(pioOBJ->ubaFrameBuf, 0, nFrameLA);
		memset(pioOBJ->ubaUdpDataBuf, 0, nFrameLH);
	}
}



/*
center node 
*/
static void app_process_hwlh_cmd_hl_tran_data(APP_HWLLH_OBJ *pioOBJ)
{
	if (NULL == pioOBJ)
	{
		return;
	}

	//leaf node , frame come from udp, send data to uart
	if ((pioOBJ->ubNodeT == NODE_TYPE_LEAF)&&(pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP))
	{
		XPRINTF((10, "leaf udp\r\n"));
		process_post(&hwlh_event_process_node, event_hwlh_udp_to_uart, pioOBJ);
	}

	//center node, frame com from udp, send data to uart
	else if ((pioOBJ->ubNodeT == NODE_TYPE_CENTER)&&(pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP))
	{
		XPRINTF((10, "center udp\r\n"));
		process_post(&hwlh_event_process_center, event_hwlh_udp_to_uart, pioOBJ);
	}

	//center node, frame com from uart, send data to leaf node by udp
	else if ((pioOBJ->ubNodeT == NODE_TYPE_CENTER)&&(pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART))
	{
		XPRINTF((10, "center uart->udp\r\n"));
		//process_post(&hwlh_event_process_center, event_hwlh_uart_to_udp, pioOBJ);
		process_post(&hwlh_event_process_center, event_hwlh_udp, pioOBJ);

	}

	//leaf node, not process
	else if ((pioOBJ->ubNodeT == NODE_TYPE_LEAF)&&(pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART))
	{
		XPRINTF((10, "leaf uart-not pro\r\n"));
		//process_post(&hwlh_event_process_center, event_hwlh_uart_to_udp, pioOBJ)
	}

}


static void handler_hwlh_z(APP_HWLLH_OBJ *pioOBJ)
{
	HWLH_FRAME_HEX_RX_ST *pFrame = NULL; 

	if (pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UART)
	{
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pioOBJ->stRxUart.ubaBuf;
	}
	else if (pioOBJ->ubFrameSrc == APP_FRAME_COME_FROM_UDP)
	{
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pioOBJ->ubaUdpDataBuf;
	}

	XPRINTF((10, "cmd is %02x\r\n", pFrame->ubCMD));
	if (NULL != pFrame)
	{
		switch (pFrame->ubCMD)
		{
			case HWLH_CMD_CFG:
				XPRINTF((10, "HWLH_CMD_CFG \r\n"));
				app_process_hwlh_cmd_cfg(pioOBJ);
				break;
			case HWLH_CMD_GET_PARENT_ADDR:
				break;
			case HWLH_CMD_FORBID_DST_HEART:
				break;
			case HWLH_CMD_START_DST_HEART:
				break;
			case HWLH_CMD_WIRESS_SEND_MAC:
				break;
			case HWLH_CMD_WIRESS_CFG:
				break;
			case HWLH_CMD_GET_NET_CFG:
				break;
			case HWLH_CMD_CHANGE_NODE_TYPE:
				break;
			case HWLH_CMD_SEARCH_NETID_BY_DEVID:
				break;
			case HWLH_CMD_MODIFY_UART_BUADRATE:
				break;
			case HWLH_CMD_TRAN_DATA:
				XPRINTF((10, "HWLH_CMD_TRAN_DATA \r\n"));
				app_process_hwlh_cmd_tran_data(pioOBJ);
				break;
			case HWLH_CMD_BROADCAST_RST:
				break;
			case HWLH_CMD_REQUST_RST:
				break;
			case HWLH_CMD_GET_CH_PANID:
				XPRINTF((10, "HWLH_CMD_GET_CH_PANID \r\n"));
				app_process_hwlh_cmd_get_ch_panid(pioOBJ);
				break;
			case HWLH_CMD_HL_TRAN_DATA:
				XPRINTF((10, "HWLH_CMD_HL_TRAN_DATA \r\n"));
				app_process_hwlh_cmd_hl_tran_data(pioOBJ);
				break;
			case HWLH_CMD_HL_ACK_DATA:
				break;
			case HWLH_CMD_GET_NODE_MAC:
				break;
			default:
				break;
		}
	}
}





//this funtion is process data receive from udp
static void app_hwlh_process_udp_data(APP_HWLLH_OBJ *pioOBJ, const process_data_t data, u_short uwDataL)
{

	if (NULL == pioOBJ || NULL==data)
	{
		return;
	}
	
	memcpy(pioOBJ->ubaUdpDataBuf, data, uwDataL);

	//if the frame is not HWHL_Z frame, not process
	if (pioOBJ->ubaUdpDataBuf[0] != HWLH_SQI)
	{
		memset(pioOBJ->ubaUdpDataBuf, 0, uwDataL);
		pioOBJ->ubUdpLen = 0;
		return;
	}
	
	pioOBJ->ubUdpLen = uwDataL;
	pioOBJ->ubFrameType = APP_FRAME_TYPE_HWLH_Z;
	pioOBJ->ubFrameSrc = APP_FRAME_COME_FROM_UDP;
	handler_hwlh_z(pioOBJ);
}


void app_hwlh_process_receive_data( const process_data_t data, u_short uwDataL)
{
	app_hwlh_process_udp_data(pAppObj, data, uwDataL);	
}



static void app_process_hwlh_generate_goal_ipaddr(uip_ipaddr_t* pioIpAddr, uip_lladdr_t* p_lladdr, struct rpl_dag *p_rpl_dag)
{
	//uip_ip6addr(pIpAddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	#if 1
	uip_ip6addr(pioIpAddr, 	p_rpl_dag->dag_id.u16[0], p_rpl_dag->dag_id.u16[1], p_rpl_dag->dag_id.u16[2], 
						 	p_rpl_dag->dag_id.u16[3], p_rpl_dag->dag_id.u16[4], p_rpl_dag->dag_id.u16[5],
						 	p_rpl_dag->dag_id.u16[6], p_rpl_dag->dag_id.u16[7]);
	uip_ds6_set_addr_iid(pioIpAddr, p_lladdr);
	#endif
}


static int findGoalIpInRouteTable(uip_ipaddr_t* pGoalIp)
{
	int nResult = -1;
	uip_ds6_route_t *prt = NULL;
	int i = 0;

	if (pGoalIp == NULL)
		return nResult;

	//XPRINTF((10, "1 in routable1\r\n"));
	prt = uip_ds6_route_head();
	//XPRINTF((10, "2 in routable1\r\n"));
	if (prt != NULL)
	{
		for(i = 0; i < uip_ds6_route_num_routes(); i++) 
		{
			//MEM_DUMP(8, "ROUT", prt->ipaddr.u8, 16);
			if (mem_cmp(pGoalIp, prt->ipaddr.u8, 16) == 0)
			{
				XPRINTF((10, "This goal ip in routable\r\n"));
				nResult = 0;
				break;
			}
			prt = uip_ds6_route_next(prt);
		}
	}

	if (nResult != 0)
	{
		XPRINTF((10, "This goal ip not in routable\r\n"));
	}
	return nResult;
}




//leaf node event process
static void app_hwlh_event_node_event_handler(process_event_t ev, process_data_t data)
{
	APP_HWLLH_OBJ *pObj = (APP_HWLLH_OBJ *)data;
	const NODE_ADDR_INFO *pcNode = NULL;
	struct rpl_dag *p_rpl_dag=NULL; 
	struct simple_udp_connection* p_udp_connection =NULL;
	uip_ipaddr_t ipaddr_t;
	
	pcNode = (const NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);

	p_rpl_dag = rpl_get_any_dag( );
	extern struct simple_udp_connection* get_leafnode_unicast_conn(void);
	p_udp_connection = get_leafnode_unicast_conn( );
	

	if ((NULL == p_rpl_dag)||(NULL == p_udp_connection))
		return;

	//send data to center node 
	if ((ev == event_hwlh_udp)&&(data != NULL))
	{
		HWLH_FRAME_HEX_RX_ST *pFrame = NULL;
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pObj->ubaUdpDataBuf;
		
		if (pFrame->ubCMD == HWLH_CMD_CFG)
		{
			int nFrameL = 0;
			const u_char *pcBuf = (const u_char *)pFrame->ubaData;
			u_char ubDataLen = pObj->ubUdpLen - HWLH_FRAME_HEX_COMMON_LEN;
			nFrameL = hwlhFillTxFrame(pObj->ubaFrameBuf, pcNode->ubaNodeAddr, 0x00, HWLH_CMD_TRAN_DATA, pcBuf, ubDataLen);

			MEM_DUMP(6, "->w1", pObj->ubaUdpDataBuf, pObj->ubUdpLen);
			simple_udp_sendto(p_udp_connection,  pObj->ubaFrameBuf, nFrameL, &p_rpl_dag->dag_id);
		}
		
		//clear udp data that receive
		memset(pObj->ubaUdpDataBuf, 0, pObj->ubUdpLen);
		pObj->ubUdpLen = 0;
	}

	//send data to node uart
	else if ((ev == event_hwlh_udp_to_uart)&&(data != NULL))
	{
		HWLH_FRAME_HEX_RX_ST *pFrame = NULL;
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pObj->ubaUdpDataBuf;

		u_char ubDataLen = 0;
		int nFrameHL = 0;
		#if 0
		nFrameL = hwlhAppHexFrameToAscii(pObj->ubaFrameBuf, (const u_char *)pFrame->ubaData, pObj->ubUdpLen-HWLH_FRAME_HEX_COMMON_LEN);
		if (nFrameL > 0)
		{
			app_process_hwlh_uart_send_bytes(pObj->ubaFrameBuf, nFrameL);
		}
		#else
		if (pObj->ubUdpLen > HWLH_FRAME_HEX_COMMON_LEN)
		{
			ubDataLen = pObj->ubUdpLen-HWLH_FRAME_HEX_COMMON_LEN;
			MEM_DUMP(10, "uhld", pFrame->ubaData, ubDataLen);
			nFrameHL = hwlhFillHLFrame(pObj->ubaFrameBuf, (const u_char *) pFrame->ubaData, ubDataLen);

			if (nFrameHL > 0)
			{
				app_process_hwlh_uart_send_bytes(pObj->ubaFrameBuf,  nFrameHL);
				MEM_DUMP(10, "uhld", pObj->ubaFrameBuf, nFrameHL);
			}
		}
		
		#endif
		//clear udp data that receive
		memset(pObj->ubaUdpDataBuf, 0, pObj->ubUdpLen);
		memset(pObj->ubaFrameBuf, 0, nFrameHL);
		pObj->ubUdpLen = 0;
		pObj->ubFrameSrc = APP_FRAME_COME_NO;
		pObj->ubFrameType = APP_FRAME_TYPE_NO;
	}

	//send data to center node ,udp
	else if ((ev == event_hwlh_uart_to_udp)&&(data != NULL))
	{
		int nFrameL = 0;
		const u_char *pcBuf = (const u_char *)&pObj->stRxUart.ubaBuf[1];
		u_char ubDataLen = pObj->stRxUart.ubLen - 4;
		nFrameL = hwlhFillTxFrame(pObj->ubaFrameBuf, pcNode->ubaNodeAddr, 0x00, HWLH_CMD_HL_ACK_DATA, pcBuf, ubDataLen);
		simple_udp_sendto(p_udp_connection,  pObj->ubaFrameBuf, nFrameL, &p_rpl_dag->dag_id);
	}

	else if ((ev == event_node_test_pro)&&(data != NULL))
	{
		HWLH_SERIAL_RX_HEX_ST *pTest = (HWLH_SERIAL_RX_HEX_ST *)data;
		simple_udp_sendto(p_udp_connection,  pTest->ubaBuf, pTest->ubLen, &p_rpl_dag->dag_id);
	}
}


//center event process
static void app_hwlh_event_center_event_handler(process_event_t ev, process_data_t data)
{
	APP_HWLLH_OBJ *pObj = (APP_HWLLH_OBJ *)data;
	HWLH_FRAME_HEX_RX_ST *pFrame = NULL;
	struct rpl_dag *p_rpl_dag=NULL; 
	struct simple_udp_connection* p_udp_connection =NULL;
	uip_ipaddr_t ipaddr_t;
	u_char ubaNode[8] = NODE_DEFAULT_ADDR;
	//const NODE_ADDR_INFO *pcNodeAddr = NULL;
	//pcNodeAddr = (const NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);

	if (pObj->ubFrameSrc == APP_FRAME_COME_FROM_UART)
	{
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pObj->stRxUart.ubaBuf;
	}
	else if (pObj->ubFrameSrc == APP_FRAME_COME_FROM_UDP)
	{
		pFrame = (HWLH_FRAME_HEX_RX_ST *)pObj->ubaUdpDataBuf;
	}
	//memcpy(ubaNode, pcNodeAddr->ubaNodeAddr, 8);
	//clean devid
	ubaNode[2] = 0x00;

	p_rpl_dag = rpl_get_any_dag( );
	extern struct simple_udp_connection* get_unicast_conn(void);
	p_udp_connection = get_unicast_conn( );
	
	if ((NULL == p_rpl_dag)||(NULL == p_udp_connection))
		return;

	//send data to leaf node receive from uart
	if ((ev == event_hwlh_udp)&&(data != NULL))
	{
		int nResult = -1;
		ubaNode[0] = pFrame->ubaAddr[0];
		ubaNode[1] = pFrame->ubaAddr[1];
		app_process_hwlh_generate_goal_ipaddr(&ipaddr_t, (uip_lladdr_t *)ubaNode, p_rpl_dag);
		MEM_DUMP(8, "LFAD", &ipaddr_t, 16);
		nResult = findGoalIpInRouteTable(&ipaddr_t);
		if (nResult == 0)
		{
			//etimer_set(&et_timecheck, CLOCK_SECOND * LEAF_NODE_WAIT_ACK_TIME);	//set time wait for leaf node ack.	
			XPRINTF((8, "->o udp leaf\r\n"));		
			simple_udp_sendto(p_udp_connection, pObj->stRxUart.ubaBuf, pObj->stRxUart.ubLen, &ipaddr_t);
		}

		//clear
		memset(pObj->stRxUart.ubaBuf, 0, pObj->stRxUart.ubLen);
		pObj->ubFrameSrc = APP_FRAME_COME_NO;
		pObj->ubFrameType = APP_FRAME_TYPE_NO;
	}

	//data come frome node by wireless
	else if ((ev == event_hwlh_udp_to_uart)&&(data != NULL))
	{
		int nFrameL = 0;
		MEM_DUMP(10, "U<-Z", pObj->ubaUdpDataBuf, pObj->ubUdpLen);
		nFrameL = hwlhHexFrameToAscii(pObj->ubaFrameBuf, (const u_char *)pObj->ubaUdpDataBuf, pObj->ubUdpLen-HWLH_TO_ASCII_SUB_LEN);
		MEM_DUMP(10, "h->a", pObj->ubaFrameBuf, nFrameL);
		if (nFrameL > 0)
		{
			app_process_hwlh_uart_send_bytes(pObj->ubaFrameBuf, nFrameL);
		}

		//clear udp data that receive
		memset(pObj->ubaUdpDataBuf, 0, pObj->ubUdpLen);
		pObj->ubUdpLen = 0;
	}
}


//uart process
static void app_hwlh_uart_handler(APP_HWLLH_OBJ *pObj,process_event_t ev, process_data_t data)
{
	if ((ev == event_rev_msg) && (NULL != data))
	{
		pObj->stRxUart = *(HWLH_SERIAL_RX_HEX_ST *)data; //copy data from uart buf
		//if the frame is not hwlh_z and hl frame format, not process
		if ((pObj->stRxUart.ubaBuf[0] != HWLH_SQI))
		{
			if (pObj->stRxUart.ubaBuf[0] != HL_HEARER)
			{
				XPRINTF((10, "Frame error\r\n"));
				memset(pObj->stRxUart.ubaBuf, 0, pObj->stRxUart.ubLen);
				return ;
			}
		}
		MEM_DUMP(10, "uf<-", pObj->stRxUart.ubaBuf, pObj->stRxUart.ubLen);
		
		pObj->ubFrameSrc = APP_FRAME_COME_FROM_UART;

		//hwlh_z frame
		if (pObj->stRxUart.ubaBuf[0] == HWLH_SQI)
		{
			pObj->ubFrameType = APP_FRAME_TYPE_HWLH_Z;
			//pObj->pRxFrame = (HWLH_FRAME_HEX_RX_ST*)pHwlhS->ubaBuf;
			XPRINTF((10, "Frame HWLH_Z\r\n"));
		}
		//lh frame  only come from leaf node
		else if (pObj->stRxUart.ubaBuf[0] == HL_HEARER)
		{
			pObj->ubFrameType = APP_FRAME_TYPE_HWLH_HL;
			XPRINTF((10, "Frame HL\r\n"));
		}

		//leaf node hwlh_z frame, set node param
		if ((pObj->ubNodeT == NODE_TYPE_LEAF)&&(pObj->ubFrameType == APP_FRAME_TYPE_HWLH_Z))
		{
			XPRINTF((10, "LEAF SET\r\n"));
			handler_hwlh_z(pObj);
			//clear data
			pObj->ubFrameType = APP_FRAME_TYPE_NO;
			pObj->ubFrameSrc = APP_FRAME_COME_NO;
		}

		//leaf node hl frame, send to center node
		else if ((pObj->ubNodeT == NODE_TYPE_LEAF)&&(pObj->ubFrameType == APP_FRAME_TYPE_HWLH_HL))
		{
			XPRINTF((10, "LEAF tran\r\n"));
			app_process_hwlh_hl_tran_data(pObj);
		}

		//center node hwlh_z frame 
		else if ((pObj->ubNodeT == NODE_TYPE_CENTER)&&(pAppObj->ubFrameType == APP_FRAME_TYPE_HWLH_Z))
		{
			XPRINTF((10, "center hwlh\r\n"));
			handler_hwlh_z(pObj);
		}

		//not process now, not surport now
		else if ((pObj->ubNodeT == NODE_TYPE_CENTER)&&(pObj->ubFrameType == APP_FRAME_TYPE_HWLH_HL))
		{
			XPRINTF((10, "center hl not pro\r\n"));
			pObj->ubFrameSrc = 0x00;
			pObj->ubFrameType = 0x00;
		}
	}
}

//leaf node event process
PROCESS_THREAD(hwlh_event_process_node, ev, data)
{
	PROCESS_BEGIN();
	XPRINTF((10, "hwlh_event_process_node\r\n"));
	event_hwlh_udp = process_alloc_event( );
	event_hwlh_udp_to_uart = process_alloc_event( );
	event_hwlh_uart_to_udp = process_alloc_event( );
	event_node_test_pro = process_alloc_event( );
	
	while(1) 
	{
		PROCESS_YIELD( );
		//XPRINTF((10, "hwlh_event_process_node\r\n"));
		app_hwlh_event_node_event_handler(ev, data);
	}
	PROCESS_END();
}


//leaf node event process
PROCESS_THREAD(hwlh_event_process_center, ev, data)
{
	PROCESS_BEGIN();
	XPRINTF((10, "hwlh_event_process_node\r\n"));
	event_hwlh_udp = process_alloc_event( );
	event_hwlh_udp_to_uart = process_alloc_event( );
	event_hwlh_uart_to_udp = process_alloc_event( );
	
	while(1) 
	{
		PROCESS_YIELD( );
		app_hwlh_event_center_event_handler(ev, data);
	}
	PROCESS_END();
}

//
PROCESS_THREAD(hwlh_uart_process, ev, data)
{
	PROCESS_BEGIN( );
	XPRINTF((10, "hwlh_uart_process_node\r\n"));
	event_rev_msg = process_alloc_event( );
	while(1) 
	{
		PROCESS_YIELD( );
		app_hwlh_uart_handler(pAppObj, ev, data);
	}
	PROCESS_END();
}



PROCESS_THREAD(hwlh_leaf_heart_process, ev, data)
{
	static struct etimer et;
	//static u_char ubaBuf[32];
	static HWLH_SERIAL_RX_HEX_ST stNodeTest;
	static u_char ubaID[8] = {0x00};
	int nFrameHL;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	const NODE_ADDR_INFO *pcNodeAddr = NULL;
	struct rpl_dag *p_rpl_dag=NULL; 

	PROCESS_BEGIN();
	XPRINTF((10, "hwlh_leaf_heart_process\r\n"));
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	pcNodeAddr = (const NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	memcpy(ubaID, pcNodeAddr->ubaNodeAddr, 8);
	ubaID[2] = 0x00;
	etimer_set(&et, 30000);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	while(1) 
	{
		etimer_set(&et, 30000);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		nFrameHL = hwlhFillTxFrame(stNodeTest.ubaBuf, (const u_char *)ubaID, 0x00, 0x06, NULL, 0);
		if (nFrameHL>0)
		{
			MEM_DUMP(0, "hw-h", stNodeTest.ubaBuf, nFrameHL);
			stNodeTest.ubLen = nFrameHL;
			p_rpl_dag = rpl_get_any_dag( );
			if (p_rpl_dag != NULL)
			{
				process_post(&hwlh_event_process_node, event_node_test_pro,&stNodeTest);
			}
		}
	}
	PROCESS_END();
}



void app_hwlh_init(void)
{
	const RF_NODE_PARAM_CONFIG *pcdevCfg = NULL;
	pAppObj = getObjP( );
	
	pcdevCfg = (const RF_NODE_PARAM_CONFIG*)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);

	pAppObj->ubNodeT = pcdevCfg->ubNodeType;
	process_start(&hwlh_uart_process, NULL);
	if (pAppObj->ubNodeT == NODE_TYPE_LEAF)
	{
		process_start(&hwlh_event_process_node, NULL);
		process_start(&hwlh_leaf_heart_process, NULL);
	}
	else if (pAppObj->ubNodeT == NODE_TYPE_CENTER)
	{
		process_start(&hwlh_event_process_center, NULL);
	}
}

