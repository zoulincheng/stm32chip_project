#include "em32lg_config.h"
#include "basictype.h"
#include "contiki.h"
#include "sysprintf.h"

#include "hwlh_light.h"
#include "hwlh_z.h"

#include "lib/ringbuf.h"

#include "extgdb_376_2.h"

static struct ringbuf rxbuf;
static uint8_t rxbuf_data[128];



PROCESS(serial_rev_process, "serial_rev_process");

PROCESS_NAME(hwlh_uart_process);
extern process_event_t event_rev_msg;


/*---------------------------------------------------------------------------*/
int serial_receive_byte(unsigned char c)
{
	static uint8_t overflow = 0; /* Buffer overflow: ignore until END */

//	XPRINTF((0, "C = %02x\r\n", c));
	if(!overflow) 
	{
		/* Add character */
		if(ringbuf_put(&rxbuf, c) == 0) 
		{
			/* Buffer overflow: ignore the rest of the line */
			overflow = 1;
		}
	} 
	else 
	{
		/* Buffer overflowed:
		* Only (try to) add terminator characters, otherwise skip */
		if(ringbuf_put(&rxbuf, c) != 0) 
		{
			overflow = 0;
		}
	}
	/* Wake up consumer process */
	process_poll(&serial_rev_process);	
	return 1;
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_rev_process, ev, data)
{
	static HWLH_SERIAL_RX_ASCII_ST stHwlhRx;
	static HWLH_SERIAL_RX_HEX_ST stHwlhHex;
	static struct etimer et_rev_timeout;
	static int ptr;
	static volatile u_char ubFrameT;
	int c;
	const RF_NODE_PARAM_CONFIG *pcNodeRfParamCfg = NULL;
	static u_char ubNodeType;
	PROCESS_BEGIN( );
	//serial_line_event_message = process_alloc_event();
	ptr = 0;
	ubFrameT = 0;
	pcNodeRfParamCfg = (const RF_NODE_PARAM_CONFIG*)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	ubNodeType = pcNodeRfParamCfg->ubNodeType;
	while(1) 
	{
		/* get frame */
		//int c = ringbuf_get(&rx485buf);
		c = ringbuf_get(&rxbuf);
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			XPRINTF((6, "T_O\r\n"));
			ptr = 0;
			memset(&stHwlhRx, 0, sizeof(HWLH_SERIAL_RX_ASCII_ST));
			ubFrameT = 0;
		}
		if(c == -1) 
		{
			/* Buffer empty, wait for poll */
			PROCESS_YIELD();
		} 
		else 
		{
			if(ptr < 128-1) 
			{
				stHwlhRx.ubaBuf[ptr++] = (uint8_t)c;
				//XPRINTF((6, "ubaBuf[ptr] = %02x   %d\r\n ", stHwlhRx.ubaBuf[ptr-1], ptr));
				//frame start
				if ((ptr == 1)&&((stHwlhRx.ubaBuf[0] == HWLH_SQI)||(stHwlhRx.ubaBuf[0] == HL_HEARER)))
				{
					//set timeout 
					//Frame start
					XPRINTF((6, "start\r\n"));
					etimer_set(&et_rev_timeout, 500);
				}
				//header error
				if (stHwlhRx.ubaBuf[0] != HWLH_SQI)
				{
					if ((stHwlhRx.ubaBuf[0] != HL_HEARER))
					{
						ptr = 0;
						memset(&stHwlhRx, 0, sizeof(HWLH_SERIAL_RX_ASCII_ST));
						ubFrameT = 0;
						XPRINTF((6, "rst 1\r\n"));
					}
					else
					{
						if ((c == HL_END))
						{
							stHwlhRx.ubLen = ptr;
							memcpy(stHwlhHex.ubaBuf, stHwlhRx.ubaBuf, stHwlhRx.ubLen);
							stHwlhHex.ubLen = stHwlhRx.ubLen;
							//stHwlhHex = stHwlhRx;
							MEM_DUMP(0,"hl<-", stHwlhHex.ubaBuf, stHwlhHex.ubLen);
							process_post(&hwlh_uart_process, event_rev_msg, &stHwlhHex);
							etimer_stop(&et_rev_timeout);
							ptr = 0;
							memset(&stHwlhRx, 0, sizeof(HWLH_SERIAL_RX_ASCII_ST));
						}
					}
				}
				else
				{
					//XPRINTF((0, "buf %02x %d \r\n", stHwlhRx.ubaBuf[ptr-1], ptr));
					//Frame type
					if ((c == HWLH_EQI))
					{
						stHwlhRx.ubLen = ptr;
						stHwlhHex.ubLen = hwlhAsciiFrameToHex(stHwlhHex.ubaBuf, (const u_char *)stHwlhRx.ubaBuf, stHwlhRx.ubLen-2);
						//stHwlhHex = stHwlhRx;
						MEM_DUMP(0,"ra<-", stHwlhRx.ubaBuf, stHwlhRx.ubLen);
						MEM_DUMP(0,"hw<-", stHwlhHex.ubaBuf, stHwlhHex.ubLen);
						process_post(&hwlh_uart_process, event_rev_msg, &stHwlhHex);
						etimer_stop(&et_rev_timeout);
						ptr = 0;
						memset(&stHwlhRx, 0, sizeof(HWLH_SERIAL_RX_ASCII_ST));
					}
				}
			} 
			else
			{
				ptr = 0;
				memset(&stHwlhRx, 0, sizeof(HWLH_SERIAL_RX_ASCII_ST));
			}
		}
	}
  	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void serial_rev_init(void)
{
  ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));
  process_start(&serial_rev_process, NULL);
}


