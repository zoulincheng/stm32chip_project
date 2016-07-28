#include "em32lg_config.h"
#include "arch_spi.h"
 
/*!
 * This function is used to initialize the SPI port.
 *
 *  @return None
 */
void SPI_Config(void)
{
  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
  USART_TypeDef *spi = USART0;

  init.baudrate     = 5000000;
  init.databits     = usartDatabits8;
  init.msbf         = 1;
  init.master       = 1;
  init.clockMode    = usartClockMode0;
  init.prsRxEnable  = 0;
  init.autoTx       = 0;

  USART_InitSync(USART0, &init);
  /* Clear previous interrupts */
  spi->IFC = _USART_IFC_MASK;

  /* configured to location 2 */
  spi->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK) | USART_ROUTE_LOCATION_LOC2;
  /* Enable signals TX, RX, CLK, CS not enable */
  spi->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;

  /* IO configuration (USART 1, Location #1) */
  GPIO_PinModeSet(RF_SPIMOSI_PORT, RF_SPIMOSI_PIN, gpioModePushPull, 1);
  GPIO_PinModeSet(RF_SPIMISO_PORT, RF_SPIMISO_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(RF_SPICLK_PORT, RF_SPICLK_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(RF_SPICS_PORT, RF_SPICS_PIN, gpioModePushPull, 1);
}



/*!
* This function is used to read/write one byte from/to SPI port (target: EzRadioPRO).
 *
 * @param[in] biDataIn    Data to be sent.
 * @return  Read value of the SPI port after writing on it.
 */   
uint8_t SPI_SendByteData(uint8_t biDataIn)
{  
  USART_TypeDef *spi = USART0;
 
  spi->TXDATA = biDataIn;
  while (!(spi->STATUS & USART_STATUS_TXC))
  {
  }
  return (uint8_t)(spi->RXDATA);
}



/*!
 * This function is used to send data over SPI port (target: EzRadioPRO).no response expected.
 *
 *  @param[in] biDataInLength  The length of the data.
 *  @param[in] *pabiDataIn     Pointer to the first element of the data.
 *
 *  @return None
 */
void SpiWriteDataBurst(uint8_t biDataInLength, uint8_t *pabiDataIn)
{
  while (biDataInLength--) 
  {
    SPI_SendByteData(*pabiDataIn++);
  }
}



/*!
 * This function is used to read data from SPI port.(target: EzRadioPRO).
 *
 *  \param[in] biDataOutLength  The length of the data.
 *  \param[out] *paboDataOut    Pointer to the first element of the response.
 *
 *  \return None
 */
void SpiReadDataBurst(uint8_t biDataOutLength, uint8_t *paboDataOut)
{
  // send command and get response from the radio IC
  while (biDataOutLength--) {
    *paboDataOut++ = SPI_SendByteData(0xFF);
  }
}




/**
 *  Pull down nSEL of the selected device.
 *
 *  
 *
 *  
 *
 ******************************************************************************/
void Spi_ClearNsel(void)
{
  GPIO_PinOutClear(RF_SPICS_PORT, RF_SPICS_PIN);
}




/**
 *  Pull-up nSEL of the selected device.
 *
 * 
 *
 *  
 *
 ******************************************************************************/
void Spi_SetNsel( )
{
  GPIO_PinOutSet(RF_SPICS_PORT, RF_SPICS_PIN);
}


