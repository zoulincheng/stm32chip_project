#ifndef _EXTGDB_376_2_DEV_INFO_H
#define _EXTGDB_376_2_DEV_INFO_H


//define node type
#define NODE_TYPE_CENTER			0x00
#define NODE_TYPE_LEAF				0x01

//define RF TX POWER
#define RF_TX_POWER_01DBM			0x00
#define RF_TX_POWER_02DBM			0x01
#define RF_TX_POWER_05DBM			0x02
#define RF_TX_POWER_08DBM			0x03
#define RF_TX_POWER_11DBM			0x04
#define RF_TX_POWER_14DBM			0x05
#define RF_TX_POWER_17DBM			0x06
#define RF_TX_POWER_20DBM			0x07


#define RF_RSSI_LIMIT				90


#define NODE_DEFAULT_ADDR 			{0x05,0x09,0x16,0x20,0x00,0x00,0x00,0x00}

#define NODE_SOFTWARE_V				0x10  //v1.0
#define NODE_HARDWARE_V				0x10  //v1.0
#define NODE_MADE_INFO				"hrun"


#define MAX_SAVE_ITEMS_IN_FLASH_NUM		6

//DEV_PARAM_STORAGE_INFO  DT;
#define LABLE_RF_NODE_PARAM			1
#define LABLE_ADDR_INFO				2
#define LABLE_MANNUFACTURE			3
#define LABLE_FIRE_NODE_INFO		4

typedef struct
{
	u_char ubNodeType;
	u_char ubTxPower;
	u_char ubRSSI_Limit;
	u_char ubRFChannel;
	u_char ubHwfsPanId;
	u_char ubHwfsDevId;
	u_char ubHwggUargBurad;
	u_char ubaHwfsNetId[2];
	u_short uwPanID;
}RF_NODE_PARAM_CONFIG;

// used to save node addr
typedef struct
{
	u_char ubaNodeAddr[8];
}NODE_ADDR_INFO;


typedef struct
{
	u_char ubSoftWareVersion;
	u_char ubHardWareVersion;
	u_char ubFirmMark[6];
}MANNUFACTURE_PARAM_CONFIG;


typedef struct
{
	u_char ubaNodeAddr[4];
}FIRE_NODE_ADDR;


/*save fire node addr info and total fire node nums*/
#define FIRE_NODE_MAX_NUM		200

typedef struct
{
	u_short node_num;
	FIRE_NODE_ADDR nodeArray[FIRE_NODE_MAX_NUM];
}FIRE_NODE_INFO;

typedef struct
{
	u_short 						udwProtectWord;

	RF_NODE_PARAM_CONFIG 			st1NodeConfig;

	NODE_ADDR_INFO 					st2nodeAddrInfo;
	
	MANNUFACTURE_PARAM_CONFIG 		st3MakerConfig;

	FIRE_NODE_INFO					st4NodeInfo;
		
}DEV_PARAM_STORAGE_INFO;



extern const DEV_PARAM_STORAGE_INFO *extgdbdevGetDeviceSettingInfo(void);
extern const void* extgdbdevGetDeviceSettingInfoSt(u_char ubSTx);
extern int extgdbdevGetMeterUsedNum(void);
extern int extgdbdevSetDeviceSettingInfoSt(u_char ubSTx,u_short wOffset,const void*pBuf,u_short wSize);
#endif




