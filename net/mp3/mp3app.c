#include "contiki.h"
#include "basictype.h"
#include "mp3app.h"
#include "string.h"
#include "lib/ringbuf.h"
#include "sysprintf.h"


PROCESS(mp3_rev_process, "mp3_rev");
PROCESS(mp3_msg_process, "mp3_msg");

static u_char mp3xorcheck(const u_char *pcData, u_char len)
{
	int i = 0;
	u_char ubxorCheck = 0;
	for (i = 0; i < len; i++)
	{
		ubxorCheck ^= pcData[i];
	}

	return ubxorCheck;
}

/*
#define MP3_CMD_PLAY		0x11		//播放						无
#define MP3_CMD_PAUSE		0x12		//暂停						无
#define MP3_CMD_NEXT		0x13		//下一曲
#define MP3_CMD_PREV		0x14		//上一曲
#define MP3_CMD_V_ADD		0x15		//音量加
#define MP3_CMD_V_SUB		0x16		//音量减
#define MP3_CMD_RST			0x19		//复位
#define MP3_CMD_SPEED		0x1a		//快进
#define MP3_CMD_NSPEED		0x1b		//快退
#define MP3_CMD_PLAY_PAUSE	0x1c	//播放/暂停
#define MP3_CMD_STOP		0x1e		//停止
*/
int mp3fillCmdNoParam(u_char *pbuf, u_char ubcmd)
{
	MP3_CON_NO_PARAM *pFrame = NULL;

	if (pbuf == NULL)
	{
		return -1;
	}

	pFrame = (MP3_CON_NO_PARAM *)pbuf;

	pFrame->comHead.ubHead = MP3_HEAD;
	pFrame->comHead.ubLen = 3;
	pFrame->comHead.ubOPT = ubcmd;

	pFrame->ubXorCode = mp3xorcheck((u_char *)&pFrame->comHead.ubLen, 2);
	pFrame->ubEnd = MP3_END;


	return 5;
}


/*
//CMD详解							对应功能				参数(8位HEX)
#define MP3_CMD_SET_V			0x31		//设置音量				0-30级可调(音量掉电记忆)
#define MP3_CMD_SET_EQ			0x32		//设置EQ				0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS)
#define MP3_CMD_SET_LOOP_MODE	0x33	//设置循环模式		0-4(全盘/文件夹/单曲/随机/不循环
#define MP3_CMD_CHANGE_DIR		0x34	//文件夹切换		0（上一文件夹），1(下一文件夹) ，flash内没有此功能
#define MP3_CMD_CHANGE_DEV		0x35	//设备切换			0（U盘），1（SD）
#define MP3_CMD_ADKEY_UP		0x36	//ADKEY软件上拉		1开上拉（10K电阻），0关上拉，默认0
#define MP3_CMD_ADKEY_EN		0x37	//ADKEY使能			1开起，0关闭，默认1
#define MP3_CMD_BUSY_LEVL		0x38	//BUSY电平切换  1为播放输出高电平，0为播放输出低电平，默认1
*/
int mp3fillCmdOneByteParam(u_char *pbuf, u_char ubcmd, u_char param)
{

	MP3_CON_B_PARAM *pFrame = NULL;

	if (pbuf == NULL)
	{
		return -1;
	}	

	pFrame = (MP3_CON_B_PARAM *)pbuf;

	pFrame->comHead.ubHead = MP3_HEAD;
	pFrame->comHead.ubLen = 4;
	pFrame->comHead.ubOPT = ubcmd;
	pFrame->ubParam = param;
	pFrame->ubXorCode = mp3xorcheck((u_char *)&pFrame->comHead.ubLen, 3);

	pFrame->ubEnd = MP3_END;


	return 6;
}


int mp3fillCmdTwoByteParam(u_char *pbuf, u_char ubcmd, u_short param)
{

	MP3_CON_W_PARAM *pFrame = NULL;

	if (pbuf == NULL)
	{
		return -1;
	}
	pFrame = (MP3_CON_W_PARAM*)pbuf;
	pFrame->comHead.ubHead = MP3_HEAD;
	pFrame->comHead.ubLen = 5;
	pFrame->comHead.ubOPT = ubcmd;
	pFrame->ubParamH = (param>>8)&0xff;
	pFrame->ubParamL = (param)&0xff;
	pFrame->ubXorCode = mp3xorcheck((u_char *)&pFrame->comHead.ubLen, 3);
	pFrame->ubEnd = MP3_END;

	return 7;
}



static struct ringbuf ringuartbuf;
static uint8_t uartbuf[128];

/*---------------------------------------------------------------------------*/
int mp3_uart_input_byte(unsigned char c)
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
	process_poll(&mp3_rev_process);
	return 1;
}

static void mp3_uart_init(void)
{
	ringbuf_init(&ringuartbuf, uartbuf, sizeof(uartbuf));
	Uart_Mp3SetInput(mp3_uart_input_byte);
}




static u_char ubamp3[32]={0x00};

PROCESS_THREAD(mp3_msg_process, ev, data)
{
	static struct etimer et_mp3;
	int nFrameL = 0;
	PROCESS_BEGIN();
	XPRINTF((10, "mp3_msg_process\r\n"));
	
	while(1)
	{
		/*
		etimer_set(&et_mp3, 10*1000);
		nFrameL = mp3fillCmdNoParam(ubamp3, MP3_CMD_PLAY);
		MEM_DUMP(5, "MP->",ubamp3, nFrameL);
		uart5_send_bytes(ubamp3, nFrameL);
		PROCESS_WAIT_UNTIL(etimer_expired(&et_mp3));
		
		etimer_set(&et_mp3, 10*1000);
		nFrameL = mp3fillCmdNoParam(ubamp3, MP3_CMD_STOP);
		MEM_DUMP(5, "MP->",ubamp3, nFrameL);
		uart5_send_bytes(ubamp3, nFrameL);
		PROCESS_WAIT_UNTIL(etimer_expired(&et_mp3));
		*/
		PROCESS_YIELD();
		
	}
	PROCESS_END();
}




PROCESS_THREAD(mp3_rev_process, ev, data)
{
	static char buf[128];
	static struct etimer et_rev_timeout;
	static int ptr = 0;
	int c;

	PROCESS_BEGIN();
	XPRINTF((10, "mp3_rev_process\r\n"));

	while(1)
	{
		c = ringbuf_get(&ringuartbuf);
		//XPRINTF((5, "%02x \n", (uint8_t)c));
		if ((ev == PROCESS_EVENT_TIMER)&&(etimer_expired(&et_rev_timeout)))
		{
			//memset(buf, 0 ,sizeof(buf));
			XPRINTF((5, "rev = %s\n", buf));
			ptr = 0;
		}

		if(c == -1)
		{
			/* Buffer empty, wait for poll */
			PROCESS_YIELD();
		}
		else
		{
			buf[ptr++] = (uint8_t)c;
			//XPRINTF((5, "%02x \n", (uint8_t)c));
			if (ptr < 128)
			{
				etimer_set(&et_rev_timeout, 500);
			}
			else
			{
				ptr = 0;
			}
		}
	}
	PROCESS_END();
}






void mp3init(void)
{
	mp3_uart_init();	
	process_start(&mp3_rev_process, NULL);
	process_start(&mp3_msg_process, NULL);
}





