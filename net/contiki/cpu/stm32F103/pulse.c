#include "contiki.h"
#include <string.h>
#include "stm32f10x.h"
#include "basictype.h"
#include "stm32f10x_it.h"
#include "iodef.h"
#include "sysprintf.h"

#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"

#include "pulse.h"

static const u_char ubMeterA[6]={0x91,0x99,0x99,0x99,0x99,0x99};
static const u_char ubMeterB[6]={0x92,0x99,0x99,0x99,0x99,0x99};

void pulse_pin_init(void);

void pulse_bkp_init(void)
{
  /* Enable PWR and BKP clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Enable write access to Backup domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Clear Tamper pin Event(TE) pending flag */
  BKP_ClearFlag( );

  //init pulse input pin
  pulse_pin_init( );
}


void pulse_pin_init(void)
{
	GPIO_InitTypeDef gpioStr;
	EXTI_InitTypeDef extiStr;
	NVIC_InitTypeDef nvicStr;

	//clock Enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	//nvic config for  exti interrupt	
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	//pulse 1  2
	nvicStr.NVIC_IRQChannel = EXTI0_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStr);
	//pulse 2
	nvicStr.NVIC_IRQChannel = EXTI1_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStr);


	//EXTI line mode config
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	extiStr.EXTI_Line = EXTI_Line0;
	extiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	extiStr.EXTI_Trigger = EXTI_Trigger_Falling;
	extiStr.EXTI_LineCmd = ENABLE;
	EXTI_Init( &extiStr);

	//EXTI line mode config
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
	extiStr.EXTI_Line = EXTI_Line1;
	extiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	extiStr.EXTI_Trigger = EXTI_Trigger_Falling;
	extiStr.EXTI_LineCmd = ENABLE;
	EXTI_Init( &extiStr);
}	

int pulse_get_counter(const u_char *pcMeterAddr)
{
	u_long nResult     = 0;
	u_long uwMeterLow  = 0;
	u_long uwMeterHigh = 0;

	if (0 == mem_cmp(ubMeterA, pcMeterAddr,6))
	{
		uwMeterLow  = BKP_ReadBackupRegister(BKP_PULS1_0);
		uwMeterHigh = BKP_ReadBackupRegister(BKP_PULS1_1);		
	}
	else if (0 == mem_cmp(ubMeterB, pcMeterAddr,6))
	{
		uwMeterLow  = BKP_ReadBackupRegister(BKP_PULS2_0);
		uwMeterHigh = BKP_ReadBackupRegister(BKP_PULS2_1);		
	}

	nResult = uwMeterLow | (uwMeterHigh<<16);

	return nResult;
}


void pulse_connter_clear(void)
{
	BKP_WriteBackupRegister(BKP_PULS1_0, 0);
	BKP_WriteBackupRegister(BKP_PULS1_1, 0);
	BKP_WriteBackupRegister(BKP_PULS2_0, 0);
	BKP_WriteBackupRegister(BKP_PULS2_1, 0);
}


//pulse 1  ->ubMeterA
void EXTI0_IRQHandler(void)
{	
	u_long nResult = 0;
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0);	
		nResult = pulse_get_counter(ubMeterA);
		nResult++;
		BKP_WriteBackupRegister(BKP_PULS1_0, (u_short)nResult);
		BKP_WriteBackupRegister(BKP_PULS1_1, (u_short)(nResult >> 16));
		//XPRINTF((0, "P = %d\r\n", nResult));
	}
}

//pulse 2  -> ubMeterB
void EXTI1_IRQHandler(void)
{	
	u_long nResult = 0;
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line1);	
		nResult = pulse_get_counter(ubMeterB);
		nResult++;
		BKP_WriteBackupRegister(BKP_PULS2_0, (u_short)nResult);
		BKP_WriteBackupRegister(BKP_PULS2_1, (u_short)(nResult >> 16));		
	}
}



#if 0
//For test BKP REG write and read.
PROCESS(bkp_test_process, "bkp_test");

PROCESS_THREAD(bkp_test_process, ev, data)
{
	//static u_char ubach[4] = {0x12, 0x23, 0x55, 0x66};
	static struct etimer et;
	static u_char i = 0;
	//u_short ubData;
	PROCESS_BEGIN();
	XPRINTF((10, "bkp_test\r\n"));
	pulse_bkp_init( );
	
	while(1) 
	{
		etimer_set(&et, 1000);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		BKP_WriteBackupRegister(BKP_PULS1_0, (0x1234 + i));
		BKP_WriteBackupRegister(BKP_PULS1_1, (0x2233 + i));
		BKP_WriteBackupRegister(BKP_PULS2_0, (0xa5a5 + i));
		BKP_WriteBackupRegister(BKP_PULS2_1, (0x5a5a + i));			

		etimer_set(&et, 1000);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		XPRINTF((6, "BKP_DR2 value is %02x\r\n", BKP_ReadBackupRegister(BKP_PULS1_0)));
		XPRINTF((6, "BKP_DR3 value is %02x\r\n", BKP_ReadBackupRegister(BKP_PULS1_1)));

		XPRINTF((6, "BKP_DR4 value is %02x\r\n", BKP_ReadBackupRegister(BKP_PULS2_0)));
		XPRINTF((6, "BKP_DR5 value is %02x\r\n", BKP_ReadBackupRegister(BKP_PULS2_1)));
		i++;
		
	}
	PROCESS_END();
}


#endif






