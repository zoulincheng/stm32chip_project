#include "em32lg_config.h"

#include "basictype.h"
#include "sysprintf.h"
#include "em32_uart.h"


/*************************************************************************//**
 * @brief energyAware Designer MCU initialization
 *
 * This code is generated by the energyAware Designer appliction to configure
 * the MCU for application specific operation. 
 *
 * The generated code is a starting point, which might require adjustment for
 * correct operation. Call this function at early initialization.
 *****************************************************************************/
void Hardware_Init(void)
{
#ifdef EFM32LG230F128
  /* Using HFRCO at 14MHz as high frequency clock, HFCLK */
  
  /* No LE clock source selected */
  
  /* Enable GPIO clock */
  //CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  /* Pin PB9 is configured to Input enabled */
  GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~_GPIO_P_MODEH_MODE9_MASK) | GPIO_P_MODEH_MODE9_INPUT;
  /* Pin PB10 is configured to Input enabled */
  GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~_GPIO_P_MODEH_MODE10_MASK) | GPIO_P_MODEH_MODE10_INPUT;
  /* Pin PB11 is configured to Input enabled */
  GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~_GPIO_P_MODEH_MODE11_MASK) | GPIO_P_MODEH_MODE11_INPUT;
  /* Pin PB12 is configured to Input enabled */
  GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~_GPIO_P_MODEH_MODE12_MASK) | GPIO_P_MODEH_MODE12_INPUT;
  /* Pin PC0 is configured to Input enabled */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_INPUT;
  /* Pin PC3 is configured to Push-pull */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;
  /* Pin PC4 is configured to Input enabled */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE4_MASK) | GPIO_P_MODEL_MODE4_INPUT;
  /* Pin PC5 is configured to Input enabled */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE5_MASK) | GPIO_P_MODEL_MODE5_INPUT;

  /* To avoid false start, configure output US1_TX as high on PD0 */
  GPIO->P[3].DOUT |= (1 << 0);
  /* Pin PD0 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_PUSHPULL;
  /* Pin PD1 is configured to Input enabled */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_INPUT;
  /* Pin PD2 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;
  /* To avoid false start, configure output US1_CS as high on PD3 */
  GPIO->P[3].DOUT |= (1 << 3);
  /* Pin PD3 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;
  /* Pin PE2 is configured to Push-pull */
  GPIO->P[4].MODEL = (GPIO->P[4].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;
  /* Pin PE3 is configured to Push-pull */
  GPIO->P[4].MODEL = (GPIO->P[4].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;

  /* Enable clock for USART1 */
  CMU_ClockEnable(cmuClock_USART1, true);
  /* Custom initialization for USART1 */
  //vSpiInitialize();
  /* Module USART1 is configured to location 1 */
  USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK) | USART_ROUTE_LOCATION_LOC1;
  /* Enable signals TX, RX, CLK, CS not enable */
  USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;

  /* Enabling TX and RX */
  USART1->CMD = USART_CMD_TXEN | USART_CMD_RXEN;
  /* Clear previous interrupts */
  USART1->IFC = _USART_IFC_MASK;
#endif
 
#ifdef EFM32TG840F32
  /* Using HFRCO at 14MHz as high frequency clock, HFCLK */
  
  /* No LE clock source selected */
  
  /* Enable GPIO clock */
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  //GPIO0-GPIO3 Input
  GPIO_PinModeSet(RF_GPIO0_PORT, RF_GPIO0_PIN, gpioModeInputPull, 0);
  GPIO_PinModeSet(RF_GPIO1_PORT, RF_GPIO1_PIN, gpioModeInputPull, 0);
  GPIO_PinModeSet(RF_GPIO2_PORT, RF_GPIO2_PIN, gpioModeInputPull, 0);
  GPIO_PinModeSet(RF_GPIO3_PORT, RF_GPIO3_PIN, gpioModeInputPull, 0);
  
  /* Pin PD4 is configured to Input enabled */
  GPIO_PinModeSet(RF_nIRQ_PORT, RF_nIRQ_PIN, gpioModeInputPull, 1);
  /* Pin PA12 is configured to Push-pull */
  GPIO_PinModeSet(RF_SDN_PORT, RF_SDN_PIN, gpioModePushPull, 1);
  
  /* To avoid false start, configure output US1_TX as high on PD0 */
  GPIO->P[3].DOUT |= (1 << 0);
  /* Pin PD0 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_PUSHPULL;
  /* Pin PD1 is configured to Input enabled */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_INPUT;
  /* Pin PD2 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;
  /* Pin PD3 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;
  /* Pin PD7 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE7_MASK) | GPIO_P_MODEL_MODE7_PUSHPULL;
  /* Pin PD8 is configured to Input enabled */
  GPIO->P[3].MODEH = (GPIO->P[3].MODEH & ~_GPIO_P_MODEH_MODE8_MASK) | GPIO_P_MODEH_MODE8_INPUT;
  
  /* Enable clock for USART1 */
  CMU_ClockEnable(cmuClock_USART1, true);
  /* Custom initialization for USART1 */
  //vSpiInitialize();
  /* Module USART1 is configured to location 1 */
  USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK) | USART_ROUTE_LOCATION_LOC1;
  /* Enable signals TX, RX, CLK, CS not enable */
  USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;  
  
  /* Enabling TX and RX */
  USART1->CMD = USART_CMD_TXEN | USART_CMD_RXEN;
  /* Clear previous interrupts */
  USART1->IFC = _USART_IFC_MASK;  
