#ifndef _IODEF_H
#define _IODEF_H

#define 		LOW		0
#define			HIGH	1


//test led green PC12
#define sysSetPinStat(port,pin,stat) GPIO_WriteBit(port,pin,(BitAction)((stat)!=0))
#define sysSetPinStatReverse(port,pin) GPIO_WriteBit(port,pin,GPIO_ReadOutputDataBit(port,pin))


//led config
#define LED_PORT		GPIOB
#define LED_P_PIN		GPIO_Pin_9
#define LED_W_PIN		GPIO_Pin_8
#define LED_S_PIN		GPIO_Pin_7
#define LED_T_PIN		GPIO_Pin_6

//led operation
#define LED_P(a)		sysSetPinStat(LED_PORT,LED_P_PIN,a)
#define LED_W(a)		sysSetPinStat(LED_PORT,LED_W_PIN,a)
#define LED_S(a)		sysSetPinStat(LED_PORT,LED_S_PIN,a)
#define LED_T(a)		sysSetPinStat(LED_PORT,LED_T_PIN,a)

//grps pin define  gprs config
#define GPRS_STATUS_PIN			GPIO_Pin_5
#define GPRS_PWRKEY_PIN			GPIO_Pin_4
#define GPRS_SRST_PIN			GPIO_Pin_3
#define GPRS_CONTROL_PIN_PORT	GPIOB

#define GPRS_POWER_EN_PIN_O		GPIO_Pin_15
#define GPRS_POWER_EN_PORT_O	GPIOA
/*
20160919
*/
#define GPRS_POWER_EN_PIN		GPIO_Pin_5
#define GPRS_POWER_EN_PORT		GPIOB

#define GPRS_POWER_CON_PIN		GPIO_Pin_1
#define GPRS_POWER_CON_PORT		GPIOA



//#define GPRS_STATUS()		((GPRS_CONTROL_PIN_PORT->IDR & GPRS_STATUS_PIN) ? 1 : 0)
#define GPRS_STATUS(a)		sysSetPinStat(GPRS_CONTROL_PIN_PORT,GPRS_STATUS_PIN,a)
#define GPRS_PWRKEY(a)		sysSetPinStat(GPRS_CONTROL_PIN_PORT,GPRS_PWRKEY_PIN,a)
#define GPRS_SRST(a)		sysSetPinStat(GPRS_CONTROL_PIN_PORT,GPRS_SRST_PIN,a)
#define GPRS_EN(a)			sysSetPinStat(GPRS_POWER_EN_PORT, GPRS_POWER_EN_PIN, a)
#define GPRS_CON(a)			sysSetPinStat(GPRS_POWER_CON_PORT, GPRS_POWER_CON_PIN, a)
#define GPRS_ENOLD(a)		sysSetPinStat(GPRS_POWER_EN_PORT_O, GPRS_POWER_EN_PIN_O, a)

//can bus config
#define CAN_TX_PIN		GPIO_Pin_12
#define CAN_RX_PIN		GPIO_Pin_11
#define CAN_PORT		GPIOA

//usart config
//debug port
#define UART1_TX_PIN	GPIO_Pin_9
#define UART1_RX_PIN	GPIO_Pin_10
#define UART1_PORT		GPIOA

//485 port
#define UART2_TX_PIN	GPIO_Pin_2
#define UART2_RX_PIN	GPIO_Pin_3
#define UART2_PORT		UART1_PORT

//RF PORT
#define UART3_TX_PIN	GPIO_Pin_10
#define UART3_RX_PIN	GPIO_Pin_11
#define UART3_PORT		GPIOB

//GPRS PORT
#define UART4_TX_PIN	GPIO_Pin_10
#define UART4_RX_PIN	GPIO_Pin_11
#define UART4_PORT		GPIOC

#define UART5_TX_PIN	GPIO_Pin_12
#define UART5_TX_PORT	UART4_PORT
#define UART5_RX_PIN	GPIO_Pin_2
#define UART5_RX_PORT	GPIOD

//ethernet config
#define ENC_RST_PIN		GPIO_Pin_7
#define ENC_RST_PORT	GPIOC
#define ENC_INT_PIN		GPIO_Pin_6
#define ENC_INT_PORT	GPIOC

#define SPI2_MOSI_PIN	GPIO_Pin_15
#define SPI2_MISO_PIN	GPIO_Pin_14
#define SPI2_SCK_PIN	GPIO_Pin_13
#define SPI2_NSS_PIN	GPIO_Pin_12
#define SPI2_PORT		GPIOB

