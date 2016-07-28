#include "em32lg_config.h"
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include "basictype.h"


/*---------------------------------------------------------------------------*/
#define RTIMER_CLOCK_TICK_RATIO (RTIMER_SECOND / CLOCK_SECOND)


static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;



void SysTick_Handler(void)
{
	INT_Disable( );
	
	current_clock++;
	if(etimer_pending() && etimer_next_expiration_time() <= current_clock) 
	{
		etimer_request_poll();
	}
	if (--second_countdown == 0) 
	{
		current_seconds++;
		second_countdown = CLOCK_SECOND;
	}
	
	INT_Enable( );
}


void clock_init( )
{
	u_long nirq_Priority = 0;
	current_clock = 0;
	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;
	nirq_Priority = NVIC_EncodePriority(INT_SYSTICK_nIRQ_GROUP, INT_SYSTICK_nIRQ_PREP, INT_SYSTICK_nIRQ_SUBP);
	NVIC_SetPriority(SysTick_IRQn, nirq_Priority);
}


clock_time_t clock_time(void)
{
	return current_clock;
}


void clock_delay(unsigned int t)
{
	unsigned int i;
	for(i = 0; i < t; i++)
	{
		
	}
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 ms.
 */
void
clock_wait(clock_time_t i)
{
	clock_time_t start;

	start = clock_time();
	while(clock_time( ) - start < (clock_time_t) i)
	{
	}
}


unsigned long
clock_seconds(void)
{
	return current_seconds;
}

