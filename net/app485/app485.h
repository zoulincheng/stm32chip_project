#ifndef _APP_485_H
#define _APP_485_H


#define TB_FRAME_HEADER_LEN			4
#define TB_SOH_FIX_LEN				6
#define TB_SAKED_FIX_LEN			4
#define TB_BSCOH_FIX_LEN			4
#define TB_ACK_FIX_LEN				4
#define TB_COMMON_FIX_LEN			3

/*
485 和 232 的物理链路传输协议相同，由起始位，数据域，校验位，停止位组成。具体描述为：起始
位 1 位，数据位 8 位，校验位 1 位偶校验，停止位 1 位，波特率 9600。
*/
#define TB_SYN		0xaa			//同步字符，当字节流出现连续两个以上的同步字符（SYN）时，认为是一个数据包开始
#define TB_EOT		0xaf			//结束字符，当字节流中出现结束字符（EOT）时，认为是数据包数据结束标志。
#define TB_DLE		0xa0
/*
转意字符，当字节流数据中出现同步字符（SYN）, 结束字符（EOT），转意字符（DLE）
需要传输时，需要在该字符前加上转意字符（DLE），接收时，忽略转意字符，其后的特征字符按
普通数据处理
*/
#define TB_DLE		0xa0	


/*
连接层传输协议必须遵循以下通讯机制：
1. 对于发送方的每一个数据包，接收方根据接收的数据必须向发送方发送响应数据包（接收确认或
接收非确认）。
2. 对于多包数据传输，其数据发送有效包号从 1 开始。数据包号为 0 的 ACK 将触发实际多包数据的
发送传输过程。发送 PKG_NO 号 SOH 包数据，接收方响应 PKG_NO 号 ACK，发送方收到 PKG_NO
号 ACK，发送方发送 PKG_NO+1 号包数据。
3. 当接收方检测到数据出错时，向发送方发送非确认 NAK 数据包。
4. 发送方发送完一个数据包后，在 20ms 后没有收到接收方的确认或非确认信号，重发此数据包，重
发次数为 3。
5. 为了启动数据发送过程，发送方首先发送 ENQ 数据包，等待接收发方发送 PGK_NO=0 的 ACK 确
认包，发送方在收到 PKG_NO=0 的 ACK 确认包后，进入 SOH 数据包发送状态。
*/

/*
485 的通讯有主从之分，与之对应的协议也有主从之分。表 5-1 为连接层所用特征字符以及相应的通
讯桢格式。
表 5-1
通讯桢含义 特征字符 通讯桢格式
数据 0xE0 (SOH) SOH SRCADDR DESTADDR PKG_NO LEN TYPE DATA
广播数据 0xBB (BCSOH) BCSOH SRCADDR =0 DESTADDR =0 CMD_NO DATA
退出 0xE1 (EXT) EXT SRCADDR DESTADDR
接收确认 0xE2 (ACK) ACK SRCADDR DESTADDR PKG_NO
接收非确认 0xE4 (NAK) NAK SRCADDR DESTADDR
结束 0xE8 (NUL) NUL SRCADDR DESTADDR
结束确认 0xE9 (NULACK) NULACK SRCADDR DESTADDR
查询 0xE7 (ENQ) ENQ SRCADDR DESTADDR
握手 0xD0 (SAK) SAK SRCADDR DESTADDR
握手确认 0xDF (SAKED) SAKED SRCADDR DESTADDR TYPE
连接 0xF1 (LINK) LINK SRCADDR DESTADDR
连接确认 0xF4 (LINKED) LINKED SRCADDR DESTADDR
终止连接 0xF2 (UNLINK) UNLINK SRCADDR DESTADDR
终止连接确认 0xF8 (UNLINKED) UNLINKED SRCADDR DESTADDR
*/
/*
xor check sum 
exapmple
		 H	DATA	 END XOR
AA AA AA AA D0 00 1E AF CE
*/

#define TB_SOH		0xe0			//数据
#define TB_BCSOH	0xbb			//广播数据
#define TB_EXT		0xe1			//退出
#define TB_ACK		0xe2			//接收确认
#define TB_NAK		0xe4			//接收非确认
#define TB_NUL		0xe8			//结束
#define TB_NULACK	0xe9			//结束确认
#define TB_ENQ		0xe7			//查询
#define TB_SAK		0xd0			//握手
#define TB_SAKED	0xdf			//握手确认
#define TB_LINK		0xf1			//连接
#define TB_LINKED	0xf4			//连接确认
#define TB_UNLINK	0xf2			//终止连接
#define TB_UNLINKED	0xf8			//终止连接确认

#define TB_WARN_FIRST_SEQ		0x01	//第一包报警数据序号


#define TB_BSCOH_RSG		0x00
#define TB_BSCOH_SILENCE	0x01
#define TB_BSCOH_TIMESYNC	0x0e


