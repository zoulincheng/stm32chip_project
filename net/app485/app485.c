#include "contiki.h"
#include "basictype.h"
#include "app485.h"
#include <string.h>
#include "sysprintf.h"
#include "lib/ringbuf.h"


process_event_t event_485_msg;
process_event_t event_485_rev_msg;

static APP_485_DATA app_485_tx;				//used to send frame to 485 bus
static APP_485_DATA app_485_rx;				//used to receive data from 485
static APP_485_DATA app_485_warn_msg;		//used to save msg to server that receive from 485 bus



static const u_char app485addr = 0x01;

PROCESS(app_485_rev_process, "app_485_rev_process");
PROCESS(app_bus_frame_process, "app_bus_frame_process");




/*
extern funtion and var
*/
PROCESS_NAME(sim900a_app_process);
PROCESS_NAME(ip_data_process);

extern process_event_t sim900_event_fire_tran;
extern process_event_t event_ip_tran;


u_char xorCHeckFrame(const u_char *ptb, u_char ubLen)
{
	int i = 0;
	u_char ubxorCheck = 0;
	for (i = 0; i < ubLen; i++)
	{
		ubxorCheck ^= ptb[i];
	}

	return ubxorCheck;
}



static int fill_485_header(u_char *pFrame,u_char cmd,u_char ubSrcAddr,u_char ubDestAddr)
{
	//fill frame header syn TB_FRAME_HEADER_LEN bytes
	u_char *pBuf = pFrame;
	int i = 0;
	TB_FRAME_COMMON *pTbComm = NULL;
	int nFrameL = 0;
	
	for (i = 0; i < TB_FRAME_HEADER_LEN; i++)
	{
		pBuf[i] = TB_SYN;
	}
	nFrameL += TB_FRAME_HEADER_LEN;

	pTbComm = (TB_FRAME_COMMON *)(pFrame+TB_FRAME_HEADER_LEN);
	pTbComm->ubCmd = cmd;
	pTbComm->ubSrcAddr = ubSrcAddr;
	pTbComm->ubDestAddr = ubDestAddr;
	return TB_COMMON_FIX_LEN+TB_FRAME_HEADER_LEN;
}

static int flll_485_soh_frame(u_char *pFrame, 
                              u_char cmd, 
                              u_char ubSrcAddr, 
                              u_char ubDestAddr, 
                              u_char pkg_no,
                              u_char type,
							  const u_char *pcData, 
							  u_char ubLen)
{
	TB_FRAME_SOH_DATA *pTb = NULL;
	u_char ubxorCheck = 0;
	int i = 0;
	int nFrameL = -1;
	
	if (pFrame == NULL)
		return -1;
	nFrameL = 0;
	
	//fill frame header 
	nFrameL += fill_485_header(pFrame, cmd, ubSrcAddr, ubDestAddr);
	
	pTb = (TB_FRAME_SOH_DATA *)(pFrame+TB_FRAME_HEADER_LEN);

	pTb->ubPkgNO = pkg_no;
	pTb->ubType = type;
	pTb->ubLen = 0;//first set len zero.
	nFrameL += 3;

	//fill frame data field
	if (pcData == NULL || ubLen == 0)
	{
		pTb->ubLen = 0;
	}
	else
	{	
		memcpy(pTb->ubaData, pcData, ubLen);
		nFrameL += ubLen;
		i = ubLen;						//buf position
	}
	
	//frame end and xor check field 
	ubxorCheck = xorCHeckFrame((const u_char *)pTb, nFrameL-TB_FRAME_HEADER_LEN);
	pTb->ubaData[i++] = TB_EOT;
	pTb->ubaData[i++] = ubxorCheck;
	nFrameL += 2;
	
	return nFrameL;
}




static int flll_485_bscsoh_frame(u_char *pFrame, 
                                 u_char cmd, 
                                 u_char ubSrcAddr, 
                                 u_char ubDestAddr, 
                                 u_char ubCmdno,
                                 const u_char *pcData, 
   							     u_char ubLen)
{
	TB_FRAME_BSC_DATA *pTb = NULL;
	u_char ubxorCheck = 0;
	int i = 0;
	int nFrameL = -1;
	
	if (pFrame == NULL)
		return -1;
	nFrameL = 0;

	//fill frame header 
	nFrameL += fill_485_header(pFrame, cmd, ubSrcAddr, ubDestAddr);	
	pTb = (TB_FRAME_BSC_DATA *)(pFrame+TB_FRAME_HEADER_LEN);

	pTb->ubCmdNo = ubCmdno;
	nFrameL += 1;

	//fill frame data field
	if (pcData == NULL || ubLen == 0)
	{

	}
	else
	{	
		memcpy(pTb->ubaData, pcData, ubLen);
		nFrameL += ubLen;
		i = ubLen;						//buf position
	}
	
	//frame end and xor check field 
	ubxorCheck = xorCHeckFrame((const u_char *)pTb, nFrameL-TB_FRAME_HEADER_LEN);
	pTb->ubaData[i++] = TB_EOT;
	pTb->ubaData[i++] = ubxorCheck;
	nFrameL += 2;
	
	return nFrameL;
}

