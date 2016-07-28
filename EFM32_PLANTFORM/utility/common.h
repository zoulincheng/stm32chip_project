/**************************************************************************************************
  Filename:       common.h
  Revised:        $Date: 2013-07-18 13:54:23  $
  Revision:       $Revision: 00001 $

  Description:    16 Î»CRC Ð£ÑéÂë
**************************************************************************************************/

#ifndef COMMON_H
#define COMMON_H

/*********************************************************************
 * INCLUDES
 */
 #include "basictype.h"
/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * MACROS
 */



/*********************************************************************
 * FUNCTIONS
 */
extern uint16 cyg_crc16(unsigned char *puchMsg, uint16 usDataLen);
extern int hex_2_ascii( const uint8  *pidata, uint8 *poBuf, int len);
extern unsigned char ASCII_to_16(uint8 ch);
extern int BCD_to_hex(uint8* indata,uint8*outdata,int len);
extern uint16 BCD_to_hex_16bits(uint8* indata);
extern uint16 linkage_calc_crc16( uint8 * pucFrame, uint8 usLen );


/*********************************************************************
*********************************************************************/

#endif /* CRC_16_H */