#define TB_SOH_DATA_TYPE_FIRE			0x20
#define TB_SOH_DATA_TYPE_WRONG			0x40
#define TB_SOH_DATA_TYPE_MODULE_START	0x24

#define M_T_FIRE		0x02
#define M_T_WRONG		0x05
#define M_T_START		0x04

#define S_T_HASFIRE				0x01	//火警<反馈>发生		主类型（ 0x02） CMD 0x20
#define S_T_NOFIRE				0x81	//火警<反馈>消失）		主类型（ 0x02）	CMD 0x20

#define S_T_DIS					0x01	//丢失					主类型（ 0x05） CMD 0x40
#define S_T_NDIS				0x81	//丢失消失				主类型（ 0x05）	CMD 0x40
#define S_T_M_OPEN				0x02	//模块负载开路			主类型（ 0x05）	CMD 0x40
#define S_T_M_NOPEN				0x82	//模块负载开路消失		主类型（ 0x05）	CMD 0x40
#define S_T_M_SHORT				0x04	//模块负载短路			主类型（ 0x05）	CMD 0x40
#define S_T_M_NSHORT			0x84	//模块负载短路消失		主类型（ 0x05）	CMD 0x40
#define S_T_KZJ_LD_WRONG		0x05	//KZJ_LD 设备故障		主类型（ 0x05）	CMD 0x40
#define S_T_KZJ_LD_OK			0x85	// KZJ_LD 设备故障消失	主类型（ 0x05）	CMD 0x40
#define S_T_MAKE_MATCH_WRONG	0x06	//生产类型不匹配		主类型（ 0x05）	CMD 0x40
#define S_T_MAKE_MATCH_OK		0x86	//生产类型不匹配消失	主类型（ 0x05）	CMD 0x40
#define S_T_IOB_SHORT			0x10	//接口板回路短路		主类型（ 0x05）	CMD 0x40
#define S_T_IOB_NSHORT			0x90	//接口板回路短路消失	主类型（ 0x05）	CMD 0x40
#define S_T_IOB_NCON			0x50	//接口板通讯故障		主类型（ 0x05）	CMD 0x40
#define S_T_IOB_CON				0xd0	//接口板通讯故障消失	主类型（ 0x05）	CMD 0x40
#define S_T_HOST_NCON			0x51	//主机通讯故障		主类型（ 0x05）		CMD 0x40
#define S_T_HOST_CON			0xd1	//主机通讯故障消失	主类型（ 0x05）		CMD 0x40
#define S_T_DISP_WRONG			0x52	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40
#define S_T_DISP_OK				0xd2	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40
#define S_T_PRINT_WRONG			0x53	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40
#define S_T_PRINT_OK			0xd3	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40
#define S_T_POWER_WRONG			0x54	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40
#define S_T_POWER_OK			0xd4	//显示盘通讯故障	主类型（ 0x05）		CMD 0x40

#define S_T_M_START				0x01	//模块启动请求		主类型（ 0x04）		CMD 0x24
#define S_T_M_STOP				0x81	//模块启动请求		主类型（ 0x04）		CMD 0x24



#define TB_SOH_MASTER_NUM				0x01
#define TB_SOH_INTERFACE_NUM			0x00
#define TB_SOH_DEV_ADDR					


#define TB_MAX_FRAME_LEN	64

typedef struct _app_485_data
{
	u_char ubLen;
	u_char ubaData[127];
}APP_485_DATA;


//探测器或模块故障信息有效数据详细描述
//火警或反馈信息有效数据详细描述
//表 6-15 模块的启动或停止信息有效数据详细描述
//cmd 0x20  ubStruMasterType 0x02  ubStruSalveType 0x01, 0x81
//cmd 0x40  ubStruMasterType 0x05  ubStruSalveType 0x01, 0x81, 0x02, 0x82,0x04, 0x84, 0x05, 0x85, 0x06, 0x86
typedef struct _tb_soh_warn_data
{
	u_char ubLength;				//长度 71数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubHostNo;				//主机号（从 1 开始）
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubAddrNo[2];				//设备地址号 （ 高字节为地址的低位， 低字节为地址的高位）
	u_char ubMakeType;				//生产类型	
	u_char ubEquipmentType;			//设备类型（用于联动，详细描述见后）			//	
	u_char ubZoneNo;				//区号（从 0 开始 0-19）
	u_char ubBuildingNo;			//栋号（从 0 开始 0-19）
	u_char ubFloorNo;				//层号（ 0-199 为 1-200 层）（ 0xFF-0xF6 为-1 至-10 层）
	u_char ubRoomNo;				//房号（ 0-255）
	u_char ubaPlaceDesc[41];		//位置描述，长度为 41，以 0 结尾的字符串
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）
	u_char ubIsolateFlag;			//隔离标志（ 0：未隔离， 1 隔离）
	u_char ubaaucEqbDesc[11];		//设备类型描述，长度为 11，以 0 结尾的字符串
}TB_SOH_WARN_DATA;

