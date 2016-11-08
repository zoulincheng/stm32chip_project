#ifndef _ARCH_SPI_H
#define _ARCH_SPI_H



void SPI_Config(void);

//void SPI_SendByteData(u_char ubData);
u_char SPI_SendByteData(u_char ubData);

u_char SPI_ReadByteData(void);
void SpiWriteDataBurst(uint8_t biDataInLength, uint8_t *pabiDataIn);
void SpiReadDataBurst(uint8_t biDataOutLength, uint8_t *paboDataOut);


#endif


