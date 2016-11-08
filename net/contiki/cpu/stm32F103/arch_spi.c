#include "contiki-conf.h"
#include "stm32f10x.h"
#include "iodef.h"
#include "basictype.h"
#include "atom.h"

#include "sysprintf.h"

#include "arch_spi.h"


extern void SI4432_NIRQ_Config(void);
#define			HARDSPI
//#define			SOFTSPI
//#define 		SOFTSPI_DEMO

#ifdef	HARDSPI
//hardware spi
//config interface between MCU and si4432
#define SPI_WAIT_TIME 	100
void SPI_Config(void)
{
	GPIO_InitTypeDef strGPIO;
	SPI_InitTypeDef	strSPI;
	
	//clk config
	//because in the arch-rtimer ,the APB1 clk is config to 18M
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//spi af pin cfg
	strGPIO.GPIO_Pin = SPI2_MISO_PIN|SPI2_MOSI_PIN|SPI2_SCK_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_AF_PP; //���ù���
	GPIO_Init(SPI2_PORT, &strGPIO); 

	//SPI nss pin cfg
	strGPIO.GPIO_Pin = SPI2_NSS_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_10MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; //
	GPIO_Init(SPI2_PORT, &strGPIO); 

#if 0	

	//si4432
	strSPI.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //˫��ȫ˫��
	strSPI.SPI_Mode = SPI_Mode_Master;   			 		//����Ϊ��SPI                                             //��ģʽ
	strSPI.SPI_DataSize = SPI_DataSize_8b;     				//SPI���ͽ���8λ֡�ṹ                                    //���ݴ�С8λ
	strSPI.SPI_CPOL = SPI_CPOL_High; 				//����ͬ��ʱ�ӵĿ���״̬Ϊ �͵�ƽ                                                //ʱ�Ӽ��ԣ�����ʱΪ��
	strSPI.SPI_CPHA = SPI_CPHA_2Edge; 					//����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����						                                               //��1��������Ч��������Ϊ����ʱ��
	strSPI.SPI_NSS = SPI_NSS_Soft; 							//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����                                                          //NSS�ź����������
	strSPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;  //18M 4 ��Ƶ��4.5M
	
	strSPI.SPI_FirstBit = SPI_FirstBit_MSB;                  //��λ��ǰ
	strSPI.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ


	//enc28j60
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;							//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						//����ͬ��ʱ�ӵĵ�һ�������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;							//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  								//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	
#else
	//si4438 spi interface config
	strSPI.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //˫��ȫ˫��
	strSPI.SPI_Mode = SPI_Mode_Master;   			 		//����Ϊ��SPI                                             //��ģʽ
	strSPI.SPI_DataSize = SPI_DataSize_8b;     				//SPI���ͽ���8λ֡�ṹ                                    //���ݴ�С8λ
	strSPI.SPI_CPOL = SPI_CPOL_Low; 				//����ͬ��ʱ�ӵĿ���״̬Ϊ �͵�ƽ                                                //ʱ�Ӽ��ԣ�����ʱΪ��
	strSPI.SPI_CPHA = SPI_CPHA_1Edge; 					//����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����						                                               //��1��������Ч��������Ϊ����ʱ��
	strSPI.SPI_NSS = SPI_NSS_Soft; 							//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����                                                          //NSS�ź����������
	strSPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;  //72M 8 ��Ƶ��4.5M
	strSPI.SPI_FirstBit = SPI_FirstBit_MSB;                  //��λ��ǰ
	strSPI.SPI_CRCPolynomial = 7;							//CRCֵ����Ķ���ʽ
#endif
	SPI_Init(SPI2, &strSPI);
	SPI_Cmd(SPI2, ENABLE);
	//SPI_SendByteData(0xff);

}



//send a byte data
u_char SPI_SendByteData(u_char ubData)
{
	/* Wait for SPI2 Tx buffer empty */

	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET));
	/*
	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET))
	{	
		udwWaitTime++;
		if (udwWaitTime > SPI_WAIT_TIME)
		{
			break;
		}
	}
	*/
	/* Send SPI2 data */
	SPI_I2S_SendData(SPI2, ubData);


	/* Wait for SPI2 data reception */
	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET));
	/*
	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET))
	{
		udwWaitTime++;
		if (udwWaitTime > SPI_WAIT_TIME)
		{
			break;
		}
	}
	*/
	/* Read SPI2 received data */
	return SPI_I2S_ReceiveData(SPI2);
}



//send a byte data
u_char SPI_SendByte(u_char ubData)
{
	/* Wait for SPI2 Tx buffer empty */
	unsigned int udwWaitTime = 0;
	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) && (udwWaitTime ++ < SPI_WAIT_TIME));
	/* Send SPI2 data */
	SPI_I2S_SendData(SPI2, ubData);

	udwWaitTime = 0;
	/* Wait for SPI2 data reception */
	while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) && (udwWaitTime++ < SPI_WAIT_TIME));
	/* Read SPI2 received data */
	return SPI_I2S_ReceiveData(SPI2);
}


