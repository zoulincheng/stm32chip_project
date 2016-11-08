/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "dev/watchdog.h"
#include <stdlib.h>


#include "stm32f10x.h"
#include "stm32f10x_iwdg.h"

//Init IWDG 
//

static void IWDG_Configuration(void)
{
	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* 狗狗时钟分频,40K/256=156HZ(6.4ms)*/
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* 喂狗时间 5s/6.4MS=781 .注意不能大于0xfff*/
	//IWDG_SetReload(781);
	//IWDG_SetReload(1563); // 10 seconds
	//IWDG_SetReload(3200);   // 20+ seconds
	IWDG_SetReload(4000);   // 20+ seconds

	/* 喂狗*/
	IWDG_ReloadCounter( );

	/* 使能狗狗*/
	IWDG_Enable( );
	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}



static void IWDG_Watchdog_Feed( void )
{
	#if 0
	IWDG->KR = IWDG_WriteAccess_Enable;
	IWDG->KR = 0xaaaa;
	IWDG->KR = IWDG_WriteAccess_Disable;
	//IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	//IWDG_ReloadCounter();
	//IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
	#else
	IWDG_ReloadCounter( );
	#endif
}



/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
	IWDG_Configuration( );
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
	IWDG_Watchdog_Feed( );
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
	// Death by watchdog.
//	exit(-1);
}
/*---------------------------------------------------------------------------*/
