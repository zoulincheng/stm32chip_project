/**
 * \file
 *         Header file for the STM32F103-specific rtimer code
 * \author
 *         Simon Berg <ksb@users.sourceforge.net>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include "sys/rtimer.h"

//#define RTIMER_ARCH_SECOND (MCK/1024)

//TIM COUNT CLK IS (TIM2 CLK /PSC)18000000/1800= 10kHz  clk  RTIMER_ARCH_SECOND   10000
//#define	RTIMER_ARCH_SECOND	(2000)
#define	RTIMER_ARCH_SECOND	(10000)  //One tick: 1/10000s = 100us


void rtimer_arch_set(rtimer_clock_t t);
void rtimer_arch_init(void);

rtimer_clock_t rtimer_arch_now(void);

void rtimer_arch_disable_irq(void);

void rtimer_arch_enable_irq(void);

#endif /* __RTIMER_ARCH_H__ */
