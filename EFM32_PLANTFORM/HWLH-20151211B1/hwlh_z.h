#ifndef _HWLH_Z_H
#define _HWLH_Z_H


#define HWLH_SQI	0x7e
#define HWLH_EQI	0x0d

#define HWLH_DATA_START				0x01
#define HWLH_TO_ASCII_SUB_LEN		0x02
#define HWLH_FRAME_HEAD_LEN			0x02
#define HWLH_FRAME_HEX_COMMON_LEN 	0x08
#define HWLH_FRAME_ERROR			(-1)
#define HWLH_ADDR_LEN				0x03
#define HWLH_CAC_CRC_SUB_LEN		0x01



#define HWLH_CMD_CFG				0x00
#define HWLH_CMD_GET_PARENT_ADDR	0x01
#define HWLH_CMD_FORBID_DST_HEART	0x02
#define HWLH_CMD_START_DST_HEART	0x03
#define HWLH_CMD_WIRESS_SEND_MAC	0xaa
#define HWLH_CMD_WIRESS_CFG			0x0b
#define HWLH_CMD_GET_NET_CFG		0x0f
#define HWLH_CMD_CHANGE_NODE_TYPE	0x0a
#define HWLH_CMD_SEARCH_NETID_BY_DEVID	0xa5
#define HWLH_CMD_MODIFY_UART_BUADRATE	0xb5
#define HWLH_CMD_TRAN_DATA			0x06
#define HWLH_CMD_BROADCAST_RST		0x07
#define HWLH_CMD_REQUST_RST			0x08
#define HWLH_CMD_GET_NODE_MAC		0xc4
#define HWLH_CMD_GET_CH_PANID		0xb6
#define HWLH_CMD_HL_TRAN_DATA		0xa3
#define HWLH_CMD_HL_ACK_DATA		0xa0

#define HWLH_BUADRATE_9600		0x00
#define HWLH_BUADRATE_19200		0x01
#define HWLH_BUADRATE_38400		0x02
#define HWLH_BUADRATE_57600		0x03
#define HWLH_BRADRATE_115200	0x04

/*

*/
typedef struct _hwlh_frame_hex_rx
{
	u_char ubSOI;
	u_char ubaAddr[3];
	u_char ubCMD;
	u_char ubaData[48];/*include data ,2byte crc and 1byte frame end*/
}HWLH_FRAME_HEX_RX_ST;


typedef struct _hwlh_frame_hex_tx
{
	u_char ubSOI;
	u_char ubaAddr[3];
	u_char ubRssi;
	u_char ubCMD;
	u_char ubaData[48];/*include data ,2byte crc and 1byte frame end*/
}HWLH_FRAME_HEX_TX_ST;


/*
all data is ascii format except frame header and frame end
*/
typedef struct _hwlh_frame_ascii
{
	u_char ubSOI;
	u_char ubaAddr[6];
	u_char ubaCMD[2];
	u_char ubaData[64];/*include data ,4byte crc and 1byte frame end*/
}HWLH_FRAME_ASCII_ST;



typedef struct _hwlh_cmd_cfg_data
{
	u_char ubChannel;
	u_short uwPanID;
}HWLH_CMD_CFG_DATA_ST;

typedef struct _hwlh_cmd_get_parent_addr
{
	u_char ubShortAddr;
}HWLH_CMD_GET_PARENT_ADDR_ST;


typedef struct _hwlh_cmd_forbid_dst_heart
{
	u_char ubDstHeart;
}HWLH_CMD_FORBID_DST_HEART_ST;


typedef struct _hwlh_cmd_start_dst_heart
{
	u_char ubDstHeart;
}HWLH_CMD_START_DST_HEART_ST;



typedef struct _hwlh_cmd_wiress_cfg
{
	u_char ubChannel;
	u_short uwPanID;
}HWLH_CMD_WIRESS_CFG_ST;


typedef struct _hwlh_cmd_get_net_cfg
{
	u_char ubChannel;
	u_short uwPanID;
}HWLH_CMD_GET_NET_CFG_ST;


typedef struct _hwlh_cmd_tran_data
{
	u_char ubLen;
	u_char ubaData[31];
}HWLH_CMD_TRAN_DATA_ST;

typedef struct _hwlh_serial_rx_ascii
{
	u_char ubLen;
	u_char ubaBuf[127];
}HWLH_SERIAL_RX_ASCII_ST;

typedef struct _hwlh_serial_rx_hex
{
	u_char ubLen;
	u_char ubaBuf[63];
}HWLH_SERIAL_RX_HEX_ST;




extern int hwlhFillRxFrame(u_char *pioFrame, const u_char *pciAddr, u_char ubCmd, const u_char *piData, u_char ubDataLen);
extern int hwlhHexFrameToAscii(u_char *pioAsciiFrame, const u_char *pciHexFrame, u_char ubHexFrameDataLen);
extern int hwlhAsciiFrameToHex(u_char *pioHexFrame, const u_char *pciAsciiFrame, u_char ubAsciiFrameDataLen);
extern int hwlhAppHexFrameToAscii(u_char *pioAsciiFrame, const u_char *pciHexFrame, u_char ubHexFrameDataLen);
extern int hwlhFillTxFrame(u_char *pioFrame, const u_char *pciAddr, u_char ubRssi,u_char ubCmd, const u_char *piData, u_char ubDataLen);
extern int hwlhFillHLFrame(u_char *pioFrame,  const u_char *piData, u_char ubDataLen);
#endif
