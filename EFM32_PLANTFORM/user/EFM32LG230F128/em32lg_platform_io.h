#ifndef _EM32LG_PLANTFORM_IO_H
#define _EM32LG_PLANGFORM_IO_H

#ifdef EFM32LG230F128
/*RF pin config*/
#define RF_SDN_PORT      	gpioPortD
#define RF_SDN_PIN        	6

#define RF_nIRQ_PORT      	gpioPortC
#define RF_nIRQ_PIN       	12
                   
#define RF_GPIO0_PORT     	gpioPortC
#define RF_GPIO0_PIN      	14

#define RF_GPIO1_PORT     	gpioPortC
#define RF_GPIO1_PIN      	13  

#define RF_GPIO2_PORT     	gpioPortC
#define RF_GPIO2_PIN      	6

#define RF_GPIO3_PORT     	gpioPortD
#define RF_GPIO3_PIN      	7
/*RF spi pin*/
#define RF_SPICS_PORT		gpioPortC
#define RF_SPICS_PIN		8

#define RF_SPICLK_PORT		gpioPortC
#define RF_SPICLK_PIN		9

#define RF_SPIMISO_PORT		gpioPortC
#define RF_SPIMISO_PIN		10

#define RF_SPIMOSI_PORT		gpioPortC
#define RF_SPIMOSI_PIN		11


/*ADC PIN define */
#define ADC_CH_2_PORT		gpioPortD  //->ADC0_CH2
#define ADC_CH_2_pin		2
#define ADC_CH_1_PORT		gpioPortD //->ADC0_CH3
#define ADC_CH_1_PIN		3


/*uart leuart or usart pin config*/
#define US1_TX_PORT		gpioPortD
#define US1_TX_PIN		0
#define US1_TX_POS		1
#define US1_RX_PORT		gpioPortD
#define US1_RX_PIN		1
#define US1_RX_POS		1


#define LEU0_TX_PORT	gpioPortD
#define LEU0_TX_PIN		4
#define LEU0_TX_POS		0
#define LEU0_RX_PORT	gpioPortD
#define LEU0_RX_PIN		5
#define LEU0_RX_POS		0


/*iic pin config*/
#define I2C1_SDA_PORT	gpioPortC
#define I2C1_SDA_PIN	4
#define I2C1_SDA_POS	0
#define I2C1_SCL_PORT	gpioPortC
#define I2C1_SCL_PIN	5
#define I2C1_SCL_POS	0
#define I2C1_CS_PORT	gpioPortA
#define I2C1_CS_PIN		8

/*LED PIN config*/
#define USER_T_LED_PORT gpioPortA  //PA0
#define USER_T_LED_PIN	0

#define LED_B_PORT		gpioPortA //PA9
#define LED_B_PIN		9

#define LED_Y_PORT		gpioPortA //PA10
#define LED_Y_PIN		10

#define LED_F_PORT		gpioPortE //PE8
#define LED_F_PIN		8



//#define LED_OFF( )		GPIO_PinOutClear(USER_T_LED_PORT, USER_T_LED_PIN)
//#define LED_ON( )		GPIO_PinOutSet(USER_T_LED_PORT, USER_T_LED_PIN)

/*common io*/
#define P_COMMONIO1_PORT	gpioPortB//PB11
#define P_COMMONIO1_PIN		11
#define P_COMMONIO2_PORT	gpioPortE//PE10 -> US0_TX  may be to used usart bootloader
#define P_COMMONIO2_PIN		10
#define P_COMMONIO3_PORT	gpioPortE//PE11 -> US0_RX may be to used usart bootloader
#define P_COMMONIO3_PIN		11
#define P_COMMONIO4_PORT	gpioPortA//PA15
#define P_COMMONIO4_PIN		15

/*This two pin may be used as leuart 0*/
#define IO_LEUX_RX_PORT		gpioPortE//PE14
#define IO_LEUX_RX_PIN		14
#define IO_LEUX_TX_PORT		gpioPortE//pe15
#define IO_LEUX_TX_PIN		15


