/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */

/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
* \file
*			Real-timer specific implementation for STM32W.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "contiki-conf.h"

#include "stm32f10x.h"
//#include "xprintf.h"
#include "rtimer-arch.h"
#include "arch_s.h"
#include "atom.h"
#include "sysprintf.h"
#include "iodef.h"



static uint16_t saved_TIMCFG;
static uint32_t time_msb = 0;   /* Most significant bits of the current time. */

/* time of the next rtimer event. Initially is set to the max
    value. */
static rtimer_clock_t next_rtimer_time = 0;



#define TIM_UG                                       (0x00000001u)
#define INT_TIMUIF                                   (0x00000001u)
#define TIM_CEN                                      (0x00000001u)
#define INT_TIMCC1IF                                 (0x00000002u)
#define TIM_CURRENT									TIM2

//unsigned char static ledstate = 0;
  
/*---------------------------------------------------------------------------*/
void TIM2_IRQHandler(void)
{
	if(TIM2->SR &INT_TIMUIF)
	{
		rtimer_clock_t now, clock_to_wait;

		//rtimer_clock_t clock_sys;
		/* Overflow event. */
		/* PRINTF("O %4x.\r\n", TIM1_CNT); */
		/* printf("OV "); */
		time_msb++;
		now = ((rtimer_clock_t) time_msb << 16) | TIM2->CNT;
		clock_to_wait = next_rtimer_time - now;

		//clock_sys = clock_time( );

		if(clock_to_wait <= 0x10000 && clock_to_wait > 0) 
		{ 
			/* We must now set the Timer Compare Register. */
			#if 0
			TIM1_CCR1 = (uint16_t) clock_to_wait;
			INT_TIM1FLAG = INT_TIMCC1IF;
			INT_TIM1CFG |= INT_TIMCC1IF;      /* Compare 1 interrupt enable. */
			#endif
			TIM2->CCR1 = (uint16_t) clock_to_wait;
			TIM2->SR = INT_TIMCC1IF;
			TIM2->DIER |= INT_TIMCC1IF;
			//XPRINTF((0,"to wait\r\n"));
		}
		TIM2->SR = INT_TIMUIF;
		TIM2->SR &= ~(1<<0);
		//XPRINTF((0,"clock is %d\r\n", clock_time( )));
	}
	else 
	{
		if(TIM2->SR & INT_TIMCC1IF) 
		{
			TIM2->DIER &= ~INT_TIMCC1IF;       /* Disable the next compare interrupt */
			ENERGEST_ON(ENERGEST_TYPE_IRQ);
			rtimer_run_next();
			ENERGEST_OFF(ENERGEST_TYPE_IRQ);
			
			//INT_TIM1FLAG = INT_TIMCC1IF;
			TIM2->SR = INT_TIMCC1IF;
			//TIM2->DIER = INT_TIMCC1IF;
			//XPRINTF((0,"schedule\r\n"));
		}
	}
	//TIM2->SR &= ~(1<<0);
}

/*---------------------------------------------------------------------------*/

void
rtimer_arch_init(void)
{
#if 0
	TIM1_CR1 = 0;
	TIM1_PSC = RTIMER_ARCH_PRESCALER;

	/* Counting from 0 to the maximum value. */
	TIM1_ARR = 0xffff;


	/* Bits of TIMx_CCMR1 as default. */
	/* Update Generation. */
	TIM1_EGR = TIM_UG;
	INT_TIM1FLAG = 0xffff;

	/* Update interrupt enable (interrupt on overflow).*/
	INT_TIM1CFG = INT_TIMUIF;

	/* Counter enable. */
	TIM1_CR1 = TIM_CEN;

	/* Enable top level interrupt. */
	INT_CFGSET = INT_TIM1;
#endif
#if 1

	RCC->APB1ENR |= 1<<0; //TIM2 CLK enable  72MHz
	//RCC_PCLK1Config(RCC_HCLK_Div8);//TIM2 CLK (72/8)*2=18MHz

	TIM2->CR1 = 0;

	TIM2->PSC = 7199;//TIM COUNT CLK IS (TIM2 CLK /PSC)18000000/1800= 10kHz  clk  RTIMER_ARCH_SECOND   10000
	/* Counting from 0 to the maximum value. */
	//TIM2->ARR = 0xffff;
	TIM2->ARR = 0xffff;

	/* Bits of TIMx_CCMR1 as default. */
	/* Update Generation. */
	TIM2->EGR = TIM_UG;
	//TIM2->SR  = 0xffff;

	/* Update interrupt enable (interrupt on overflow).*/
	TIM2->DIER = INT_TIMUIF;

	/* Counter enable. */
	TIM2->CR1 = TIM_CEN;

	{
		//TIM interrupt NVIC CONFIG
		NVIC_InitTypeDef strNVIC;
		/* Enable the TIM2 global Interrupt */
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
		strNVIC.NVIC_IRQChannel = TIM2_IRQn;
		strNVIC.NVIC_IRQChannelPreemptionPriority = 1;
		strNVIC.NVIC_IRQChannelSubPriority = 3;
		strNVIC.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&strNVIC);	
	}
