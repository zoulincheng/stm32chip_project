#ifndef _ARCH_SPI_H
#define _ARCH_SPI_H


void SPI_Config(void);
uint8_t SPI_SendByteData(uint8_t biDataIn);
void SpiWriteDataBurst(uint8_t biDataInLength, uint8_t *pabiDataIn);
void SpiReadDataBurst(uint8_t biDataOutLength, uint8_t *paboDataOut);


#endif




