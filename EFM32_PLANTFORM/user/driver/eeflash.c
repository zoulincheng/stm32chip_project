#include <stdint.h>
#include "em_device.h"
#include "em_msc.h"
#include "basictype.h"
#include <string.h>


#define FLASH_PAGE_OFFSET_MASK  		(FLASH_PAGE_SIZE-1)
#define FLASH_PAGE_MASK  				(0xffffFFFF-FLASH_PAGE_OFFSET_MASK)
/*
* em32lg230f128 flash define
*/
#define FLASH_ADDR_START 			(u_long)0x00000000
#define FLASH_ADDR_END_ADDR			(u_long)0x00020000

#define FLASH_BLOCK_START 			(u_long)(124*1024)
#define FLASH_BLOCK_SIZE   			(u_long)(4*1024)
#define FLASH_ADDR_END    			(u_long)(FLASH_ADDR_START+FLASH_BLOCK_START+FLASH_BLOCK_SIZE)
#define BANK_WRITE_START_ADDR  		(u_long)(FLASH_ADDR_START + FLASH_BLOCK_START)
#define BANK_WRITE_END_ADDR    		FLASH_ADDR_END
/**********************************************************
 * Handles errors from MSC. Sets the status field to tell
 * programmer which error occured and then
 * enter an endless loop 
 * 
 * @param ret
 *    MSC error code
 **********************************************************/
static void handleMscError(msc_Return_TypeDef ret)
{
  if (ret != mscReturnOk)
  {
    /* Generate error code. */
    switch(ret)
    {
    case (mscReturnTimeOut):      
      //state.flashLoaderStatus = FLASHLOADER_STATUS_ERROR_TIMEOUT;
      break;
    case (mscReturnLocked):
      //state.flashLoaderStatus = FLASHLOADER_STATUS_ERROR_LOCKED;
      break;
    case (mscReturnInvalidAddr):
      //state.flashLoaderStatus = FLASHLOADER_STATUS_ERROR_INVALIDADDR;
      break;
    default:
      //state.flashLoaderStatus = FLASHLOADER_STATUS_ERROR_UNKNOWN;
      break;
    }
    while(1);
  }
}


/**********************************************************
 * Waits on the MSC_STATUS register until the selected
 * bits are set or cleared. This function will busy wait
 * until (MSC_STATUS & mask) == value. 
 * Errors are also handled by this function. Errors
 * will cause the flashloader to set the status
 * flag and stop execution by entering a busy loop. 
 * 
 * @param mask
 *    The mask to apply to MSC_STATUS
 * 
 * @param value
 *    The value to compare against, after applying the mask
 * 
 **********************************************************/
static void mscStatusWait( uint32_t mask, uint32_t value )
{
	uint32_t status;
	int timeOut = MSC_PROGRAM_TIMEOUT;

	while (1)
	{
		status = MSC->STATUS;

		/* Check for errors */
		if ( status & ( MSC_STATUS_LOCKED | MSC_STATUS_INVADDR ) )
		{
			/* Disable write access */
			//MSC->WRITECTRL &= ~(MSC_WRITECTRL_WREN | MSC_WRITECTRL_WDOUBLE);
			//MSC->WRITECTRL &= ~(MSC_WRITECTRL_WREN);
			MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;

			/* Set error flag and enter busy loop */
			if ( status & MSC_STATUS_LOCKED )
			{
				handleMscError( mscReturnLocked );
			}

			/* Set error flag and enter busy loop */
			if ( status & MSC_STATUS_INVADDR ) 
			{
				handleMscError( mscReturnInvalidAddr );
			}
		}

		/* We only care about WORDTIMEOUT when waiting for WDATAREADY */
		if ( (status & MSC_STATUS_WORDTIMEOUT) && (mask == MSC_STATUS_WDATAREADY) )
		{
			handleMscError( mscReturnTimeOut );
		}

		/* Check end condition */
		if ( ( status & mask ) == value )
		{
			/* We are done waiting */
			break;
		}

		timeOut--;
		if ( timeOut == 0 )
		{
			/* Timeout occured. Set flag and enter busy loop */
			handleMscError( mscReturnTimeOut );
		}
	}
}



/**********************************************************
 * Execute a flash command (erase/write etc). Will busy 
 * wait until command is complete. 
 *
 * @param cmd
 *    Flash command (value to write to MSC_CMD)
 **********************************************************/
static void doFlashCmd( uint32_t cmd )
{
  MSC->WRITECMD = cmd;
  mscStatusWait( MSC_STATUS_BUSY, 0 );
}

/**********************************************************
 * Writes one word to flash.
 *
 * @param addr
 *    Address to write to. Must be a valid flash address.
 * 
 * @param data
 *    Word to write
 **********************************************************/
static void pgmWord( uint32_t addr, uint32_t data )
{
  MSC->ADDRB    = addr;
  MSC->WRITECMD = MSC_WRITECMD_LADDRIM;
  MSC->WDATA    = data;
  doFlashCmd( MSC_WRITECMD_WRITEONCE );
}


/**********************************************************
 * Erases on page of flash. 
 *
 * @param addr
 *    Address of page. Must be a valid flash address.
 * 
 * @param pagesize
 *    Size of one page in bytes
 **********************************************************/