static int fill_485_ack_frame(u_char *pFrame, u_char cmd, u_char ubSrcAddr, u_char ubDestAddr, u_char pkg_no)
{
	int nFrameL = -1;
	u_char ubxorCheck = 0;
	TB_FRAME_ACK *pTb = NULL;

	if (pFrame == NULL)
		return -1;
		
	nFrameL = 0;
	nFrameL += fill_485_header(pFrame, cmd, ubSrcAddr, ubDestAddr);

	pTb = (TB_FRAME_ACK *)(pFrame+TB_FRAME_HEADER_LEN);
	pTb->ubPkgNo = pkg_no;
	nFrameL += 1;
	ubxorCheck = xorCHeckFrame((const u_char *)pTb, nFrameL-TB_FRAME_HEADER_LEN);
	pTb->ubEot = TB_EOT;
	pTb->ubXor = ubxorCheck;
	nFrameL += 2;

	return nFrameL;
}



static int fill_485_sake_frame(u_char *pFrame, u_char cmd, u_char ubSrcAddr, u_char ubDestAddr, u_char type)
{
	int nFrameL = -1;
	u_char ubxorCheck = 0;
	TB_FRAME_SAKED *pTb = NULL;

	if (pFrame == NULL)
		return -1;
		
	nFrameL = 0;
	nFrameL += fill_485_header(pFrame, cmd, ubSrcAddr, ubDestAddr);

	pTb = (TB_FRAME_SAKED *)(pFrame+TB_FRAME_HEADER_LEN);
	pTb->ubType = type;
	nFrameL += 1;
	ubxorCheck = xorCHeckFrame((const u_char *)pTb, nFrameL-TB_FRAME_HEADER_LEN);
	pTb->ubEot = TB_EOT;
	pTb->ubXor = ubxorCheck;
	nFrameL += 2;

	return nFrameL;
}


static int fill_485_common_frame(u_char *pFrame, u_char cmd, u_char ubSrcAddr, u_char ubDestAddr)
{
	int nFrameL = -1;
	u_char ubxorCheck = 0;
	TB_FRAME_COMMON_FRAME *pTb = NULL;

	if (pFrame == NULL)
		return -1;
		
	nFrameL = 0;
	nFrameL += fill_485_header(pFrame, cmd, ubSrcAddr, ubDestAddr);

	pTb = (TB_FRAME_COMMON_FRAME *)(pFrame+TB_FRAME_HEADER_LEN);
	ubxorCheck = xorCHeckFrame((const u_char *)pTb, nFrameL-TB_FRAME_HEADER_LEN);
	pTb->ubEot = TB_EOT;
	pTb->ubXor = ubxorCheck;
	nFrameL += 2;

	return nFrameL;
}


static bool isApp485Frame(const u_char *pcFrame)
{
	const APP_485_DATA *pApp485 = (const APP_485_DATA *)pcFrame;
	//frame headder check
	if (pApp485->ubaData[0] != TB_SYN || pApp485->ubaData[1] != TB_SYN)
		return false;
		
	//frame end check
	if (pApp485->ubaData[pApp485->ubLen-2] != TB_EOT)
		return false;
	return true;
} 




static const u_char *find_485_frame_cmd(const u_char *pcFrame)
{
	int i = 0; 
	const APP_485_DATA *pApp485 = (const APP_485_DATA *)pcFrame;
	if (pcFrame == NULL)
		return NULL;
	for (i = 0; i < pApp485->ubLen-1; i++)
	{
		if (pApp485->ubaData[i] == TB_SYN && pApp485->ubaData[i+1] != TB_SYN)
		{
			return (const u_char *)&pApp485->ubaData[i+1];
		}
	}

	return NULL;
}

static bool isMy485Frame(const u_char *pcFrame)
{
	const TB_FRAME_COMMON *pTCom = NULL;
	if (isApp485Frame(pcFrame))
	{
		pTCom = (const TB_FRAME_COMMON *)find_485_frame_cmd(pcFrame);
		if (pTCom != NULL)
		{
			if (pTCom->ubDestAddr == app485addr)
			{
				return true;
			}
		}
	}

	return false;
}



