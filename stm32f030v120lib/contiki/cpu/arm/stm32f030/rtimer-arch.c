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

#include "stm32f0xx.h"

//#include "xprintf.h"
#include "rtimer-arch.h"
//#include "arch_s.h"
//#include "atom.h"
#include "sysprintf.h"
//#include "iodef.h"



#if 0
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#endif

static uint16_t saved_TIMCFG;
static uint32_t time_msb = 0;   /* Most significant bits of the current time. */

/* time of the next rtimer event. Initially is set to the max
    value. */
static rtimer_clock_t next_rtimer_time = 0;





/*---------------------------------------------------------------------------*/

void
rtimer_arch_init(void)
{
}

/*---------------------------------------------------------------------------*/
void rtimer_arch_disable_irq(void)
{
}


/*---------------------------------------------------------------------------*/
void rtimer_arch_enable_irq(void)
{

}


/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void)
{
	return 0;
}


void
rtimer_arch_set(rtimer_clock_t t)
{
	
}



/*---------------------------------------------------------------------------*/
void rtimer_arch_schedule(rtimer_clock_t t)
{

}
/*---------------------------------------------------------------------------*/








/** @} */