#define SPI2_NSS(a)		sysSetPinStat(SPI2_PORT,SPI2_NSS_PIN,a)
#define SPI2_MOSI(a)	sysSetPinStat(SPI2_PORT,SPI2_MOSI_PIN,a)
#define SPI2_SCK(a)		sysSetPinStat(SPI2_PORT,SPI2_SCK_PIN,a)
#define SPI2_MISO()		((SPI2_PORT->IDR & SPI2_MISO_PIN) ? 1 : 0)


//key
#define ALARM_KEY_PIN	GPIO_Pin_4
#define ALARM_KEY_PORT	GPIOA
#define ALARM_KEY_V( )	(GPIOA->IDR&ALARM_KEY_PIN)//((ALARM_KEY_PORT->IDR & ALARM_KEY_PIN)? 1 : 0)

#if 0
#define SILENCER_KEY_PIN	GPIO_Pin_5
#define SILENCER_KEY_PORT	GPIOA
#define SILENCER_KEY_V( )	(GPIOA->IDR&SILENCER_KEY_PIN) //((SILENCER_KEY_PORT->IDR & SILENCER_KEY_PIN) ? 1 : 0)

#define SELF_TEST_KEY_PIN	GPIO_Pin_7
#define SELF_TEST_KEY_PORT	GPIOA
#define SELF_TEST_KEY_V( )	(GPIOA->IDR&SELF_TEST_KEY_PIN)	//((SELF_TEST_KEY_PORT->IDR&SELF_TEST_KEY_PIN) ? 1 : 0)
#else
#define SELF_TEST_KEY_PIN	GPIO_Pin_5
#define SELF_TEST_KEY_PORT	GPIOA
#define SELF_TEST_KEY_V( )	(GPIOA->IDR&SILENCER_KEY_PIN) //((SILENCER_KEY_PORT->IDR & SILENCER_KEY_PIN) ? 1 : 0)

#define SILENCER_KEY_PIN	GPIO_Pin_7
#define SILENCER_KEY_PORT	GPIOA
#define SILENCER_KEY_V( )	(GPIOA->IDR&SELF_TEST_KEY_PIN)	//((SELF_TEST_KEY_PORT->IDR&SELF_TEST_KEY_PIN) ? 1 : 0)
#endif

#define		KEY_PORT		GPIOA
#define		KEY_VALUE		(ALARM_KEY_PIN|SILENCER_KEY_PIN|SELF_TEST_KEY_PIN)



//state led
#define POWER_LED_PIN		GPIO_Pin_6
#define POWER_LED_PORT		GPIOA
#define POWER_LED(a)		sysSetPinStat(POWER_LED_PORT,POWER_LED_PIN,a)

#define NET_LED_PIN			GPIO_Pin_4
#define NET_LED_PORT		GPIOC
#define NET_LED(a)			sysSetPinStat(NET_LED_PORT,NET_LED_PIN,a)

#define ALARM_LED_PIN		GPIO_Pin_0
#define ALARM_LED_PORT		GPIOB
#define ALARM_LED(a)		sysSetPinStat(ALARM_LED_PORT,ALARM_LED_PIN,a)


#define FAULT_LED_PIN		GPIO_Pin_1
#define FAULT_LED_PORT		GPIOB
#define FAULT_LED(a)		sysSetPinStat(FAULT_LED_PORT,FAULT_LED_PIN,a)

//buzzer
#define BUZZER_PIN			GPIO_Pin_5
#define BUZZER_PORT			GPIOC
#define BUZZER(a)			sysSetPinStat(BUZZER_PORT,BUZZER_PIN,a)


/*
20160919
*/
#define SWITCH_1_PIN		GPIO_Pin_8
#define SWITCH_1_PORT		GPIOA
#define SWITCH_1(a)			sysSetPinStat(SWITCH_1_PORT,SWITCH_1_PIN,a)

#define SWITCH_2_PIN		GPIO_Pin_9
#define SWITCH_2_PORT		GPIOC
#define SWITCH_2(a)			sysSetPinStat(SWITCH_2_PORT,SWITCH_2_PIN,a)

#define SWITCH_3_PIN		GPIO_Pin_8
#define SWITCH_3_PORT		GPIOC
#define SWITCH_3(a)			sysSetPinStat(SWITCH_3_PORT,SWITCH_3_PIN,a)

//end20160919


//enc28j60
#define ENC28J60_RST_PIN	GPIO_Pin_7
#define ENC28J60_RST_PORT	GPIOC
#define ENC28J60_RST(a)			sysSetPinStat(ENC28J60_RST_PORT,ENC28J60_RST_PIN,a)

		                                
#endif