#define LED(a)					BUS_RegBitWrite(&GPIO->P[USER_T_LED_PORT].DOUT, USER_T_LED_PIN, a)
#define LED_Y(a)				BUS_RegBitWrite(&GPIO->P[LED_Y_PORT].DOUT, LED_Y_PIN, a)
#define LED_F(a)				BUS_RegBitWrite(&GPIO->P[LED_F_PORT].DOUT, LED_F_PIN, a)
#define LED_B(a)				BUS_RegBitWrite(&GPIO->P[LED_B_PORT].DOUT, LED_Y_PIN, a)
#define RF_SDN_SET(a)			BUS_RegBitWrite(&GPIO->P[RF_SDN_PORT].DOUT, RF_SDN_PIN, a)
#define GET_RF_nIRQ_PIN() 		GPIO_PinInGet(RF_nIRQ_PORT, RF_nIRQ_PIN)
#define GET_RF_GPIO0_PORT()		GPIO_PinInGet(RF_GPIO0_PORT, RF_GPIO0_PIN)
#define GET_RF_GPIO1_PORT()		GPIO_PinInGet(RF_GPIO1_PORT, RF_GPIO1_PIN)
#define GET_RF_GPIO2_PORT()		GPIO_PinInGet(RF_GPIO2_PORT, RF_GPIO2_PIN)
#define GET_RF_GPIO3_PORT()		GPIO_PinInGet(RF_GPIO3_PORT, RF_GPIO3_PIN)
#define I2C1_CS_SET(a)			BUS_RegBitWrite(&GPIO->P[I2C1_CS_PORT].DOUT, I2C1_CS_PIN, a)

#define LED4(a)			LED_Y(a)
#define LED3(a)			LED_F(a)



/*
#define RF_NIRQ()			GPIO_ReadInputDataBit(RF_NIRQ_PORT,RF_NIRQ_PIN)
#define	RF_NSEL(a)			sysSetPinStat(RF_NSEL_PORT,RF_NSEL_PIN,a)
#define	RF_SDN(a)			sysSetPinStat(RF_SDN_PORT,RF_SDN_PIN,a)
#define	RF_SDI(a)			sysSetPinStat(RF_SDI_PORT,RF_SDI_PIN,a)
#define	RF_SCLK(a)			sysSetPinStat(RF_SCLK_PORT,RF_SCLK_PIN,a)
#define RF_SDO()			GPIO_ReadInputDataBit(RF_SDO_PORT,RF_SDO_PIN)
*/

/*SPI IO common interface*/
#define RF_SPINSEL(a)		BUS_RegBitWrite(&GPIO->P[RF_SPICS_PORT].DOUT, RF_SPICS_PIN, a)			//spi cs
#define RF_SDN(a)			BUS_RegBitWrite(&GPIO->P[RF_SDN_PORT].DOUT, RF_SDN_PIN, a)				//rf sdn
#define RF_SDI(a)			BUS_RegBitWrite(&GPIO->P[RF_SPIMOSI_PORT].DOUT, RF_SPIMOSI_PIN, a)		//spi mosi
#define RF_SCLK(a)			BUS_RegBitWrite(&GPIO->P[RF_SPICLK_PORT].DOUT, RF_SPICLK_PIN, a)		//spi clk
#define RF_SDO()			GPIO_PinInGet(RF_SPIMISO_PORT, RF_SPIMISO_PIN)							//spi miso
#define RF_NIRQS			GET_RF_nIRQ_PIN()														//rf nirq state



/*interrupt define*/

#define RF_nIRQ_FLAG	(1<<RF_nIRQ_PIN)
#define INT_RF_nIRQ_GROUP	0			//0->7
#define INT_RF_nIRQ_PREP	0
#define INT_RF_nIRQ_SUBP	0

#define INT_USART1_nIRQ_GROUP	0
#define INT_USART1_nIRQ_PREP	0
#define INT_USART1_nIRQ_SUBP	1

#define INT_SYSTICK_nIRQ_GROUP	3
#define INT_SYSTICK_nIRQ_PREP	3
#define INT_SYSTICK_nIRQ_SUBP	3

#define INT_LETIMER0_nIRQ_GROUP 3
#define INT_LETIMER0_nIRQ_PREP  3
#define INT_LETIMER0_nIRQ_SUBP	2


#endif /**/
#endif
