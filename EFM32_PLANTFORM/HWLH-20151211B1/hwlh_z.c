#include "basictype.h"

#include "hwlh_light.h"
#include "hwlh_z.h"

#include "sysprintf.h"

#include "utility.h"
#include "common.h"

/*

*/


/*
* \brief Fill the frame to buf
* \param pioFrame The buf is used to save frame
* \param pciAddr  pciAddr[0], pciAddr[1] is node mac, pciaddr[2] is devid
* \param ubCmd	  Frame cmd
* \param piData   Data need to fill the frame
* \param ubDataLen data length
* \return -1 error, > 0 frame length
* \note all the data format is hex
*/

//this frame is from PC 
int hwlhFillRxFrame(u_char *pioFrame, const u_char *pciAddr, u_char ubCmd, const u_char *piData, u_char ubDataLen)
{
	HWLH_FRAME_HEX_RX_ST *pFrame = NULL;
	int nFrameL = 0;
	int i = 0;
	u_short ubCRC = 0;
	
	if (NULL == pioFrame || NULL == pciAddr )
	{
		return  HWLH_FRAME_ERROR;
	}

	pFrame = (HWLH_FRAME_HEX_RX_ST *)pioFrame;
	//fill header
	pFrame->ubSOI = HWLH_SQI;
	nFrameL++;
	//fill addr
	for (i = 0; i < HWLH_ADDR_LEN; i++)
	{
		pFrame->ubaAddr[i] = pciAddr[i];
		nFrameL++;
	}

	//fill cmd
	pFrame->ubCMD = ubCmd;
	nFrameL++;

	i = 0;
	if ((piData!=NULL) && (ubDataLen != 0))
	{
		for (i = 0; i < ubDataLen; i++)
		{
			pFrame->ubaData[i] = piData[i];
			nFrameL++;
		}
	}

	//Fill crc
	ubCRC = cyg_crc16((u_char *)pFrame->ubaAddr,nFrameL-HWLH_CAC_CRC_SUB_LEN);
	pFrame->ubaData[i++] = ubCRC;
	nFrameL++;
	pFrame->ubaData[i++] = ubCRC>>8;
	nFrameL++;

	//fill frame end
	pFrame->ubaData[i++] = HWLH_EQI;
	nFrameL++;

	return nFrameL;
}




/*

*/


/*
* \brief Fill the frame to buf
* \param pioFrame The buf is used to save frame
* \param pciAddr  pciAddr[0], pciAddr[1] is node mac, pciaddr[2] is devid
* \param ubCmd	  Frame cmd
* \param piData   Data need to fill the frame
* \param ubDataLen data length
* \return -1 error, > 0 frame length
* \note all the data format is hex
*/

//this frame is from node 
int hwlhFillTxFrame(u_char *pioFrame, const u_char *pciAddr, u_char ubRssi,u_char ubCmd, const u_char *piData, u_char ubDataLen)
{
	HWLH_FRAME_HEX_TX_ST *pFrame = NULL;
	int nFrameL = 0;
	int i = 0;
	u_short ubCRC = 0;
	
	if (NULL == pioFrame )
	{
		return  HWLH_FRAME_ERROR;
	}

	pFrame = (HWLH_FRAME_HEX_TX_ST *)pioFrame;
	//fill header
	pFrame->ubSOI = HWLH_SQI;
	nFrameL++;
	//fill addr
	if (pciAddr == NULL)
	{
		for (i = 0; i < HWLH_ADDR_LEN; i++)
		{
			pFrame->ubaAddr[i] = 0x00;
			nFrameL++;
		}
	}
	else
	{
		for (i = 0; i < HWLH_ADDR_LEN; i++)
		{
			pFrame->ubaAddr[i] = pciAddr[i];
			nFrameL++;
		}
	}
	//fill rssi
	pFrame->ubRssi = ubRssi;
	nFrameL++;
	
	//fill cmd
	pFrame->ubCMD = ubCmd;
	nFrameL++;

	i = 0;
	if ((piData!=NULL) && (ubDataLen != 0))
	{
		for (i = 0; i < ubDataLen; i++)
		{
			pFrame->ubaData[i] = piData[i];
			nFrameL++;
		}
	}

	//Fill crc
	ubCRC = cyg_crc16((u_char *)pFrame->ubaAddr,nFrameL-HWLH_CAC_CRC_SUB_LEN);
	pFrame->ubaData[i++] = ubCRC;
	nFrameL++;
	pFrame->ubaData[i++] = ubCRC>>8;
	nFrameL++;

	//fill frame end
	pFrame->ubaData[i++] = HWLH_EQI;
	nFrameL++;

	return nFrameL;
}





