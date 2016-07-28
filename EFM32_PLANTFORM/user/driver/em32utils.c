#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"

/**********************************************************
 * Retrieves total flash size from the DI page of the target
 * 
 * @returns
 *    The flash size in bytes
 **********************************************************/
int getFlashSize(void)
{  
  /* Read memory size from the DI page */
  uint32_t msize = readMem((uint32_t)&(DEVINFO->MSIZE));
  
  /* Retrieve flash size (in kB) */
  uint32_t flashSize = (msize & _DEVINFO_MSIZE_FLASH_MASK) >> _DEVINFO_MSIZE_FLASH_SHIFT;
  
  /* Return value in bytes */
  return flashSize * 1024;
}

/**********************************************************
 * Retrieves page size from the DI page of the target
 * 
 * @returns
 *    The page size in bytes
 **********************************************************/
int getPageSize(void)
{  
  uint32_t part = readMem((uint32_t)&(DEVINFO->PART));
  uint32_t msize = readMem((uint32_t)&(DEVINFO->MSIZE));
  
  uint32_t prodRev = (part & _DEVINFO_PART_PROD_REV_MASK) >> _DEVINFO_PART_PROD_REV_SHIFT;
  uint32_t family = (part &  _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT;
  
  uint32_t pageSize;
  
  /* Figure out the size of the flash pages. */
  switch(family)
  {
  case _DEVINFO_PART_DEVICE_FAMILY_GG:          /* Giant Gecko   */
    if (prodRev < 13)
    {
      /* All Giant Gecko rev B, with prod rev. < 13 use 2048 as page size, not 4096 */
      pageSize = 2048;
    } 
    else
    {
      pageSize = 4096;
    }
    break;
    
  case _DEVINFO_PART_DEVICE_FAMILY_LG:          /* Leopard Gecko */
  case _DEVINFO_PART_DEVICE_FAMILY_WG:          /* Wonder Gecko  */  
    pageSize = 2048;
    break;

  case _DEVINFO_PART_DEVICE_FAMILY_G:
  case _DEVINFO_PART_DEVICE_FAMILY_TG:
    pageSize = 512;
    break;
    
  case _DEVINFO_PART_DEVICE_FAMILY_ZG:
    pageSize = 1024;
    break;

  default:
    pageSize = 512;
    break;
  }
  
  return pageSize;
}

/**********************************************************
 * Retrieve the device name from the DI page of the target
 * 
 * @param deviceName[out]
 *    Device name is stored in this buffer when 
 *    the function returns. The calling function is
 *    responsible for allocating memory for the string
 **********************************************************/
void getDeviceName(char deviceName[])
{
  char familyCode[3];
  
  uint32_t part = readMem((uint32_t)&(DEVINFO->PART));
  uint32_t msize = readMem((uint32_t)&(DEVINFO->MSIZE));
  
  uint32_t flashSize = (msize & _DEVINFO_MSIZE_FLASH_MASK) >> _DEVINFO_MSIZE_FLASH_SHIFT;
  uint32_t family = (part &  _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT;  
  uint32_t partNum = (part &  _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT;  
  
  switch (family)
  {
  case _DEVINFO_PART_DEVICE_FAMILY_GG:          /* Giant Gecko   */
    xsprintf(familyCode, "%s", "GG");
    break;
  case _DEVINFO_PART_DEVICE_FAMILY_LG:          /* Leopard Gecko */
    xsprintf(familyCode, "%s", "LG");
    break;
  case _DEVINFO_PART_DEVICE_FAMILY_WG:          /* Wonder Gecko  */
    xsprintf(familyCode, "%s", "WG");
    break;
  case _DEVINFO_PART_DEVICE_FAMILY_G:           /* Gecko */
    xsprintf(familyCode, "%s", "G");
    break;
  case _DEVINFO_PART_DEVICE_FAMILY_TG:          /* Tiny Gecko */
    xsprintf(familyCode, "%s", "TG");
    break;
  case _DEVINFO_PART_DEVICE_FAMILY_ZG:          /* Zero Gecko */
    xsprintf(familyCode, "%s", "ZG");
    break;
  default:
    xsprintf(familyCode, "%s", "XX");            /* Unknown family */
    break;
  }
  
  xsprintf(deviceName, "EFM32%s%dF%d", familyCode, partNum, flashSize);
}


