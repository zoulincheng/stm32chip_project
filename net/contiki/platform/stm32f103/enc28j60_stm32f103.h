#ifndef _ENC28J60_STM32F103_H
#define _ENC28J60_STM32F103_H




void enc28j60_arch_spi_init(void);
uint8_t enc28j60_arch_spi_write(uint8_t data);
uint8_t enc28j60_arch_spi_read(void);
void enc28j60_arch_spi_select(void);
void enc28j60_arch_spi_deselect(void);


#endif

