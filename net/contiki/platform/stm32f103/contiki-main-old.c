#include "stm32f10x.h"

#include <stdint.h>
#include <stdio.h>

#include "debug-uart.h"
#include "sysprintf.h"

#include "contiki.h"

#include <clock.h>

unsigned int idle_count = 0;

//PROCINIT(&etimer_process);
PROCINIT(NULL);


PROCESS(blink_process, "blink_led");
PROCESS_THREAD(blink_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	XPRINTF((0, "\r\n"));
	etimer_set(&et, 100);
	while(1) 
	{
		XPRINTF((0, "blink_led___1\r\n"));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		XPRINTF((0, "blink_led___2\r\n"));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
	}
	PROCESS_END();
}



int main(void)
{
	//System debug uart init
	dbg_setup_uart( );
	 xdev_out(dbg_send_char);//set std output

	clock_init( );
	process_start(&etimer_process, NULL);

	process_start(&blink_process, NULL);

	while (1)
	{
		process_run( );
		etimer_request_poll( );
		//PRINTF("hello\r\n");
	}
//	return 0;
}