/*
* \brief Change hex frame to ascii frame
* \param pioAsciiFrame, pointer to the buf that save ascii frame
* \param pciHexFrame , pointer to the buf that save hex frame
* \param ubHexFrameDataLen hex frame data len, not include frame header and frame end.
* \return -1 error, >0 the ascii frame total length
*/

int hwlhHexFrameToAscii(u_char *pioAsciiFrame, const u_char *pciHexFrame, u_char ubHexFrameDataLen)
{
	int nFrameL = 0;
	if (NULL == pioAsciiFrame || NULL == pciHexFrame)
	{
		return HWLH_FRAME_ERROR;
	}
	nFrameL = hex_2_ascii((const u_char *)&pciHexFrame[HWLH_DATA_START], (u_char*)&pioAsciiFrame[HWLH_DATA_START], ubHexFrameDataLen);
	pioAsciiFrame[0] = HWLH_SQI;
	pioAsciiFrame[nFrameL+1] = HWLH_EQI;

	return (nFrameL+2);
}




/*
* \brief Change hex frame to ascii frame
* \param pioAsciiFrame, pointer to the buf that save ascii frame
* \param pciHexFrame , pointer to the buf that save hex frame
* \param ubHexFrameDataLen hex frame data len, not include frame header and frame end.
* \return -1 error, >0 the ascii frame total length
*/

int hwlhAppHexFrameToAscii(u_char *pioAsciiFrame, const u_char *pciHexFrame, u_char ubHexFrameDataLen)
{
	int nFrameL = 0;
	if (NULL == pioAsciiFrame || NULL == pciHexFrame)
	{
		return HWLH_FRAME_ERROR;
	}
	nFrameL = hex_2_ascii((const u_char *)pciHexFrame[1], (u_char*)&pioAsciiFrame[1], ubHexFrameDataLen);
	pioAsciiFrame[0] = HL_HEARER;
	pioAsciiFrame[nFrameL+1] = HL_END;

	return (nFrameL+2);
}





/*
* \brief Change hex frame to ascii frame
* \param pioHexFrame, pointer to the buf that save hex frame
* \param pciAsciiFrame , pointer to the buf that save asccii frame
* \param ubAsciiFrameDataLen hex frame data len, not include frame header and frame end.
* \return -1 error, >0 the hex frame total length
*/

int hwlhAsciiFrameToHex(u_char *pioHexFrame, const u_char *pciAsciiFrame, u_char ubAsciiFrameDataLen)
{
	int nFrameL = 0;
	if (NULL == pioHexFrame || NULL == pciAsciiFrame)
	{
		return HWLH_FRAME_ERROR;
	}
	nFrameL = ascii_2_hex((u_char *)&pioHexFrame[HWLH_DATA_START], (const uint8 *)&pciAsciiFrame[HWLH_DATA_START], ubAsciiFrameDataLen);
	pioHexFrame[0] = HWLH_SQI;
	pioHexFrame[nFrameL+1] = HWLH_EQI;

	return (nFrameL+2);
}


/*
* \brief Fill the frame to buf
* \param pioFrame The buf is used to save frame
* \param piData   Data need to fill the frame
* \param ubDataLen data length
* \return -1 error, > 0 frame length
* \note all the data format is hex
*/

//this frame is from PC 
int hwlhFillHLFrame(u_char *pioFrame,  const u_char *piData, u_char ubDataLen)
{
	int nFrameL = 0;
	u_char *pBuf = pioFrame;
	int i = 0;
	int j = 0;
	u_short ubCRC = 0;
	
	if (NULL == pioFrame )
	{
		return  HWLH_FRAME_ERROR;
	}

	//MEM_DUMP(10, "HLi", piData, ubDataLen);
	//fill header
	pBuf[i++] = HL_HEARER;
	nFrameL++;

	//fill data
	if ((piData!=NULL) && (ubDataLen != 0))
	{
		for (j = 0; j < ubDataLen; j++)
		{
			pBuf[i++] = piData[j];
			nFrameL++;
		}
	}

	//Fill crc
	ubCRC = cyg_crc16((u_char *)&pBuf[1],nFrameL-HWLH_CAC_CRC_SUB_LEN);
	pBuf[i++] = ubCRC;
	nFrameL++;
	
	pBuf[i++] = ubCRC>>8;
	nFrameL++;

	//fill frame end
	pBuf[i++] = HL_END;
	nFrameL++;

	//MEM_DUMP(10, "HLo", pBuf, nFrameL);
	return nFrameL;
}


