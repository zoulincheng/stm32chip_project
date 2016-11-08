#include "contiki.h"
#include "basictype.h"
#include "sysprintf.h"
#include <string.h>

#include "stm32f10x.h"
#include "eeflash.h"

#include "dev/watchdog.h"

 /*page size is 2048 byte*/
#define FLASH_PAGE_SIZE    				(0x0800UL)
#define FLASH_PAGE_OFFSET_MASK  		(FLASH_PAGE_SIZE-1)
#define FLASH_PAGE_MASK  				(0xffffFFFF-FLASH_PAGE_OFFSET_MASK)


#ifdef MCU_STM32F103RDT6
/*
* \stm32f106RD6 flash is 384k, addr is 0x0800_0000 to 0x0805_ffff
*  make the last page to save user data
*/

#define FLASH_ADDR_START 			(u_long)0x08000000
#define FLASH_ADDR_END_ADDR			(u_long)0x0805ffff

#define FLASH_BLOCK_START 			(u_long)(380*1024)
#define FLASH_BLOCK_SIZE   			(u_long)(4*1024)
#endif

#ifdef MCU_STM32F103RBT6
/*
* \stm32f106Rb6 flash is 128k, addr is 0x0800_0000 to 0x0802_0000
*  make the last page to save user data  0x0801_f000
*/

#define FLASH_ADDR_START 			(u_long)0x08000000
#define FLASH_ADDR_END_ADDR			(u_long)0x08020000

#define FLASH_BLOCK_START 			(u_long)(124*1024)
#define FLASH_BLOCK_SIZE   			(u_long)(4*1024)
#endif


#define FLASH_ADDR_END    			(u_long)(FLASH_ADDR_START+FLASH_BLOCK_START+FLASH_BLOCK_SIZE)
#define BANK_WRITE_START_ADDR  		(u_long)(FLASH_ADDR_START + FLASH_BLOCK_START)
#define BANK_WRITE_END_ADDR    		FLASH_ADDR_END


/*
* \ brief  check a block flash is needed to erase.
*
* \ param dwiAddr The specify address in the flash between
*         BANK_WRITE_END_ADDR to BANK_WRITE_START_ADDR
* \ param wiSize
*
* \ return 
*          -2 the block flash need to erase,if we write data to the flash,
*           0 can write data to the block flash.
*/
static int eeCheckBlockFlash(u_long dwiAddr, u_short wiSize)
{
//    u_int nResult;
    u_short i = 0;

    for (i = 0; i < wiSize; i++)
    {
        if (((u_long*)dwiAddr)[i] != 0xffffffff)
            return -2;/*Flag This block in the flash is needed to erase.*/
    }

    return 0;
}


/*
* \ brief  write multi  word data to the flash block
*
* \ param dwiAddr save the data in the flash,The specify address in the flash between,
*         BANK_WRITE_END_ADDR to BANK_WRITE_START_ADDR.
*
* \ param pciSrc pointer to the data need to save.
* \ param wiDataLen read number of data.
*
* \ return -1 addr error, 
*          -2 addr num is odd.
*          0 write success.
*/
 int eeWriteMultiWord(u_long dwiAddr, const u_long *pciSrc, u_short wiDataLen)
{
    u_short i = 0;
    u_long *pDestAddr = (u_long*)dwiAddr;
    int nResult;

    /*The addr is over the specify page.*/
    if ((dwiAddr > BANK_WRITE_END_ADDR) || (dwiAddr< BANK_WRITE_START_ADDR))
    {
        PRINTF((0,"eeWriteMultiWord:Out of range\r\n"));
        return -1;
    }
    
    /*The addr(dwiAddr) is a odd num, write data here will be made a fault.*/
    if (dwiAddr % 2 != 0)
    {
        return -2;   
    }
    
    /*Check block Flash is needed to erase.*/
    nResult = eeCheckBlockFlash(dwiAddr, wiDataLen);
    FLASH_Unlock( );
    if (nResult == -2)
    {
        FLASH_ErasePage((u_long)pDestAddr);
    }
    
    for (i = 0; i < wiDataLen; i++)
    {
        FLASH_ProgramWord((u_long)pDestAddr, *pciSrc);
        pDestAddr++;
        pciSrc++;
    }
    FLASH_Lock( );
    return 0;
}

u_long eeBlockGetBase(void)
{
	return BANK_WRITE_START_ADDR;
}

u_long eeBlockGetSize(void)
{
	return FLASH_BLOCK_SIZE;
}

u_long eeBlockRead(u_long dwAbsAddr,void*poBuf,u_long dwiSize)
{
	u_long dwLength = 0;
	if(poBuf)
	{
		memcpy(poBuf,(void*)dwAbsAddr,dwiSize);
		dwLength = dwiSize;
	}
	return dwLength;
}


static u_char ubaFlashBuf[2048] = {0};
//static u_char ubaFlashBuf[512] = {0};
u_long eeBlockWrite(u_long dwAbsAddr,const void*pciBuf,u_long dwiSize)
{
	u_long dwPageStart;
	u_long dwPageEnd;
	u_long dwPageOffsetStart;
	u_long dwPageOffsetEnd;
	u_long dwWriteSize;
	u_char *pbkBuf = NULL;
	
	pbkBuf =(u_char *)(&ubaFlashBuf[0]);
//	if(pbkBuf == NULL)
//		return 0;

//	NutFeedWatchdog();
	watchdog_periodic( );
	
	dwPageStart	= dwAbsAddr&FLASH_PAGE_MASK;
	dwPageEnd	= (dwAbsAddr+dwiSize)&FLASH_PAGE_MASK;
	
	dwWriteSize = 0;
	
	for(;dwPageStart <= dwPageEnd; dwPageStart += FLASH_PAGE_SIZE)
	{
		u_long dwPageWriteSize;
		
		if(dwPageStart == dwPageEnd)
			dwPageOffsetEnd = (dwAbsAddr+dwiSize)&FLASH_PAGE_OFFSET_MASK;
		else
			dwPageOffsetEnd =  FLASH_PAGE_SIZE;

		if(dwPageStart == (dwAbsAddr&FLASH_PAGE_MASK))
			dwPageOffsetStart = dwAbsAddr&FLASH_PAGE_OFFSET_MASK;
		else
			dwPageOffsetStart = 0;

		eeBlockRead(dwPageStart,pbkBuf,FLASH_PAGE_SIZE);
		//eeBlockRead(dwPageStart,pbkBuf,512);
		
		dwPageWriteSize = dwPageOffsetEnd - dwPageOffsetStart;
		
		if(memcmp(&pbkBuf[dwPageOffsetStart],&((u_char*)pciBuf)[dwWriteSize],dwPageWriteSize) != 0)
		{
			memcpy(&pbkBuf[dwPageOffsetStart],&((u_char*)pciBuf)[dwWriteSize],dwPageWriteSize);
			if(eeWriteMultiWord(dwPageStart,(u_long *) pbkBuf, FLASH_PAGE_SIZE/sizeof(u_long)))
				break;
		}

		dwWriteSize += dwPageOffsetEnd - dwPageOffsetStart;
	}
	
	//free(pbkBuf);
	return dwWriteSize;
}

