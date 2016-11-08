#include "contiki.h"
#include "basictype.h"
#include "appkey.h"
#include "iodef.h"
#include "sysprintf.h"
#include "stm32f10x.h"


static process_event_t event_key_msg;
static process_event_t event_self_check;

PROCESS(key_msg_read_process, "key_msg");
PROCESS(key_msg_handler_process, "key_msg_handler");
PROCESS(dev_check_self_process, "check_self");

PROCESS_NAME(blink_process);


static u_short keyGetValue(void)
{
	return (KEY_PORT->IDR)&KEY_VALUE;
}



static void alarmKeyProcess(void)
{
	//ALARM_LED(0);
	FAULT_LED(0);
	BUZZER(0);
}

static void silenceKeyProcess(void)
{
	//ALARM_LED(1);
	FAULT_LED(1);
	BUZZER(1);
}

void swEnable(void)
{
	SWITCH_1(0);
	SWITCH_2(0);
	SWITCH_3(0);
}


void swDisable(void)
{
	SWITCH_1(1);
	SWITCH_2(1);
	SWITCH_3(1);
}



static void keyMsgProcess(const KEY_MSG * pcMsg)
{
	if (!(pcMsg->ubKeyValue & ALARM_KEY_PIN))
	{
		XPRINTF((12, "ALARM_KEY\n"));
		alarmKeyProcess( );
		swEnable( );
		
	}
	
	if (!(pcMsg->ubKeyValue & SILENCER_KEY_PIN))
	{
		XPRINTF((12, "SILENCER_KEY_PIN\n"));
		silenceKeyProcess( );
		swDisable( );
	}
	
	if (!(pcMsg->ubKeyValue & SELF_TEST_KEY_PIN))
	{
		XPRINTF((12, "SELF_TEST_KEY_PIN\n"));
		process_post(&dev_check_self_process, event_self_check, (void *)pcMsg);
	}
}


void keyMsgHandler(process_event_t ev, process_data_t data)
{
	static KEY_MSG keyMsg;

	if (ev == event_key_msg && data != NULL)
	{
		keyMsg = *(KEY_MSG *)data;
		keyMsgProcess((const KEY_MSG*)&keyMsg);
	}
}


static void buzzOpt(u_char ubstate)
{
	BUZZER(ubstate);
}

static void ledOpt(u_char ubstate)
{
	ALARM_LED(ubstate);
	FAULT_LED(ubstate);
	NET_LED(ubstate);
	POWER_LED(ubstate);
}



typedef struct _check_msg
{
	struct etimer et_timer;
	u_char ubstate;
}CHECK_MSG;

#define CHECK_BUZZ_TIME		1000
#define CHECK_LED_FIRST_TIME	5000
#define CHECK_LED_TIME			1000



void selfCheckHandler(process_event_t ev, process_data_t data)
{
	static struct etimer et_buzz;
	static u_char ubbzstate = 0;
	
	static struct etimer et_led;
	static u_char ubledstate = 0;
	
	static u_char checkfinish = 0;
	static u_char count = 0;


	if (ev == event_self_check && data != NULL)
	{
		checkfinish = 0;
		count = 0;

		XPRINTF((6, "selfcheck\n"));
		//buzz operation
		ubbzstate = 0;
		etimer_set(&et_buzz, CHECK_BUZZ_TIME);
		buzzOpt(ubbzstate);

		//led operation
		ubledstate = 0;
		etimer_set(&et_led, CHECK_LED_FIRST_TIME);
		ledOpt(ubledstate);
		process_exit(&blink_process);
		
	}

	else if (ev == PROCESS_EVENT_TIMER && data == &et_buzz)
	{
		if (!checkfinish)
		{
			if (ubbzstate == 0)
				ubbzstate = 1;
			else
				ubbzstate = 0;
			buzzOpt(ubbzstate);
			etimer_set(&et_buzz, CHECK_BUZZ_TIME);
		}
	}
	else if (ev == PROCESS_EVENT_TIMER && data == &et_led)	
	{
		count ++;
		if (ubledstate == 0)
			ubledstate = 1;
		else
			ubledstate = 0;
		ledOpt(ubledstate);

		if (count >= 8)
		{
			checkfinish = 1;
			BUZZER(1);
			ledOpt(1);
			NET_LED(0);
			POWER_LED(0);
			process_start(&blink_process, NULL);
		}
		else
		{
			etimer_set(&et_led, CHECK_LED_TIME);
		}
	}
}



PROCESS_THREAD(dev_check_self_process, ev, data)
{
	
	PROCESS_BEGIN();
	while (1)
	{
		PROCESS_YIELD( );
		selfCheckHandler(ev, data);
	}
	PROCESS_END();
}



PROCESS_THREAD(key_msg_handler_process, ev, data)
{
	PROCESS_BEGIN();
	while (1)
	{
		PROCESS_YIELD();
		keyMsgHandler(ev, data);
	}
	PROCESS_END();
}





PROCESS_THREAD(key_msg_read_process, ev, data)
{
	static struct etimer etkey;
	static KEY_MSG keyMsgRead;
	static KEY_MSG keyMsg;
	static u_short ubKey = 0;
	static u_char ubKeyCount = 0;
	PROCESS_BEGIN();
	
	event_key_msg = process_alloc_event( );
	event_self_check = process_alloc_event( );
	
	memset(&keyMsg, 0, sizeof(KEY_MSG));
	memset(&keyMsgRead, 0, sizeof(KEY_MSG));

	while (1)
	{
		etimer_set(&etkey, 50);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etkey));
		ubKey = keyGetValue();

		if (ubKey != KEY_VALUE)//have key press
		{
			ubKeyCount++;
			keyMsgRead.ubKeyValue = ubKey;
			keyMsgRead.ubCountTime++;
			XPRINTF((11, "KEY = %04x\n", ubKey));
			ubKey = 0;
		}
		else 
		{
			if (ubKeyCount != 0)
			{
				ubKeyCount = 0;
				if (keyMsgRead.ubCountTime >= 2)
				{
					keyMsg = keyMsgRead;
					memset(&keyMsgRead, 0, sizeof(KEY_MSG));
					XPRINTF((6, "KEY = %04x\n", keyMsg.ubKeyValue));
					process_post(&key_msg_handler_process, event_key_msg, &keyMsg);
				}
				else
				{
					memset(&keyMsgRead, 0, sizeof(KEY_MSG));
				}
			}
		}
	}
	PROCESS_END();
}



void initAppKey(void)
{
	process_start(&key_msg_read_process, NULL);
	process_start(&key_msg_handler_process, NULL);
	process_start(&dev_check_self_process, NULL);
	swDisable();
}




