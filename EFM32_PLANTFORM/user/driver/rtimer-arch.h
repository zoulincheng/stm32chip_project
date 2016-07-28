/**
 * \file
 *         Header file for the STM32F103-specific rtimer code
 * \author
 *         Simon Berg <ksb@users.sourceforge.net>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "sys/rtimer.h"


#define	RTIMER_ARCH_SECOND	(16384)  //One tick: 61us  32768/2
void rtimer_arch_set(rtimer_clock_t t);
void rtimer_arch_init(void);

rtimer_clock_t rtimer_arch_now(void);

void rtimer_arch_disable_irq(void);

void rtimer_arch_enable_irq(void);

#endif /* __RTIMER_ARCH_H__ */
