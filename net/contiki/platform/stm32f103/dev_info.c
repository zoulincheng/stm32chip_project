#include "contiki.h"
#include "basictype.h"
#include "sysprintf.h"
#include <string.h>

//#include "extgdb_376_2.h"

#include "eeflash.h"
#include "dev_info.h"



//__attribute__((at(0x0805F000)))

extern int mem_cmp (const void* dst, const void* src, unsigned int cnt);

// This variable is save at addr 0x0805f000  FLASH
#ifdef MCU_STM32F103RDT6
//#define DEVINFO_FLASH_ADDR	(0x0805F000)

const DEV_PARAM_STORAGE_INFO devInfo @0x0805F000 ={

	//u_short udwProtectWord; 
	0x5aa5,

	//RF_NODE_PARAM_CONFIG stNodeConfig;
	//u_char ubNodeType;
	//u_char ubTxPower;
	//u_char ubRSSI_Limit;
	//u_char ubRFChannel;
	//u_short uwPanID;
	{
		NODE_TYPE_LEAF,
		RF_TX_POWER_20DBM,
		RF_RSSI_LIMIT,
		1,
		0x00,
		0x00,
		0x00,
		{0x00,0x00},
		0xABCD
	},

	//NODE_ADDR_INFO stnodeAddrInfo;
	//u_char ubaNodeAddr[8];
	{
		NODE_DEFAULT_ADDR
	},

	//u_char ubSoftWareVersion;
	//u_char ubHardWareVersion;
	//u_char ubFirmMark[6];
	{
		NODE_SOFTWARE_V,
		NODE_HARDWARE_V,
		{NODE_MADE_INFO}
	}
}; 

#endif

#ifdef MCU_STM32F103RBT6
#define DEVINFO_FLASH_ADDR	(0x0801F000)
//This variable is save at addr 0x0801f000  FLASH
const DEV_PARAM_STORAGE_INFO devInfo @0x0801F000 ={

	//u_short udwProtectWord; 
	0x5aa5,

	//RF_NODE_PARAM_CONFIG stNodeConfig;
	//u_char ubNodeType;
	//u_char ubTxPower;
	//u_char ubRSSI_Limit;
	//u_char ubRFChannel;
	//u_short uwPanID;
	{
		NODE_TYPE_LEAF,
		//NODE_TYPE_CENTER,
		RF_TX_POWER_20DBM,
		RF_RSSI_LIMIT,
		5,
		0x00,	//painid
		0x00,	//devid
		0x00,	//9600
		{0x00,0x00},//netid
		//RF_CHANNEL_470_200,
		0xABCD
	},

	//NODE_ADDR_INFO stnodeAddrInfo;
	//u_char ubaNodeAddr[8];
	{
		NODE_DEFAULT_ADDR
	},

	//u_char ubSoftWareVersion;
	//u_char ubHardWareVersion;
	//u_char ubFirmMark[6];
	{
		NODE_SOFTWARE_V,
		NODE_HARDWARE_V,
		{NODE_MADE_INFO}
	},
	{
		0x00
	}
}; 

#endif


static const void*pdevSettingBase[] = {
	&devInfo.st1NodeConfig,
	&devInfo.st2nodeAddrInfo,
	&devInfo.st3MakerConfig,
	&devInfo.st4NodeInfo
};



const DEV_PARAM_STORAGE_INFO *extgdbdevGetDeviceSettingInfo(void)
{
	return (DEV_PARAM_STORAGE_INFO*)&devInfo;
}

/*
* \ brief Get param struct addr.
* \ param ubSTx Param struct lable num.
* \ return NULL, failed, other value 
*/

const void* extgdbdevGetDeviceSettingInfoSt(u_char ubSTx)
{
	if((ubSTx > 0) && (ubSTx<= MAX_SAVE_ITEMS_IN_FLASH_NUM))
	{

		ubSTx -= 1;

		return  pdevSettingBase[ubSTx];
	}	
	return NULL;
}





/*
* \ brief Write data to the addr
* \ param woffset offset addr
* \ param pBuf The pointer to the data
* \ param wSize Data size
* \ return -1 failed, 0 success.
*/

#if 0
int extgdbdevSetDeviceSettingInfoSt(u_char ubSTx,u_short wOffset,const void*pBuf,u_short wSize)
{
	if((ubSTx > 0) && (ubSTx<= 5))
	{
		ubSTx -= 1;
		XPRINTF((7,"ST:0x%08X+0x%04X[sz %d]\r\n",pdevSettingBase[ubSTx],wOffset,wSize));
		eeBlockWrite((u_long)pdevSettingBase[ubSTx]+wOffset,pBuf,wSize);
		return 0;
	}
	return -1;
}
#else
//static DEV_PARAM_STORAGE_INFO nodeInfo;

static u_char ubanodeInfo[2048];

