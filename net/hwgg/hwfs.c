#include "contiki.h"
#include "basictype.h"
#include <string.h>
#include "hwfs.h"
#include "hwgg.h"
#include "common.h"



int hwfsFillFrame(u_char *pioBuf, const u_char *pcNetId, u_char ubDevID, u_char ubCmd, const u_char *pcData, u_char ubLen)
{
	int nLen = -1;
	int i = 0;
	u_short uwCrc = 0;
	HWFS_FRAME *pHwfs = NULL;
	if (pioBuf == NULL || ubLen > 8)
		return nLen;

	pHwfs = (HWFS_FRAME *)pioBuf;
	
	pHwfs->ubHwfsHead = HWGG_HEAD;
	memcpy(pHwfs->ubaNetID, pcNetId, HWFS_NETID_LEN);
	pHwfs->ubDevID = ubDevID;
	pHwfs->ubCMD = ubCmd;
	if (pcData != NULL && ubLen != 0)
	{
		memcpy(pHwfs->ubaData, pcData, ubLen);
		nLen = ubLen + HWFS_FRAME_FIX_LEN;
		i = ubLen;
	}
	else
	{
		nLen = HWFS_FRAME_FIX_LEN;
	}
	uwCrc = cyg_crc16((const u_char*)pHwfs->ubaNetID, nLen-HWFS_FRAME_HEAD_CRC_END_BYTES);
	pHwfs->ubaData[i++] = uwCrc&0xff;
	pHwfs->ubaData[i++] = (uwCrc>>8)&0xff;
	pHwfs->ubaData[i++] = HWGG_END;
	return nLen;
}



int hwfsFillToHostFrame(u_char *pioBuf, const u_char *pcNetId, u_char ubDevID, u_char ubRssi,u_char ubCmd, const u_char *pcData, u_char ubLen)
{
	int nLen = -1;
	int i = 0;
	u_short uwCrc = 0;
	HWFS_HOST_FRAME *pHwfs = NULL;
	if (pioBuf == NULL || ubLen > 8)
		return nLen;

	pHwfs = (HWFS_HOST_FRAME *)pioBuf;
	
	pHwfs->ubHwfsHead = HWGG_HEAD;
	memcpy(pHwfs->ubaNetID, pcNetId, HWFS_NETID_LEN);
	pHwfs->ubDevID = ubDevID;
	pHwfs->ubRSSI = ubRssi;
	pHwfs->ubCMD = ubCmd;
	if (pcData != NULL && ubLen != 0)
	{
		memcpy(pHwfs->ubaData, pcData, ubLen);
		nLen = ubLen + HWFS_FRAME_HOST_FIX_LEN;
		i = ubLen;
	}
	else
	{
		nLen = HWFS_FRAME_HOST_FIX_LEN;
	}
	uwCrc = cyg_crc16((const u_char*)pHwfs->ubaNetID, nLen-HWFS_FRAME_HEAD_CRC_END_BYTES);
	pHwfs->ubaData[i++] = uwCrc&0xff;
	pHwfs->ubaData[i++] = (uwCrc>>8)&0xff;
	pHwfs->ubaData[i++] = HWGG_END;
	return nLen;
}

/*
* \brief Change hex frame to ascii frame
* \param pioAsciiFrame, pointer to the buf that save ascii frame
* \param pciHexFrame , pointer to the buf that save hex frame
* \param ubHexFrameDataLen hex frame data len, not include frame header and frame end.
* \return -1 error, >0 the ascii frame total length
*/

int hwfsHexFrameToAscii(u_char *pioAsciiFrame, const u_char *pciHexFrame, u_char ubHexFrameDataLen)
{
	int nFrameL = 0;
	if (NULL == pioAsciiFrame || NULL == pciHexFrame)
	{
		return -1;
	}
	nFrameL = hex2ascii((const u_char *)&pciHexFrame[HWFS_DATA_START], (u_char*)&pioAsciiFrame[HWFS_DATA_START], ubHexFrameDataLen);
	pioAsciiFrame[0] = HWGG_HEAD;
	pioAsciiFrame[nFrameL+1] = HWGG_END;

	return (nFrameL+2);
}



/*
* \brief Change hex frame to ascii frame
* \param pioHexFrame, pointer to the buf that save hex frame
* \param pciAsciiFrame , pointer to the buf that save asccii frame
* \param ubAsciiFrameDataLen hex frame data len, not include frame header and frame end.
* \return -1 error, >0 the hex frame total length
*/

int hwfsAsciiFrameToHex(u_char *pioHexFrame, const u_char *pciAsciiFrame, u_char ubAsciiFrameDataLen)
{
	int nFrameL = 0;
	if (NULL == pioHexFrame || NULL == pciAsciiFrame)
	{
		return -1;
	}
	nFrameL = asciis2hex((u_char *)&pioHexFrame[HWFS_DATA_START], (const uint8 *)&pciAsciiFrame[HWFS_DATA_START], ubAsciiFrameDataLen);
	pioHexFrame[0] = HWGG_HEAD;
	pioHexFrame[nFrameL+1] = HWGG_END;

	return (nFrameL+2);
}