static int soh_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;
	APP_485_DATA *pwarnMsg = &app_485_warn_msg;
	TB_WARN_MSG *pTbWarnMsg = (TB_WARN_MSG *)pwarnMsg->ubaData;

	//process first msg
	if (pSOH->ubPkgNO == TB_WARN_FIRST_SEQ)
	{
		memset(pwarnMsg, 0, sizeof(*pwarnMsg));
		
		pTbWarnMsg->ubLen = pSOH->ubLen;
		pTbWarnMsg->ubType = pSOH->ubType;
		memcpy(pTbWarnMsg->ubaData, pSOH->ubaData, pSOH->ubLen);
		
		pwarnMsg->ubLen = pSOH->ubLen+2;
		
		MEM_DUMP(6,"TB01", pTbWarnMsg, pTbWarnMsg->ubLen+2);
	}
	//process msg is not first
	else
	{
		memcpy(&pTbWarnMsg->ubaData[pTbWarnMsg->ubLen], pSOH->ubaData, pSOH->ubLen);
		pwarnMsg->ubLen += pSOH->ubLen;
		pTbWarnMsg->ubLen += pSOH->ubLen;

		MEM_DUMP(6, "TB02",pSOH->ubaData, pSOH->ubLen);
		MEM_DUMP(6, "TB12",pwarnMsg->ubaData, pwarnMsg->ubLen);
	}
	nFrameL = fill_485_ack_frame(pApp485Tx->ubaData, TB_ACK, app485addr, pSOH->tbCommon.ubSrcAddr, pSOH->ubPkgNO);
	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;
}

static int tbsack_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;
	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_SAKED, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);
	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;
}

static int tblink_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;
	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_LINKED, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);
	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;	
}

static int tbenq_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;
	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_ACK, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);
	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;		
}

static int tbnul_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;

	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_NULACK, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);

	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;		
}


static int tack_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;

	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_EXT, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);

	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;			
}


static int tbunlink_frame_process(APP_485_DATA *pApp485Tx, const u_char *pcTbFrame)
{
	int nFrameL = -1;
	const TB_FRAME_SOH_DATA *pSOH = (const TB_FRAME_SOH_DATA *)pcTbFrame;

	nFrameL = fill_485_sake_frame(pApp485Tx->ubaData, TB_UNLINKED, app485addr, pSOH->tbCommon.ubSrcAddr, 0x00);

	if (nFrameL > 0)
	{
		pApp485Tx->ubLen = nFrameL;
	}
	return nFrameL;		
}

void app485_frame_process(u_char *pioBuf, const u_char *pcData)
{
	const TB_FRAME_COMMON *pcTbCommon = NULL;
	int nFrameL = -1;
	bool is485 = false;
	const u_char *pFrame = NULL;
	
	const APP_485_DATA *pApp485 = (const APP_485_DATA *)pcData;
	APP_485_DATA *pApp485Tx = (APP_485_DATA *)pioBuf;

	if (pioBuf == NULL || pcData == NULL)
		return;

	is485 = isApp485Frame(pcData);
	if (!is485)
		return;

	pFrame = (const u_char *)find_485_frame_cmd(pcData);

	if (pFrame != NULL)
	{
		MEM_DUMP(9, "<-48",pApp485->ubaData, pApp485->ubLen);
		pcTbCommon = (const TB_FRAME_COMMON *)pFrame;
		switch(pcTbCommon->ubCmd)
		{
			case TB_SOH:		//数据
				nFrameL = soh_frame_process(pApp485Tx, pFrame);
				break;
			case TB_BCSOH:
				break;
			case TB_EXT:
				break;
			case TB_ACK:	//接收确认
				nFrameL = tack_frame_process(pApp485Tx, pFrame);
				break;
			case TB_NAK:
				break;
			case TB_NUL:	//结束
				nFrameL = tbnul_frame_process(pApp485Tx, pFrame);
				break;
			case TB_NULACK:
				break;
			case TB_ENQ:	//查询
				nFrameL = tbenq_frame_process(pApp485Tx, pFrame);
				break;
			case TB_SAK:	//握手
				nFrameL = tbsack_frame_process(pApp485Tx, pFrame);
				break;
			case TB_SAKED:
				break;
			case TB_LINK:	//连接
				nFrameL = tblink_frame_process(pApp485Tx, pFrame);
				break;
			case TB_LINKED:
				break;
			case TB_UNLINK:	//终止连接
				nFrameL = tbunlink_frame_process(pApp485Tx, pFrame);
				/*post data to */
				process_post(&sim900a_app_process, sim900_event_fire_tran, &app_485_warn_msg);
				process_post(&ip_data_process, event_ip_tran, &app_485_warn_msg);
				break;
			case TB_UNLINKED:
				break;
			default:
				break;
		}
	}

	//send event msg to 
	if (nFrameL > 0)
	{
		uart2_send_bytes(pApp485Tx->ubaData, pApp485Tx->ubLen);
		MEM_DUMP(9, "->48", pApp485Tx->ubaData, pApp485Tx->ubLen);
	}
}


