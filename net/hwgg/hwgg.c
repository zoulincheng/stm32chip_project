#include "contiki.h"
#include "basictype.h"
#include <string.h>
#include "hwgg.h"
#include "common.h"

static const u_char ubaDstAddr[2]={0xff,0xff};
static const u_char ubaSrcAddr[2]={0xff,0xff};
static const u_char ubaEndAddr[2]={0xff,0xff};

int hwggFillFrame(u_char *pioBuf, u_char ubSeq,u_char ubCMD, const u_char *pcData, u_char ubLen)
{
	int nLen = -1;
	int i = 0;
	u_short uwCrc = 0;
	HWGG_FRAME *pHwgg = NULL;
	if (pioBuf == NULL)
	{
		return nLen;
	}

	pHwgg = (HWGG_FRAME *)pioBuf;
	pHwgg->ubHwggHead = HWGG_HEAD;
	pHwgg->ubLen = 0;
	pHwgg->ubPanId = ubSeq;

	//addr 
	memcpy(pHwgg->ubaDstAddr, ubaDstAddr, HWGG_NETADDR_LEN);
	memcpy(pHwgg->ubaSrcAddr, ubaSrcAddr, HWGG_NETADDR_LEN);
	memcpy(pHwgg->ubaEndAddr, ubaEndAddr, HWGG_NETADDR_LEN);
	pHwgg->ubSeq = ubSeq;
	pHwgg->ubCmd = ubCMD;
	if (pcData != NULL && ubLen != 0)
	{
		nLen = 0;
		memcpy(pHwgg->ubaData, pcData, ubLen);
		nLen += HWGG_FRAME_FIX_LEN+ubLen-HWGG_HEAD_END_CRC_LEN;
		i = ubLen;
	}
	else
	{
		nLen = HWGG_FRAME_FIX_LEN-HWGG_HEAD_END_CRC_LEN;
		
	}
	pHwgg->ubLen = nLen;
	uwCrc = cyg_crc16((const u_char *)&pHwgg->ubLen, pHwgg->ubLen);
	pHwgg->ubaData[i++] = (uwCrc>>8) & 0xff;
	pHwgg->ubaData[i++] = uwCrc & 0xff;
	pHwgg->ubaData[i++] = HWGG_END;

	return (nLen + HWGG_HEAD_END_CRC_LEN);
}




bool hwggCheckFrame(const u_char *pciFrame)
{
	u_short uwCrc = 0;
	u_short uwCrcF = 0;
	const HWGG_FRAME *pHwgg = NULL;

	u_char ubLen = 0;
	const u_char *pCheck = NULL; 

	if (pciFrame == NULL)
		return false;

	pHwgg = (const HWGG_FRAME *)pciFrame;

	//check frame head
	if (pHwgg->ubHwggHead != HWGG_HEAD)
	{
		return false;
	}

	//check cmd
	if (pHwgg->ubCmd != HWGG_CMD_HEART && pHwgg->ubCmd != HWGG_CMD_LOW_VOLTAGE && pHwgg->ubCmd != HWGG_CMD_WARN)
	{
		return false;
	}

	//total frame length
	ubLen = pHwgg->ubLen + HWGG_HEAD_END_CRC_LEN;
	//check frame end
	pCheck = (pciFrame+ubLen-1);
	//XPRINTF((10, "END = %02x\n", pCheck[0]));
	if (pCheck[0] != HWGG_END)
	{
		return false;
	}

	pCheck = (pciFrame+ubLen-3);
	//check  crc
	uwCrc = cyg_crc16((const u_char *)&pHwgg->ubLen, pHwgg->ubLen);
	uwCrcF = (pCheck[0]<<8) | (pCheck[1]);
	//XPRINTF((10, "CRC = %04x, crcf = %04x\n", uwCrc, uwCrcF));
	
	if (uwCrc != uwCrcF)
	{
		return false;
	}

	return true;
}




