#endif  
}

extern int uart0_send_char (int ch);



extern int serial_receive_byte(unsigned char c);

void sys_init(void)
{
	/*
	* Now enanle all osc source, may be disable 
	*/
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	//CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	//CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

	/* Select 48 MHz external crystal oscillator */
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	/* Start LFXO, and use LFXO for low-energy modules */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	/* Power up trace and debug clocks. Needed for DWT. */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	/* Enable DWT cycle counter. Used to measure transfer speed. */
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_HFPER, true);
	/*enable gpio clk*/
	CMU_ClockEnable(cmuClock_GPIO, true);
	/*enable clk usart0 is used for spi*/
	CMU_ClockEnable(cmuClock_USART0, true);
	/*enable usart1 clk, this is used for debug*/
	CMU_ClockEnable(cmuClock_USART1, true);
	/* Enable CORE LE clock in order to access LE modules */
	CMU_ClockEnable(cmuClock_CORELE, true);
	
	/*ioconfig*/
	//RF pin config
	GPIO_PinModeSet(RF_GPIO0_PORT, RF_GPIO0_PIN, gpioModeInputPull, 0);
	GPIO_PinModeSet(RF_GPIO1_PORT, RF_GPIO1_PIN, gpioModeInputPull, 0);
	GPIO_PinModeSet(RF_GPIO2_PORT, RF_GPIO2_PIN, gpioModeInputPull, 0);
	GPIO_PinModeSet(RF_GPIO3_PORT, RF_GPIO3_PIN, gpioModeInputPull, 0);
	GPIO_PinModeSet(RF_nIRQ_PORT, RF_nIRQ_PIN, gpioModeInputPull, 1);
	GPIO_PinModeSet(RF_SDN_PORT, RF_SDN_PIN, gpioModePushPull, 1);

	/* Pin LED is configured to out enable */
	GPIO_PinModeSet(USER_T_LED_PORT, USER_T_LED_PIN, gpioModePushPull, 0);	
	GPIO_PinModeSet(LED_B_PORT, LED_B_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(LED_Y_PORT, LED_Y_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(LED_F_PORT, LED_F_PIN, gpioModePushPull, 0);

	/*debug usart1*/
	usart1_init( );
	xdev_out(usart1_sendbyte);
	//Uart_StdSetInput(serial_line_input_byte);	//For shell read data  

	//
	leuart0_init( );

	//clock_init and sys init
	//clock_init( );

	//user led test

}

/*-----------------------------------------------------------------------------------------------*/


int main(void)
{
	u_short uwRandrom;
	/* Chip errata */
	CHIP_Init( );
	sys_init( );
	XPRINTF((0, "randdomseed = %d \r\n", uwRandrom));


	XPRINTF((0,"Processes running\r\n"));
	while(1) 
	{

	}
}