static void app485frameHandler(process_event_t ev, process_data_t data)
{
	if (ev == event_485_msg && data != NULL)
	{
		app485_frame_process((u_char *)&app_485_tx, (const u_char *)data);
	}
}



PROCESS_THREAD(app_bus_frame_process, ev, data)
{

	PROCESS_BEGIN();
	XPRINTF((2, "app_bus_frame_process\r\n"));

	while(1)
	{
		PROCESS_YIELD();
		app485frameHandler(ev, data);
	}
	PROCESS_END();
}





static struct ringbuf ringuartbuf;
static uint8_t uartbuf[128];

/*---------------------------------------------------------------------------*/
int uart485_input_byte(unsigned char c)
{
	static uint8_t overflow = 0; /* Buffer overflow: ignore until END */
	if(!overflow)
	{
		/* Add character */
		if(ringbuf_put(&ringuartbuf, c) == 0)
		{
			/* Buffer overflow: ignore the rest of the line */
			overflow = 1;
		}
	}
	else
	{
		/* Buffer overflowed:
		* Only (try to) add terminator characters, otherwise skip */
		if(ringbuf_put(&ringuartbuf, c) != 0)
		{
			overflow = 0;
		}
	}
	/* Wake up consumer process */
	process_poll(&app_485_rev_process);
	
	return 1;
}

void uart485_init(void)
{
	
	ringbuf_init(&ringuartbuf, uartbuf, sizeof(uartbuf));
	process_start(&app_485_rev_process, NULL);
	Uart_485SetInput(uart485_input_byte);
}



bool isFrameEnd(const u_char *pFrame, u_char lastIndex)
{
	if ((pFrame[lastIndex] == TB_EOT && pFrame[lastIndex-1] != TB_DLE) 
	    || (pFrame[lastIndex] == TB_EOT && pFrame[lastIndex-1] == TB_DLE && pFrame[lastIndex-2] == TB_DLE))
	{
		return true;
	}

	return false;
}


static void delDleFromFrame(APP_485_DATA *p485, const u_char *pcFrame, u_char oldLen)
{
	
}



PROCESS_THREAD(app_485_rev_process, ev, data)
{
	static char buf[128];
	static APP_485_DATA stApp485;
	static struct etimer et_rev_timeout;
	static int ptr = 0;
	static u_char frameEnd = 0;
	int c;

	PROCESS_BEGIN();
	XPRINTF((2, "app_485_rev_process\r\n"));
	event_485_msg = process_alloc_event( );
	//event_485_rev_msg = process_alloc_event( );

	while(1)
	{
		c = ringbuf_get(&ringuartbuf);
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			XPRINTF((6, "time out\n"));
			memset(buf, 0 ,sizeof(buf));
			//MEM_DUMP(6, "DATA", buf, ptr);
			ptr = 0;
		}

		if(c == -1)
		{
			/* Buffer empty, wait for poll */
			PROCESS_YIELD();
		}
		else
		{
			//XPRINTF((9, "%02x\r\n", c));
			if (ptr == 0 && (uint8_t)c != TB_SYN)
			{
				ptr = 0;
			}
			else
			{
				buf[ptr++] = (uint8_t)c;
				//XPRINTF((6, "c = %02x\n", (u_char)c));
				//set timer for receie time out
				if (ptr == 1 && buf[0] == TB_SYN)
				{
					etimer_set(&et_rev_timeout, 100);
					frameEnd = 0;
				}
				//frame to long, error
				if (ptr >= 127)
				{
					ptr = 0;
				}

				#if 1
				//frame end
				if (isFrameEnd((const u_char*)buf, ptr-1))
				{
					frameEnd = ptr;
				}
				//receive frame xorcheck
				if ((frameEnd+1) == ptr && frameEnd > 7 )
				{
					//MEM_DUMP(6, "BUS4", buf, ptr);
					etimer_stop(&et_rev_timeout);
					memcpy(stApp485.ubaData, buf, ptr);
					stApp485.ubLen = ptr;
					process_post(&app_bus_frame_process, event_485_msg, &stApp485);
					//app485_frame_process((u_char *)&app_485_tx, (const u_char *)&stApp485);
					memset(buf, 0 ,sizeof(buf));
					
					ptr = 0;
				}
				#endif
			}
		}
	}
	PROCESS_END();
}


void app485Init(void)
{
	uart485_init( );
	process_start(&app_bus_frame_process, NULL);
}

