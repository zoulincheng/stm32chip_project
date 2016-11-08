#ifndef _SI4432_CONFIG_CONST_H
#define _SI4432_CONFIG_CONST_H

//si4432 carrier setting  470Mhz
const u_char rf_txrx_carrier[][2]=
{
	{0x75,0x57},
	{0x76,0x00},
	{0x77,0x00}
};


//si4432 tx data rate setting 9.6kps
const u_char rf_txrate[][2]=
{
	{0x6e, 0x4e},
	{0x6f, 0xa5},
	{0x70, 0x2c},
	{0x58, 0x80}
};

//si4432 tx feq deviation setting +-45kHz
const u_char rf_txfd[][2]=
{
	{0x72, 0x48},
	{0x71, 0x23}
};



//si4432 gfsk/fsk rx modem settings
//9.6kps  fd+-45kHz AFC enable sampling clock 249.600kHZ BW
const u_char rf_gfsk_fsk_rx[][2]=
{
	{0x1c, 0x1e},
	{0x20, 0xd0},
	{0x21, 0x00},
	{0x22, 0x9d},
	{0x23, 0x49},
	{0x24, 0x10},
	{0x25, 0x45},
	{0x1d, 0x44},
	{0x1e, 0x0a},
	{0x2a, 0x20},
	{0x1f, 0x00},
	{0x69, 0x60}
};



//si4432 preamble + sync + header + data + crc
//come with si tools excel 
const u_char rf_phfif_mode[][2]=
{
	{0x30, 0xad},
	{0x32, 0x8c},
	{0x33, 0x0a},
	{0x34, 0x08},
	{0x35, 0x2a},
	{0x36, 0x2d},
	{0x37, 0xd4},
	{0x38, 0x00},
	{0x39, 0x00},
	{0x3a, 0x00},
	{0x3b, 0x00},
	{0x3c, 0x00},
	{0x3d, 0x00},
	{0x3e, 0x01},
	{0x3f, 0x00},
	{0x40, 0x00},
	{0x41, 0x00},
	{0x42, 0x00},
	{0x43, 0xff},
	{0x44, 0xff},
	{0x45, 0xff},
	{0x46, 0xff},
	{0x70, 0x2c}, 
	{0x71, 0x23}
};


/*
470Mhz   9.6KPS    FD +-45   <PREAMBLE><SYNC><DATA><CRC>	
REG VALUE
*/
const u_char si4432_config_regs[][2]=
{
	{0x1C,	0x1E},//	S2 9C1E		These registers are for RX modem ONLY
	{0x1D,	0x44},//	S2 9D44		
	{0x1E,	0x0A},//	S2 9E0A		
	{0x1F,	0x00},//	S2 9F00		
	{0x20,	0xD0},//	S2 A0D0		These registers are for RX modem ONLY
	{0x21,	0x00},//	S2 A100		
	{0x22,	0x9D},//	S2 A29D		
	{0x23, 	0x49},//	S2 A349		
	{0x24,	0x10},//	S2 A410		
	{0x25,	0x45},//	S2 A545		
	{0x2A,	0x20},//	S2 AA20		
	{0x2C,	0x28},//	N/A	This setting is relevant only in OOK	
	{0x2D,	0x82},//	N/A	This setting is relevant only in OOK	
	{0x2E,	0x2A},//	N/A	This setting is relevant only in OOK	
					
	{0x30,	0xAD},//	S2 B0AD		
	{0x32,	0x0C},//	S2 B20C	This is the Default Value after RESET	
	{0x33,	0x0A},//	S2 B30A		
	{0x34,	0x08},//	S2 B408	This is the Default Value after RESET	
	{0x35,	0x2A},//	S2 B52A	This is the Default Value after RESET	Relevant for RX settings Only
	{0x36,	0x2D},//	S2 B62D	This is the Default Value after RESET	
	{0x37,	0xD4},//	S2 B7D4	This is the Default Value after RESET	
	{0x38,	0x00},//	S2 B800	This is the Default Value after RESET	
	{0x39,	0x00},//	S2 B900	This is the Default Value after RESET	
	{0x3A,	0x00},//	S2 BA00	This is the Default Value after RESET	Relevant for TX settings Only
	{0x3B,	0x00},//	S2 BB00	This is the Default Value after RESET	Relevant for TX settings Only
	{0x3C,	0x00},//	S2 BC00	This is the Default Value after RESET	Relevant for TX settings Only
	{0x3D,	0x00},//	S2 BD00	This is the Default Value after RESET	Relevant for TX settings Only
	{0x3E,	0x01},//	S2 BE01		
	{0x3F,	0x00},//	S2 BF00	This is the Default Value after RESET	Relevant for RX settings Only
	{0x40,	0x00},//	S2 C000	This is the Default Value after RESET	Relevant for RX settings Only
	{0x41,	0x00},//	S2 C100	This is the Default Value after RESET	Relevant for RX settings Only
	{0x42,	0x00},//	S2 C200	This is the Default Value after RESET	Relevant for RX settings Only
	{0x43,	0xFF},//	S2 C3FF	This is the Default Value after RESET	Relevant for RX settings Only
	{0x44,	0xFF},//	S2 C4FF	This is the Default Value after RESET	Relevant for RX settings Only
	{0x45,	0xFF},//	S2 C5FF	This is the Default Value after RESET	Relevant for RX settings Only
	{0x46,	0xFF},//	S2 C6FF	This is the Default Value after RESET	Relevant for RX settings Only
					
	{0x58,	0x80},//	S2 D880		
	{0x69,	0x60},//	S2 E960		
	{0x6E,	0x4E},//	S2 EE4E		TX DATA RATE
	{0x6F,	0xA5},//	S2 EFA5		
					
	{0x70,	0x2C},//	S2 F02C		
	{0x71,	0x23},//	S2 F123		
	{0x72,	0x48},//	S2 F248		TX Frequency Deviation 
					
	{0x75,	0x57},//	S2 F557		
	{0x76,	0x00}//	S2 F600		Carrier Frequency
};


#endif