//接口板回路通讯故障信息有效数据详细描述
typedef struct _tb_soh_iob_loob_wrong_data
{
	u_char ubLength;				//长度 52数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubCanFaultType;			//CAN 故障类型
	u_char ubHostNo;				//主机号（从 1 开始）
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubaaucDesc[41];			//故障描述
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）
}TB_SOH_IOB_LOOP_WRONG_DATA;


//主机通讯故障信息有效数据详细描述
typedef struct _tb_soh_host_commu_wrong_data
{
	u_char ubLength;				//长度 51数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubCanFaultType;			//CAN 故障类型
	u_char ubHostNo;				//主机号（从 1 开始）
	u_char ubaaucDesc[41];			//故障描述
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）
}TB_SOH_HOST_COMMU_WRONG_DATA;


//显示盘通讯故障信息有效数据详细描述
typedef struct _tb_soh_fsd_wrong_data
{
	u_char ubLength;				//长度 58数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubCanFaultType;			//CAN 故障类型
	u_char ubHostNo;				// 主机号（从 1 开始） 更改为 扩容箱号
	u_char ubFSDAddrNo;				//显示盘号（从 1 开始）

	u_char ubMakeType;				//生产类型	
	u_char ubEquipmentType;			//设备类型（用于联动，详细描述见后）			//	
	u_char ubZoneNo;				//区号（从 0 开始 0-19）
	u_char ubBuildingNo;			//栋号（从 0 开始 0-19）
	u_char ubFloorNo;				//层号（ 0-199 为 1-200 层）（ 0xFF-0xF6 为-1 至-10 层）
	u_char ubRoomNo;				//房号（ 0-255）
	u_char ubaPlaceDesc[41];		//位置描述，长度为 41，以 0 结尾的字符串
	
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）

}TB_SOH_FSD_WRONG_DATA;


#if 0
//表 6-13 打印机故障信息有效数据详细描述
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//长度 52数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubPrintFaultType;		//打印机故障类型 0xF8 //打印机故障
	u_char ubHostNo;				// 主机号（从 1 开始） �
	u_char ubPrinterNo;				//显示盘号（从 1 开始）

	u_char ubaaucDesc[41];			//故障描述
	
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）

}TB_SOH_PRINT_WRONG_DATA;
#endif

//表 6-13 电源故障信息有效数据详细描述
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//长度 52数据，不包含datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//主类型
	u_char ubStruSalveType;			//从类型（ 0x01:火警<反馈>发生 0x81:火警<反馈>消失）
	u_char ubPowerFaultType;		//电源故障类型
	u_char ubHostNo;				// 主机号（从 1 开始）
	u_char ubPowerNo;				//显示盘号（从 1 开始）

	u_char ubaaucDesc[41];			//故障描述
	
	u_char ubYear;					//年（两位 BCD 码）
	u_char ubMonth;					//年（两位 BCD 码）
	u_char ubDay;					//日（两位 BCD 码）
	u_char ubHour;					//时（两位 BCD 码）
	u_char ubMinute;				//分（两位 BCD 码）
	u_char ubSecond;				//秒（两位 BCD 码）

}TB_SOH_PRIONT_WRONG_DATA;



/*
erery frame has this struct
*/
typedef struct _tb_frame_common
{
	u_char ubCmd;
	u_char ubSrcAddr;
	u_char ubDestAddr;
}TB_FRAME_COMMON;


/*
data frame format
*/
typedef struct _tb_frame_soh_data
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNO;
	u_char ubLen;
	u_char ubType;
	u_char ubaData[];	//include frame end and check sum use ^(xor) method
}TB_FRAME_SOH_DATA;


typedef struct _tb_warn_msg
{
	u_char ubLen;
	u_char ubType;
	u_char ubaData[];
}TB_WARN_MSG;

typedef struct _tb_soh_data_common
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNO;
	u_char ubLen;
	u_char ubType;
	u_char ubStruMasterType;
	u_char ubStruSalveType;
}TB_SOH_DATA_COMMON;


typedef struct _tb_soh_msg_common
{
	u_char ubLen;
	u_char ubType;
	u_char ubStruMasterType;
	u_char ubStruSalveType;	
}TB_SOH_MSG_COMMON;

/*
broadcast data frame format
*/
typedef struct _tb_frame_bsc_data
{
	TB_FRAME_COMMON tbCommon;
	u_char ubCmdNo;
	u_char ubaData[];	//include frame end and check sum use ^(xor) method
}TB_FRAME_BSC_DATA;


/*
receive conform ack frame format
*/
typedef struct _tb_frame_ack
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNo;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_ACK;



/*
SAKED frame format
*/
typedef struct _tb_frame_saked
{
	TB_FRAME_COMMON tbCommon;
	u_char ubType;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_SAKED;


typedef struct _tb_frame_common_frame
{
	TB_FRAME_COMMON tbCommon;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_COMMON_FRAME;


#endif
