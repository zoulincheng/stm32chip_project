#ifndef _HWFS_H
#define _HWFS_H



/*these cmd is used to communication to host machine and transnate to router,
if the frame is belong to router, router node process this frame, if the frame
is belong to end node, this frame need to send to end node at nexxt heart frame.
*/

#define HWFS_CMD_CFG_NET			0x00
#define HWFS_CMD_QUERY_ROUTER_ADDR	0x01
#define HWFS_CMD_FORBID_END_NODE	0x02	//屏蔽目标节点，使其不能发送心跳包
#define HWFS_CMD_ALLOW_END_NODE		0x03
#define HWFS_CMD_SEND_MAC			0xaa
#define HWFS_CMD_CFG_WIRELESS		0x0b
#define HWFS_CMD_GET_NET_PARAM		0x0f
#define HWFS_CMD_DEV_TYPE_CHANGE	0x0a
#define HWFS_CMD_SEARCH_NETID_BY_DEVID	0xa5
#define HWFS_CMD_MODIFY_UART_BUARD	0xb5
#define HWFS_CMD_TRAN_DATA			0x06
#define HWFS_CMD_BROAD_CAST			0x07
#define HWFS_CMD_GET_MAC_BY_SHORT_ADDDR	0xc4
#define HWFS_CMD_MODIFY_DOWN_DELAY_TIME	0xd5

#define HWFS_FRAME_FIX_LEN			8				//hwfs frame length, not include data length, is used to comulate data length
#define HWFS_FRAME_HOST_FIX_LEN		9
#define HWFS_NETID_LEN				2				//end node addr length
#define HWFS_FRAME_HEAD_CRC_END_BYTES	4			//1byte head + 1byte end + 2 bytes crc = 4bytes

#define HWFS_UP_DATA_LEN		4					//end node up data length
#define HWFS_DATA_START			0x01

#define HWFS_UART_BURAD_9600		0x00
#define HWFS_UART_BURAD_19200		0x01
#define HWFS_UART_BURAD_38400		0x02
#define HWFS_UART_BURAD_57600		0x04
#define HWFS_UART_BURAD_115200		0x05

/*
this struct is data struct when cmd is HWFS_CMD_CFG_NET
*/
typedef struct _hwfs_cfg_net_data
{
	u_char ubchannel;					//cfg rf channel
	u_char ubRev;						//this byte reserve, not care
	u_char ubPanID;						//the net panid
}HWFS_CFG_NET_DATA;



/*
This struct is the hwfs frame,is used to commulation to host
*/
typedef struct _hwfs
{
	u_char ubHwfsHead;					//hwfs frame head
	u_char ubaNetID[HWFS_NETID_LEN];					//hwfs frame node addr 
	u_char ubDevID;						//hwfs frame devid, 
	u_char ubCMD;						//hwfs frame cmd
	u_char ubaData[];   //data + 2 byte crc + 1 byte frame end  
}HWFS_FRAME;


typedef struct _hwfs_host
{
	u_char ubHwfsHead;					//hwfs frame head
	u_char ubaNetID[HWFS_NETID_LEN];	//hwfs frame node addr 
	u_char ubDevID;						//hwfs frame devid, 
	u_char ubRSSI;						//ubRssi
	u_char ubCMD;						//hwfs frame cmd
	u_char ubaData[];   //data + 2 byte crc + 1 byte frame end  
}HWFS_HOST_FRAME;

/*
This struct is the data whic come from end node, and is save by router node.
*/
typedef struct _hwfs_data
{
	u_char ubRSSI;							//rf radio rssi
	u_char devid;							//end node addr
	u_char cmd;								//cmd which is belong to HWFS cmd, see HWFS_CMD_XXXX
	u_char ubaData[HWFS_UP_DATA_LEN];		//data length
}HWFS_DATA;


typedef struct _hwfs_up_load
{
	u_char ubLayer;
	u_char ubTxCount;
	u_char ubRSSI;
	u_char devid;
	u_char cmd;
	u_char ubaData[HWFS_UP_DATA_LEN];
}HWFS_UP_LOAD;


int hwfsFillFrame(u_char *pioBuf, const u_char *pcNetId, u_char ubDevID, u_char ubCmd, const u_char *pcData, u_char ubLen);
int hwfsFillToHostFrame(u_char *pioBuf, const u_char *pcNetId, u_char ubDevID, u_char ubRssi,u_char ubCmd, const u_char *pcData, u_char ubLen);
#endif