int extgdbdevSetDeviceSettingInfoSt(u_char ubSTx,u_short wOffset,const void*pBuf,u_short wSize)
{
	if((ubSTx > 0) && (ubSTx<= MAX_SAVE_ITEMS_IN_FLASH_NUM))
	{
		//ubSTx -= 1;
		DEV_PARAM_STORAGE_INFO *pdevInfo = (DEV_PARAM_STORAGE_INFO *)ubanodeInfo;
		u_short uwWriteWord = 0;
		XPRINTF((9,"ST:0x%08X+0x%04X[sz %d]\r\n",pdevSettingBase[ubSTx],wOffset,wSize));
		//eeBlockRead((u_long)DEVINFO_FLASH_ADDR, &nodeInfo, sizeof(DEV_PARAM_STORAGE_INFO));
		eeBlockRead((u_long)DEVINFO_FLASH_ADDR, pdevInfo, sizeof(DEV_PARAM_STORAGE_INFO));

		//MEM_DUMP(9, "ENFO", (void const *)DEVINFO_FLASH_ADDR, sizeof(DEV_PARAM_STORAGE_INFO));
		//MEM_DUMP(6, "INFO", &nodeInfo, sizeof(DEV_PARAM_STORAGE_INFO));
		//MEM_DUMP(9, "INFO", pdevInfo, sizeof(DEV_PARAM_STORAGE_INFO));
		if (ubSTx == 1)
		{
			//nodeInfo.st1NodeConfig = *(RF_NODE_PARAM_CONFIG *)pBuf;
			pdevInfo->st1NodeConfig = *(RF_NODE_PARAM_CONFIG *)pBuf;
		}
		else if (ubSTx == 2)
		{
			//nodeInfo.st2nodeAddrInfo = *(NODE_ADDR_INFO *)pBuf;
			pdevInfo->st2nodeAddrInfo = *(NODE_ADDR_INFO *)pBuf;
		}
		else if (ubSTx == 3)
		{
			//nodeInfo.st3MakerConfig = *(MANNUFACTURE_PARAM_CONFIG*)pBuf;
			pdevInfo->st3MakerConfig = *(MANNUFACTURE_PARAM_CONFIG*)pBuf;
		}
		else if (ubSTx == 4)
		{
			pdevInfo->st4NodeInfo = *(FIRE_NODE_INFO*)pBuf;
		}

        //MEM_DUMP(6, "fNFO", &nodeInfo, sizeof(DEV_PARAM_STORAGE_INFO));
        MEM_DUMP(9, "fNFO", pdevInfo, sizeof(DEV_PARAM_STORAGE_INFO));

		if (sizeof(DEV_PARAM_STORAGE_INFO)%(sizeof(u_long)) == 0)
		{
			uwWriteWord = (sizeof(DEV_PARAM_STORAGE_INFO)/sizeof(u_long));
		}
		else 
		{
			uwWriteWord = (sizeof(DEV_PARAM_STORAGE_INFO)/sizeof(u_long))+1;
		}
        
		//eeBlockWrite((u_long)pdevSettingBase[ubSTx]+wOffset,pBuf,wSize);
		//eeWriteMultiWord(DEVINFO_FLASH_ADDR, (const u_long * )&nodeInfo, (sizeof(DEV_PARAM_STORAGE_INFO)/sizeof(u_long)));
		eeWriteMultiWord(DEVINFO_FLASH_ADDR, (const u_long * )pdevInfo, uwWriteWord);
		MEM_DUMP(9, "FNFO", (void const *)DEVINFO_FLASH_ADDR, sizeof(DEV_PARAM_STORAGE_INFO));
		return 0;
	}
	return -1;
}

#endif


//test mcu inter flash

#if 0
static MANNUFACTURE_PARAM_CONFIG stMake={
0x29,
0x32,
"abcde"
};
void extgdbTestFlash(void)
{
	const DEV_PARAM_STORAGE_INFO *pDevInfo = NULL;
	const MANNUFACTURE_PARAM_CONFIG *pMakerInfo = NULL;
	MANNUFACTURE_PARAM_CONFIG stMakerInfo = {0};
	u_long udwPaddr = NULL;
	
	pDevInfo = extgdbdevGetDeviceSettingInfo( );

	PRINTF("Protect is %02x\r\n", pDevInfo->udwProtectWord);
	PRINTF("Made info is %s\r\n", pDevInfo->st3MakerConfig.ubFirmMark);

	udwPaddr = (u_long)extgdbdevGetDeviceSettingInfoSt(1);
	PRINTF("ST1 addr is %08x\r\n", udwPaddr);


	udwPaddr = (u_long)extgdbdevGetDeviceSettingInfoSt(2);
	PRINTF("ST2 addr is %08x\r\n", udwPaddr);

	//test write flash
	PRINTF("Read st3 and print");
	udwPaddr = (u_long)extgdbdevGetDeviceSettingInfoSt(3);
	PRINTF("ST3 addr is %08x\r\n", udwPaddr);
	pMakerInfo = (MANNUFACTURE_PARAM_CONFIG *)udwPaddr;
	PRINTF("Softwarev is %02x\r\n", pMakerInfo->ubSoftWareVersion);
	PRINTF("Hardwarev is %02x\r\n", pMakerInfo->ubHardWareVersion);
	PRINTF("Hardwarev is %s\r\n", pMakerInfo->ubFirmMark);

	PRINTF("copy st3 and print");
	stMakerInfo = *pMakerInfo;
	PRINTF("Softwarev is %02x\r\n", stMakerInfo.ubSoftWareVersion);
	PRINTF("Hardwarev is %02x\r\n", stMakerInfo.ubHardWareVersion);
	PRINTF("Hardwarev is %s\r\n", stMakerInfo.ubFirmMark);


	PRINTF("write st3 and print");
	stMakerInfo.ubSoftWareVersion = 0x22;
	stMakerInfo.ubHardWareVersion = 0x33;

	extgdbdevSetDeviceSettingInfoSt(3, 0, &stMakerInfo, sizeof(MANNUFACTURE_PARAM_CONFIG));
	PRINTF("Softwarev is %02x\r\n", pMakerInfo->ubSoftWareVersion);
	PRINTF("Hardwarev is %02x\r\n", pMakerInfo->ubHardWareVersion);
	PRINTF("Hardwarev is %s\r\n", pMakerInfo->ubFirmMark);



	udwPaddr = (u_long)extgdbdevGetDeviceSettingInfoSt(4);
	PRINTF("ST4 addr is %08x\r\n", udwPaddr);
	
}

#endif