u_char SPI_ReadByte(u_char ubdata)
{
	return SPI_SendByte(ubdata);
}

#endif



#ifdef SOFTSPI
void SPI_Config(void)
{
	GPIO_InitTypeDef strGPIO;
	
	//clk config
	//because in the arch-rtimer ,the APB1 clk is config to 18M
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	
	// SPI pin GPIO Config Out put
	strGPIO.GPIO_Pin = SPI2_SCK_PIN|SPI2_MOSI_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_10MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(SPI2_PORT, &strGPIO); 

	// interface SPI pin GPIO Config input
	strGPIO.GPIO_Pin = SPI2_MISO_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_10MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_IPU; //��������
	GPIO_Init(SPI2_PORT, &strGPIO); 

	//interface spi nss Config
	strGPIO.GPIO_Pin = SPI2_NSS_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_10MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; //
	GPIO_Init(SPI2_PORT, &strGPIO); 
	
	SPI2_NSS(1);
}


/************************************************************************** 
 * ��������SPI_SendByte 
 * ����  ��SPIģ�鷢�ͺ��� 
 * ����  ���������� 
 * ����  ���������� 
 *************************************************************************/  
u8 SPI_SendByteData(u8 byte)  
{       
	u8 i = 0;  
	u8 bit_r = 0; 
	for(i=0;i<8;i++)   // output 8-bit  
	{   
		//MOSI_PIN=byte & 0x80;     //output 'byte' MSB to MOSI_PIN  
		if(byte & 0x80)
			SI4432SDI(1);  
		else
			SI4432SDI(0); 

		byte <<= 1;                 // shift next bit into MSB..  

		bit_r<<=1;  
		if(SI4432SDO) bit_r++;  

		SI4432SCLK(1); 
		SI4432SCLK(1); 
		SI4432SCLK(1); 

		SI4432SCLK(0);
		SI4432SCLK(0);
		SI4432SCLK(0);
	}  
	return(bit_r);                        // return read byte  
}  

#endif



#ifdef SOFTSPI_DEMO
void SPI_Config(void)
{
	GPIO_InitTypeDef strGPIO;
	
	//clk config
	//because in the arch-rtimer ,the APB1 clk is config to 18M
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	//interface SPI pin GPIO Config Out put
	/*RF SDI*/
	strGPIO.GPIO_Pin = RF_SDI_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(RF_SDI_PORT, &strGPIO); 

	/*RF SCLK*/
	strGPIO.GPIO_Pin = RF_SCLK_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(RF_SCLK_PORT, &strGPIO); 	

	// interface SPI pin GPIO Config input
	/*RF SDO*/
	strGPIO.GPIO_Pin = RF_SDO_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_IPU; //��������
	GPIO_Init(RF_SDO_PORT, &strGPIO); 

	// interface nSEL Config
	strGPIO.GPIO_Pin = RF_NSEL_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; //
	GPIO_Init(RF_NSEL_PORT, &strGPIO); 
	
	// interface nIRQ Pin config
	strGPIO.GPIO_Pin = RF_NIRQ_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_IPU; //
	GPIO_Init(RF_NIRQ_PORT, &strGPIO); 

	//interface SDN Config
	strGPIO.GPIO_Pin = RF_SDN_PIN;
	strGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	strGPIO.GPIO_Mode = GPIO_Mode_Out_PP; //
	GPIO_Init(RF_SDN_PORT, &strGPIO); 

//	RF_SDN(0);
//	RF_SCLK(0); 
}


/************************************************************************** 
 * ��������SPI_SendByte 
 * ����  ��SPIģ�鷢�ͺ��� 
 * ����  ���������� 
 * ����  ���������� 
 *************************************************************************/  
u8 SPI_SendByteData(u8 byte)  
{       
	u8 i = 0;  
	u8 bit_r = 0; 
	u8 j = 0;
	for(i=0;i<8;i++)   // output 8-bit  
	{   
		//MOSI_PIN=byte & 0x80;     //output 'byte' MSB to MOSI_PIN  
		if(byte & 0x80)
			RF_SDI(1);  
		else
			RF_SDI(0); 

		byte <<= 1;                 // shift next bit into MSB..  
		bit_r<<=1;  
		if(RF_SDO()) 
			bit_r++;  

		RF_SCLK(1); 
		
		RF_SCLK(0);
	}  
	return(bit_r);                        // return read byte  
}  

#endif





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
    *paboDataOut++ = SPI_SendByteData(0xff);
  }
}




