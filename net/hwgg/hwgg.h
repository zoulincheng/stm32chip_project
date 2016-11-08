#ifndef _HWGG_H
#define _HWGG_H


/*end node data struct
this data frame ,cmd and define are used to communication 
*/

#define HWGG_HEAD	0x7e
#define HWGG_END	0x0d


/*these cmd is used to communication to  end node by router node*/
#define HWGG_CMD_HEART			0xcb
#define HWGG_CMD_WARN			0xca
#define HWGG_CMD_LOW_VOLTAGE	0xc1
#define HWGG_CMD_TRAN			0xcc

#define HWGG_CONST_DST_ADDR			0xffff
#define HWGG_CONST_SRC_ADDR			0xffff
#define HWGG_CONST_END_ADDR			0xffff
#define HWGG_CONST_SEQ				0xff
#define HWGG_CONST_SRC_LAYER		0xff
#define HWGG_CONST_END_LAYER		0xff

#define HWGG_HEAD_END_CRC_LEN		0x04


/*
typedef struct _hwgg
{
	u_char ubHwggHead;		1byte			//hwgg frame head			
	u_char ubLen;			1byte			//hwgg frame len [ublen to ubaData]
	u_char ubPanId;			1byte			//hwgg frame panid

	u_char ubaDstAddr[2];	2bytes			//hwgg dst addr, the frame send to this addr
	u_char ubaSrcAddr[2];	2bytes			//hwgg src addr, the frame come from this addr
	u_char ubaEndAddr[2];	2bytes			//终点地址（EndAddr）

	u_char ubSrcLayer;		1byte			//源层（SrcLayer）
	u_char ubEndLayer;		1byte			//终点层（EndLayer）

	u_char ubSeq;			1byte			//hwgg frame seq, when ack to end node, not change
	u_char ubCmd;			1byte			//hwgg frame cmd

	u_char ubaData[];    //data + 2 byte crc + 1 byte frame end   
}HWGG_FRAME;
13 + 2byte crc + 1byte end = 16bytes
*/

#define HWGG_FRAME_FIX_LEN			16				//the frame length, not include data length

#define HWGG_NODE_MAC_LEN			4				//end node mac addr length

#define HWGG_MAX_END_NODES			200				//max end node in out net

#define HWGG_NETADDR_LEN			2

#define HWGG_NODE_IN_NET			0x01
#define HWGG_NODE_OUT_NET			0x00

//#define HWGG_NODE_CHECK_TIMS_S		360  //second
//#define HWGG_NODE_CHECK_TIMS_S		3600  //second
#define HWGG_NODE_CHECK_TIMS_S		7200  //second


/*this frame struct is used to communication to end node by router node*/
typedef struct _hwgg
{
	u_char ubHwggHead;					//hwgg frame head
	u_char ubLen;						//hwgg frame len [ublen to ubaData]
	u_char ubPanId;						//hwgg frame panid

	u_char ubaDstAddr[2];				//hwgg dst addr, the frame send to this addr
	u_char ubaSrcAddr[2];				//hwgg src addr, the frame come from this addr
	u_char ubaEndAddr[2];				//终点地址（EndAddr）

	u_char ubSrcLayer;					//源层（SrcLayer）
	u_char ubEndLayer;					//终点层（EndLayer）

	u_char ubSeq;						//hwgg frame seq, when ack to end node, not change
	u_char ubCmd;						//hwgg frame cmd

	u_char ubaData[];    //data + 2 byte crc + 1 byte frame end   
}HWGG_FRAME;


typedef struct _node_info
{
	struct _node_info *next;
	u_char nodeNetState;
	u_char ubaHWGGMacAddr[HWGG_NODE_MAC_LEN];
	u_long lastRevPacketTime;
}NODE_INFO;

typedef struct _fire_node
{
	u_char ubLen;
	u_char ubaSrcMac[HWGG_NODE_MAC_LEN];
	u_char ubaDstMac[HWGG_NODE_MAC_LEN];
	u_char ubCmd;
	u_char ubaData[];
}FIRE_NODE;

typedef struct _fire_node_data
{
	u_char ublen;
	u_char ubadata[127];
}FIRE_NODE_DATA;

#define FIRE_FIX_LEN		10

#if 0
typedef struct _node_info
{
	struct _node_info *next;
	u_char ubaHWGGNetAddr[HWGG_NETADDR_LEN];		//hex code addr: 
	u_char ubaHWGGMacAddr[HWGG_NODE_MAC_LEN];		//hex node mac addr: end node
	u_char ubHwfsDevID;								//device  id
	u_char ubheartFlag;								//1 not send to host, 0 send heart to host

	u_char ubNetState;								//net state, in net or not in net
	u_char ubHeartBeatCount;						//record heart count in 100s

	u_char ubLenUp;									//data buf length  up 
	u_char ubaDataUp[HWGG_MAX_DATA_LEN];			//data up buf:cmd + data, to end node

	u_char ubLenDown;								//data buf length down
 	u_char ubaDataDown[HWGG_MAX_DATA_LEN];			//data down buf:cmd + data, to end node	

	clock_time_t lastRevPacketTime;					//record the last heart time uint second
}NODE_INFO;
#endif


int hwggFillFrame(u_char *pioBuf, u_char uwpanId, u_char ubCMD, const u_char *pcData, u_char ubLen);

#endif
