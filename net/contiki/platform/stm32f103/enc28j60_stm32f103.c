#include "contiki.h"
#include "basictype.h"
#include "arch_spi.h"
#include "stm32f10x.h"
#include "iodef.h"
#include "enc28j60_stm32f103.h"

/*
void enc28j60_arch_spi_init(void);
uint8_t enc28j60_arch_spi_write(uint8_t data);
uint8_t enc28j60_arch_spi_read(void);
void enc28j60_arch_spi_select(void);
void enc28j60_arch_spi_deselect(void);
*/

void enc28j60_arch_spi_init(void)
{
	//GPIO_InitTypeDef strGPIO;
	SPI_Config( );
}

uint8_t enc28j60_arch_spi_write(uint8_t data)
{
	#if 0
	SPDR = data;
	while(!(SPSR & (1 << SPIF))) ;
	return SPDR;
	#else
	return SPI_SendByteData(data);
	#endif
}

uint8_t enc28j60_arch_spi_read(void)
{
	#if 0
	SPDR = 0xAA; /* dummy */
	while(!(SPSR & (1 << SPIF))) ;
	return SPDR;
	#else
	return SPI_SendByteData(0xAA);
	#endif
}

void enc28j60_arch_spi_select(void)
{
	#if 0
	CS_SPI_PORT &= ~(1 << SPI_CS);
	delay_us(1000);
	#else
	SPI2_NSS(0);
	clock_wait(1); //wait 1 ms
	#endif
}


void enc28j60_arch_spi_deselect(void)
{
	#if 0
	CS_SPI_PORT |= (1 << SPI_CS);
	#else
	SPI2_NSS(1);
	#endif
}


