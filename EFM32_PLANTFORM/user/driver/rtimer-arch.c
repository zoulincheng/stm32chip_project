#include "em32lg_config.h"
#include "contiki.h"
#include "rtimer-arch.h"


#include "sysprintf.h"

static uint32_t saved_TIMCFG;
static uint32_t time_msb = 0;   /* Most significant bits of the current time. */

/* time of the next rtimer event. Initially is set to the max
    value. */
static rtimer_clock_t next_rtimer_time = 0;


/*
* LETIMER0_IRQHandler service funtion
*/

void LETIMER0_IRQHandler(void)
{
	uint32_t letimer_intflags;
	LETIMER_TypeDef *letimer = LETIMER0;
	letimer_intflags = letimer->IF;
	//under flow
	if ((letimer_intflags & LETIMER_IF_UF) == LETIMER_IF_UF)
	{
		rtimer_clock_t now, clock_to_wait;
		//LETIMER_IntClear(LETIMER_TypeDef * letimer, uint32_t flags)
		//clear interrupt flag
		letimer->IFC |= LETIMER_IFC_UF;

		//rtimer_clock_t clock_sys;
		time_msb++;
		now = ((rtimer_clock_t) time_msb << 16) |(0xffff-(uint16_t)letimer->CNT);
		clock_to_wait = next_rtimer_time - now;		

		//XPRINTF((0, "rtimer\r\n"));
		//XPRINTF((0, "clock_time is %d\r\n", clock_time( )));
		if(clock_to_wait <= 0x10000 && clock_to_wait > 0) 
		{ 
			/* We must now set the Timer Compare Register. */
			//TIM2->CCR1 = (uint16_t) clock_to_wait;
			//TIM2->SR = INT_TIMCC1IF;
			//TIM2->DIER |= INT_TIMCC1IF;
			//letimer->COMP1 = (uint16_t)(0x0ffff-(uint16_t)(now + clock_to_wait));
			letimer->COMP1 = ((uint16_t)clock_to_wait) <= letimer->CNT ? ((uint16_t)letimer->CNT- (uint16_t)clock_to_wait) : (uint16_t)(0xffff-(uint16_t)clock_to_wait + (uint16_t)letimer->CNT);
			letimer->IFS |= LETIMER_IFS_COMP1;
			letimer->IEN |= LETIMER_IEN_COMP1;
		}
	}
	else 
	{
		//comp1 match interrupt process
		if((letimer_intflags & LETIMER_IF_COMP1) == LETIMER_IF_COMP1) 
		{
			//TIM2->DIER &= ~INT_TIMCC1IF;       /* Disable the next compare interrupt */
			letimer->IFS &= ~LETIMER_IFS_COMP1;
			ENERGEST_ON(ENERGEST_TYPE_IRQ);
			rtimer_run_next( );
			ENERGEST_OFF(ENERGEST_TYPE_IRQ);
			
			//TIM2->SR = INT_TIMCC1IF;
			letimer->IFC |= LETIMER_IFC_COMP1; //clear flag
			next_rtimer_time = 0;
			//XPRINTF((0, " comp1 rtimer\r\n"));
		}
	}
}


/*---------------------------------------------------------------------------*/
void rtimer_arch_disable_irq(void)
{
	LETIMER_TypeDef *letimer = LETIMER0;
	INT_Disable( );
	saved_TIMCFG = letimer->IEN&0x1f;
	letimer->IEN = 0;
	INT_Enable( );
}

/*---------------------------------------------------------------------------*/
void rtimer_arch_enable_irq(void)
{
	LETIMER_TypeDef *letimer = LETIMER0; 
	letimer->IEN |= saved_TIMCFG;
}


/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void)
{
	LETIMER_TypeDef *letimer = LETIMER0; 
	rtimer_clock_t t;
	INT_Disable( );
	t = ((rtimer_clock_t) time_msb << 16) | (0XFFFF-(uint16_t)letimer->CNT);
	INT_Enable( );
	return t;
}