#endif
#if 0
	//TIM2,3,4 clk is 72MHz after system reset
	//PCLK1 is 36MHz after system reset
	TIM_TypeDef strTIM;
	NVIC_InitTypeDef strNVIC;
	TIM_TimeBaseInitTypeDef strTIMTB;

	//RCC config for TIM
	/* PCLK1 = HCLK/4 */
	//RCC_PCLK1Config(RCC_HCLK_Div4);
	/* TIM2 clock enable */
	//TIM2 clk is 72MHz
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Enable the TIM2 global Interrupt */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
	strNVIC.NVIC_IRQChannel = TIM2_IRQn;
	strNVIC.NVIC_IRQChannelPreemptionPriority = 1;
	strNVIC.NVIC_IRQChannelSubPriority = 3;
	strNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&strNVIC);

	/* Time base configuration */
	TIM_DeInit(TIM2);
	//strTIMTB.TIM_Period = 65535; 			//设置计数溢出大小，每计满数就产生一个更新事件
	strTIMTB.TIM_Period = 5000; 			//设置计数溢出大小，每计满数就产生一个更新事件
	strTIMTB.TIM_Prescaler = (7200-1);		//预分频系数为，这样计数器时钟为72MHz/7200 = 10kHz       
	strTIMTB.TIM_ClockDivision = TIM_CKD_DIV1;
	strTIMTB.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &strTIMTB);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);	 //清除溢出中断标志 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //开启TIM2的中断
#endif
}

/*---------------------------------------------------------------------------*/
void rtimer_arch_disable_irq(void)
{
	//ATOMIC(saved_TIMCFG = TIM2->DIER; TIM2->DIER = 0;)	
	//saved_TIMCFG = TIM2->DIER; TIM2->DIER = 0;
	//OSCRITICAL cr;
	//OSInitCritical(&cr);
	//OSEnterCritical(&cr);
	saved_TIMCFG = TIM2->DIER;
	TIM2->DIER = 0;
	//OSExitCritical(&cr);
}




/*---------------------------------------------------------------------------*/
void rtimer_arch_enable_irq(void)
{
	TIM2->DIER = saved_TIMCFG;
}


/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void)
{
#if 0
  	rtimer_clock_t t;

 	 //ATOMIC(t = ((rtimer_clock_t) time_msb << 16) | TIM1_CNT;)
	t = ((rtimer_clock_t) time_msb << 16) | TIM1->CNT;
	
	return t;
  
#endif
#if 1
	rtimer_clock_t t;
	//OSCRITICAL cr;
	//PRINTF("rtimer now\r\n");
	//ATOMIC(t = ((rtimer_clock_t) time_msb << 16) | TIM2->CNT;)
	//OSInitCritical(&cr);
	//OSEnterCritical(&cr);
	t = ((rtimer_clock_t) time_msb << 16) | TIM2->CNT;
	//OSExitCritical(&cr);
	//PRINTF("rtimer now\r\n");
	return t;
#endif
}


void
rtimer_arch_set(rtimer_clock_t t)
{
//offset = t -  RTIMER_ARCH_TIMER_BASE->TC_CV;
	
}



/*---------------------------------------------------------------------------*/
void rtimer_arch_schedule(rtimer_clock_t t)
{
#if 0
	rtimer_clock_t now, clock_to_wait;
	XPRINTF((0,"rtimer_arch_schedule time %4x\r\n", /*((uint32_t*)&t)+1, */ (uint32_t)t));
	next_rtimer_time = t;
	now = rtimer_arch_now();
	clock_to_wait = t - now;

	XPRINTF((0,"now %2x\r\n", TIM1_CNT));
	XPRINTF((0,"clock_to_wait %4x\r\n", clock_to_wait));

	if(clock_to_wait <= 0x10000) {
	/* We must now set the Timer Compare Register. */
	TIM1_CCR1 = (uint16_t)now + (uint16_t)clock_to_wait;
	INT_TIM1FLAG = INT_TIMCC1IF;
	INT_TIM1CFG |= INT_TIMCC1IF;        /* Compare 1 interrupt enable. */
	XPRINTF((0,"2-INT_TIM1FLAG %2x\r\n", INT_TIM1FLAG));
	}
	/* else compare register will be set at overflow interrupt closer to
	the rtimer event. */
#endif
	rtimer_clock_t now, clock_to_wait;
	//XPRINTF((0,"rtimer_arch_schedule time %4x\r\n", /*((uint32_t*)&t)+1, */ (uint32_t)t));
	next_rtimer_time = t;
	now = rtimer_arch_now();
	clock_to_wait = t - now;

	//XPRINTF((0,"now %2x\r\n", TIM2->CNT));
	//XPRINTF((0,"clock_to_wait %4x\r\n", clock_to_wait));

	if(clock_to_wait <= 0x10000) 
	{
		/* We must now set the Timer Compare Register. */
		TIM2->CCR1= (uint16_t)now + (uint16_t)clock_to_wait;
		TIM2->SR = INT_TIMCC1IF;
		TIM2->DIER |= INT_TIMCC1IF;        /* Compare 1 interrupt enable. */
		//XPRINTF((0,"2-INT_TIM1FLAG %2x\r\n", TIM2->SR));
	}
	/* else compare register will be set at overflow interrupt closer to
	the rtimer event. */

}
/*---------------------------------------------------------------------------*/





void TIM2_Init(u16 uwARR, u16 uwPSC)
{
	#if 1
	RCC->APB1ENR |= 1<<0; //TIM2 CLK enable  72MHz
	TIM2->ARR = uwARR; //auto number 
	TIM2->PSC = uwPSC; //

	TIM2->DIER |= 1 << 0; //Allow update interrupt
	//TIM2->DIER |= 1 << 6; //Allow trige interrupt

	TIM2->CR1 |= 0x01; //enable TIM2

	{
		NVIC_InitTypeDef strNVIC;
		/* Enable the TIM2 global Interrupt */
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
		strNVIC.NVIC_IRQChannel = TIM2_IRQn;
		strNVIC.NVIC_IRQChannelPreemptionPriority = 1;
		strNVIC.NVIC_IRQChannelSubPriority = 3;
		strNVIC.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&strNVIC);	
	}
#endif
}







/** @} */