static void eraseSector( uint32_t addr, uint32_t pagesize )
{
  uint32_t *p = (uint32_t*)addr, result;

  /* Check if page already is erased. If so we can
   * simply return. */
  do
  {
    result    = *p++;
    pagesize -= 4;
  } while ( pagesize && ( result == 0xFFFFFFFF ) );

  if ( result != 0xFFFFFFFF )
  {
    /* Erase the page */
    MSC->ADDRB    = addr;
    MSC->WRITECMD = MSC_WRITECMD_LADDRIM;
    doFlashCmd( MSC_WRITECMD_ERASEPAGE );
  }
}

/**********************************************************
 * Writes multiple words to flash
 *
 * @param addr
 *    Where to start writing. Must be a valid flash address.
 * 
 * @param p
 *    Pointer to data
 * 
 * @param cnt
 *    Number of bytes to write. Must be a multiple
 *    of four.
 **********************************************************/
static void pgmBurst( uint32_t addr, const uint32_t *p, uint32_t cnt )
{
  /* Wait until MSC is ready */
  mscStatusWait( MSC_STATUS_BUSY, 0 );
  
  /* Enter start address */
  MSC->ADDRB    = addr;
  MSC->WRITECMD = MSC_WRITECMD_LADDRIM;
  
  /* Write first word. Address will be automatically incremented. */
  MSC->WDATA    = *p++;
  MSC->WRITECMD = MSC_WRITECMD_WRITETRIG;
  cnt          -= 4;

  /* Loop until all words have been written */
  while ( cnt )
  {
    mscStatusWait( MSC_STATUS_WDATAREADY, MSC_STATUS_WDATAREADY );
    MSC->WDATA = *p++;
    cnt       -= 4;
  }
  
  /* End writing */
  MSC->WRITECMD = MSC_WRITECMD_WRITEEND;
}

/**********************************************************
 * Writes multiple words to flash, using double word
 * writes.
 *
 * @param addr
 *    Where to start writing. Must be a valid flash address.
 * 
 * @param p
 *    Pointer to data
 * 
 * @param cnt
 *    Number of bytes to write. 
 * 
 * @returns 
 *    Number of bytes left that was NOT written.
 **********************************************************/
uint32_t pgmBurstDouble( uint32_t addr, uint32_t *p, uint32_t cnt )
{
  /* Wait until MSC is ready */
  mscStatusWait( MSC_STATUS_BUSY, 0 );
  
  /* Enter start address */
  MSC->ADDRB    = addr;
  MSC->WRITECMD = MSC_WRITECMD_LADDRIM;
  
  /* Write first words. Address will be automatically incremented. */
  MSC->WDATA    = *p++;
  MSC->WDATA    = *p++;
  MSC->WRITECMD = MSC_WRITECMD_WRITETRIG;
  cnt          -= 8;

  /* Loop until all words have been written */
  while ( cnt > 7 )
  {
    mscStatusWait( MSC_STATUS_WDATAREADY, MSC_STATUS_WDATAREADY );
    MSC->WDATA = *p++;
    MSC->WDATA = *p++;
    cnt       -= 8;
  }
  
   /* End writing */
  MSC->WRITECMD = MSC_WRITECMD_WRITEEND;
  
  /* Return number of bytes not written by this function */
  return cnt;
}



int erasePage(u_long pageaddr, u_long pagesize)
{
	MSC_Status_TypeDef ret = mscReturnOk;
	uint32_t *p = (uint32_t*)pageaddr;
	uint32_t result;

	/* Check if page already is erased. If so we can
	* simply return. */
	do
	{
		result    = *p++;
		pagesize -= 4;
	} while ( pagesize && ( result == 0xFFFFFFFF ));

	if ( result != 0xFFFFFFFF )
	{
		ret = MSC_ErasePage((uint32_t *)(pageaddr&FLASH_PAGE_MASK));
	}

	return ret;
}


int eeWriteMultiWord(u_long dwiAddr, const u_long *pciSrc, u_short wiDataLen)
{
  MSC_Status_TypeDef ret;
  u_short uwLen = wiDataLen*sizeof(u_long);

   /*The addr is over the specify page.*/
  if ((dwiAddr > BANK_WRITE_END_ADDR) || (dwiAddr< BANK_WRITE_START_ADDR))
  {
      //PRINTF((0,"eeWriteMultiWord:Out of range\r\n"));
      return -1;
  }

  #if 1
  /* Initialize the MSC for writing */
  MSC_Init();
  /* Erase the page */
  /*
  aglin page
  */
  ret = erasePage(dwiAddr, FLASH_PAGE_SIZE);

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  if (ret != mscReturnOk)
  {
    MSC_Deinit( );
    return ret;
  }

  /* Write data to the userpage */
  ret = MSC_WriteWord((uint32_t *)dwiAddr, pciSrc, uwLen);

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  if (ret != mscReturnOk)
  {
    MSC_Deinit();
    return ret;
  }
  /* Deinitialize the MSC. This disables writing and locks the MSC */
  MSC_Deinit();
  #else
  eraseSector(dwiAddr, FLASH_PAGE_SIZE);
  pgmBurst(dwiAddr, pciSrc, wiDataLen);
  #endif
  
  return mscReturnOk;
}


/**********************************************************
 * read data on page of flash. 
 *
 * @param dwAbsAddr
 *    Address of page. Must be a valid flash address.
 * @param poBuf
 *    read data to this buf
 * @param dwisize
 *    read data word numbers
 * return read data length
 **********************************************************/
u_long eeBlockRead(u_long dwAbsAddr,void*poBuf,u_long dwiSize )
{
	u_long dwLength = 0;
	if(poBuf)
	{
		memcpy(poBuf,(void*)dwAbsAddr,dwiSize);
		dwLength = dwiSize;
	}
	return dwLength;
}