void
rtimer_arch_set(rtimer_clock_t t)
{
	
}



/*---------------------------------------------------------------------------*/
void rtimer_arch_schedule(rtimer_clock_t t)
{
	LETIMER_TypeDef *letimer = LETIMER0; 
	rtimer_clock_t now, clock_to_wait;
	
	INT_Disable( );
	now = rtimer_arch_now();
	next_rtimer_time = t;
	clock_to_wait = t - now;

	
	if(clock_to_wait <= 0x10000 && clock_to_wait > 0) 
	{
		/* We must now set the Timer Compare Register. */
		//LETIMER0->COMP1 = (uint16_t)(0x0ffff - (uint16_t)(now + clock_to_wait));
		letimer->COMP1 = ((uint16_t)clock_to_wait) <= letimer->CNT ? ((uint16_t)letimer->CNT - (uint16_t)clock_to_wait):(uint16_t)(0xffff-(uint16_t)clock_to_wait + (uint16_t)letimer->CNT);
		letimer->IFS |= LETIMER_IFS_COMP1;
		letimer->IEN |= LETIMER_IEN_COMP1;
		
		//next_rtimer_time = 0;
	}
	INT_Enable( );
}

/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_next_trigger()
{
  return next_rtimer_time;
}



void rtimer_arch_init(void)
{

	LETIMER_TypeDef *letimer = LETIMER0;
	uint32_t nirq_Priority;
	/* Enable necessary clocks */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	/* The CORELE clock is also necessary for the RTC and all
	low energy peripherals, but since this function
	is called before RTC_setup() the clock enable
	is only included here */

	CMU_ClockEnable(cmuClock_LETIMER0, true);  

	CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_2);//32.768khz/2  61us/tick
	/* Set initial compare values for COMP0 */
	LETIMER_CompareSet(letimer, 0, 0x0000ffff);

	/* Set configurations for LETIMER 0 */
	const LETIMER_Init_TypeDef letimerInit = 
	{
	.enable         = false,                  /* Don't start counting when init completed  */
	.debugRun       = false,                  /* Counter shall not keep running during debug halt. */
	.rtcComp0Enable = false,                  /* Don't start counting on RTC COMP0 match. */
	.rtcComp1Enable = false,                  /* Don't start counting on RTC COMP1 match. */
	.comp0Top       = true,                   /* Load COMP0 register into CNT when counter underflows. COMP is used as TOP */
	.bufTop         = false,                  /* Don't load COMP1 into COMP0 when REP0 reaches 0. */
	.out0Pol        = 0,                      /* Idle value for output 0. */
	.out1Pol        = 0,                      /* Idle value for output 1. */
	.ufoa0          = letimerUFOANone,        /* No output on output 0 */
	.ufoa1          = letimerUFOANone,        /* No output on output 1*/
	.repMode        = letimerRepeatFree       /* RepeateFree model */
	};

	/* Initialize LETIMER */
	LETIMER_Init(letimer, &letimerInit); 	

	/* Clear previous  interrupts */
	//LETIMER_IntClear(letimer, LETIMER_IFC_UF);
	letimer->IFC = _LETIMER_IFC_MASK;
	NVIC_ClearPendingIRQ(LETIMER0_IRQn);

	LETIMER_IntSet(letimer, LETIMER_IFS_UF);
	LETIMER_IntEnable(letimer, LETIMER_IEN_UF);
	//set Priority
	nirq_Priority = NVIC_EncodePriority(INT_LETIMER0_nIRQ_GROUP, INT_LETIMER0_nIRQ_PREP, INT_LETIMER0_nIRQ_SUBP);
	NVIC_SetPriority(LETIMER0_IRQn, nirq_Priority);
	
	NVIC_EnableIRQ(LETIMER0_IRQn);
	LETIMER_Enable(letimer, true);
}


