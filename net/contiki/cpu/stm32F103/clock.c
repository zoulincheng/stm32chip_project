/*
#include <stm32f10x_map.h>
#include <nvic.h>
*/
//替换头文件
//#include "sys/process.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <debug-uart.h>

#include "sysprintf.h"
#include "basictype.h"
//#include "radio/si4432_rf.h"
//#include "radio/si4432.h"
#include "si446x.h"

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

clock_time_t g_sysTime = 0;


#if 0
static __INLINE uint32_t SysTick_Config(uint32_t ticks)
{ 
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1);            /* Reload value impossible */
                                                               
  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Cortex-M0 System Interrupts */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | 
                   SysTick_CTRL_TICKINT_Msk   | 
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}
#endif

void SysTick_Handler(void)
{
	//(void)SysTick->CTRL;
	//SCB->ICSR = SCB_ICSR_PENDSTCLR;
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
}

#if 0
void
clock_init()
{
  /*
  NVIC_SET_SYSTICK_PRI(8);
  SysTick->LOAD = MCK/8/CLOCK_SECOND;
  SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT;
  */
  #if 0
  current_clock = 0;
  //Systick时钟每s触发一次CLOCK_SECOND次
  if (SysTick_Config(SystemCoreClock / CLOCK_SECOND))
  { 
    //
   	while (1);
  }
  #endif
  /* SysTick initialization using the system clock.*/
  SysTick->LOAD = SystemCoreClock / CLOCK_SECOND - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_ENABLE_Msk |
                  SysTick_CTRL_TICKINT_Msk;}
#else

void
clock_init()
{
	current_clock = 0;
	SysTick_Config(SystemCoreClock / CLOCK_SECOND);   
}
#endif



clock_time_t
clock_time(void)
{
	return current_clock;
}

#if 0
/* The inner loop takes 4 cycles. The outer 5+SPIN_COUNT*4. */

#define SPIN_TIME 2 /* us */
#define SPIN_COUNT (((MCK*SPIN_TIME/1000000)-5)/4)

#ifndef __MAKING_DEPS__

void
clock_delay(unsigned int t)
{
#ifdef __THUMBEL__ 
  asm volatile("1: mov r1,%2\n2:\tsub r1,#1\n\tbne 2b\n\tsub %0,#1\n\tbne 1b\n":"=l"(t):"0"(t),"l"(SPIN_COUNT));
#else
#error Must be compiled in thumb mode
#endif
}
#endif
#endif /* __MAKING_DEPS__ */

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
	//XPRINTF((0, "start t %d \r\n", start));
	while(clock_time( ) - start < (clock_time_t) i)
	{
		//XPRINTF((0, "sys t %d \r\n", clock_time( )));
	}
}


unsigned long
clock_seconds(void)
{
	return current_seconds;
}



