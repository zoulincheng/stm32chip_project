#include "contiki-conf.h"
#include "stm32f10x.h"
#include "iodef.h"
#include "basictype.h"
#include "atom.h"

#include "sysprintf.h"


void OSInitSys(void)
{
	GPIO_InitTypeDef gpioInitSt;
	NVIC_InitTypeDef nvicInitSt;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//然后调用GPIO重映射函数，根据需求实现重映射：
	

	//Periph clock enable 
	RCC_APB1PeriphClockCmd(
		  RCC_APB1Periph_USART2
		|RCC_APB1Periph_USART3
//		|RCC_APB1Periph_WWDG
		|RCC_APB1Periph_UART4
		|RCC_APB1Periph_UART5
//		|RCC_APB1Periph_BKP
		,ENABLE);

	//------------------------------------------------------------
	//RCC_APB2Periph_AFIO
	RCC_APB2PeriphClockCmd(
					  RCC_APB2Periph_GPIOA
					|RCC_APB2Periph_GPIOB
					|RCC_APB2Periph_GPIOC
					|RCC_APB2Periph_GPIOD
					//|RCC_APB2Periph_GPIOE
					|RCC_APB2Periph_AFIO
					|RCC_APB2Periph_USART1
					|RCC_APB2ENR_AFIOEN
					, ENABLE);

	//NVIC interrupt congig
	/*Common interrupt when use which interrupt,
	  come here config the interrupt source.
	*/
	//UART1 RX interrupt config
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	// Enable the USART1 gloabal Interrupt 
	nvicInitSt.NVIC_IRQChannel = USART1_IRQn;
	nvicInitSt.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitSt.NVIC_IRQChannelSubPriority = 1;
	nvicInitSt.NVIC_IRQChannelCmd = ENABLE;//DISABLE;
	NVIC_Init(&nvicInitSt);


	// Enable the USART2 gloabal Interrupt 
	nvicInitSt.NVIC_IRQChannel = USART2_IRQn;
	nvicInitSt.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitSt.NVIC_IRQChannelSubPriority = 1;
	nvicInitSt.NVIC_IRQChannelCmd = ENABLE;//DISABLE;
	NVIC_Init(&nvicInitSt);	

	nvicInitSt.NVIC_IRQChannel = USART3_IRQn;
	nvicInitSt.NVIC_IRQChannelPreemptionPriority = 1;
	nvicInitSt.NVIC_IRQChannelSubPriority = 0;
	nvicInitSt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInitSt);

	nvicInitSt.NVIC_IRQChannel = UART4_IRQn;
	nvicInitSt.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitSt.NVIC_IRQChannelSubPriority = 1;
	nvicInitSt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInitSt);

	//uart5
	nvicInitSt.NVIC_IRQChannel = UART5_IRQn;
	nvicInitSt.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitSt.NVIC_IRQChannelSubPriority = 1;
	nvicInitSt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInitSt);	

	//cfg uart1 and uart2
	gpioInitSt.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9;
	gpioInitSt.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitSt);

	gpioInitSt.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10;
	gpioInitSt.GPIO_Mode = GPIO_Mode_IPU;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitSt);

	//cfg uart3
	gpioInitSt.GPIO_Pin = UART3_TX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART3_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = UART3_RX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_IPU;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART3_PORT, &gpioInitSt);

	//cfg uart4
	gpioInitSt.GPIO_Pin = UART4_TX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART4_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = UART4_RX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_IPU;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART4_PORT, &gpioInitSt);

	//cfg uart5
	gpioInitSt.GPIO_Pin = UART5_TX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART5_TX_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = UART5_RX_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_IPU;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART5_RX_PORT, &gpioInitSt);

	//led pin cfg
	gpioInitSt.GPIO_Pin = LED_P_PIN|LED_W_PIN|LED_S_PIN|LED_T_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &gpioInitSt);

	//grps pin config
	gpioInitSt.GPIO_Pin = GPRS_PWRKEY_PIN|GPRS_SRST_PIN|GPRS_STATUS_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_CONTROL_PIN_PORT,&gpioInitSt);

	gpioInitSt.GPIO_Pin = GPRS_POWER_EN_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_POWER_EN_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = GPRS_POWER_EN_PIN_O;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_POWER_EN_PORT_O, &gpioInitSt);	

	gpioInitSt.GPIO_Pin = GPRS_POWER_CON_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_POWER_CON_PORT, &gpioInitSt);
	//GPRS_PWRKEY(0);
	//GPRS_SRST(0);


	//led key module
	//key
	gpioInitSt.GPIO_Pin = ALARM_KEY_PIN|SILENCER_KEY_PIN|SELF_TEST_KEY_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_IPU;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(ALARM_KEY_PORT, &gpioInitSt);
	/*end key*/
	
	//led  key mode buzzer
	gpioInitSt.GPIO_Pin = POWER_LED_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(POWER_LED_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = NET_LED_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(NET_LED_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = FAULT_LED_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(FAULT_LED_PORT, &gpioInitSt);	

	gpioInitSt.GPIO_Pin = ALARM_LED_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(ALARM_LED_PORT, &gpioInitSt);	
	/*end led*/

	//buzzer
	gpioInitSt.GPIO_Pin = BUZZER_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BUZZER_PORT, &gpioInitSt);
	BUZZER(1);
	//ALARM_LED(1);
	FAULT_LED(1);
	/**/
	

	//enc28j60
	gpioInitSt.GPIO_Pin = ENC28J60_RST_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(ENC28J60_RST_PORT, &gpioInitSt);


	//swch
	gpioInitSt.GPIO_Pin = SWITCH_1_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(SWITCH_1_PORT, &gpioInitSt);

	gpioInitSt.GPIO_Pin = SWITCH_2_PIN | SWITCH_3_PIN;
	gpioInitSt.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitSt.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(SWITCH_2_PORT, &gpioInitSt);

}




