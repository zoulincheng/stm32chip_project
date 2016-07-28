#ifndef _APPHWLLH_H
#define _APPHWLLH_H


#define APP_FRAME_COME_FROM_UDP		0x01
#define APP_FRAME_COME_FROM_UART	0x02
#define APP_FRAME_COME_NO			0x00

#define APP_FRAME_TYPE_HWLH_Z		0x01
#define APP_FRAME_TYPE_HWLH_HL		0x02
#define APP_FRAME_TYPE_NO			0x00


typedef struct _app_hwllh_obj
{
	u_char ubUdpLen;
	u_char ubFrameSrc;
	u_char ubFrameType;
	u_char ubNodeT;
	u_char ubaUdpDataBuf[48];
	u_char ubaFrameBuf[48];

	//struct simple_udp_connection* p_udp_connection;
	//struct rpl_dag *p_rpl_dag;
	//uip_ipaddr_t ipaddr_t;

	HWLH_SERIAL_RX_HEX_ST stRxUart;
	//HWLH_FRAME_HEX_RX_ST *pRxFrame;
	//HWLH_FRAME_HEX_TX_ST *pTxFrame;
	//HL_FRAME_U *pHLFrame;
}APP_HWLLH_OBJ;




#endif
