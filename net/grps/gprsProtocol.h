#ifndef _GPRSPROTOCOL_H
#define _GPRSPROTOCOL_H

#define GPRS_F_HEAD			0x7e
#define GPRS_F_END			0x7f

#define GPRS_F_SYN_CMD		0x0e
#define GPRS_F_MAC_LEN		0x04
#define GPRS_F_CRC_END_LEN	0x03

#define GPRS_F_CMD_HEART	0x01			//heart packet  dev cmd
#define GPRS_F_CMD_WARN		0x02			//warn packet   dev cmd
#define GPRS_F_CMD_ACK		0x99            //ack packet the same between server and dev
#define GPRS_F_CMD_DATA_SYNC	0x03        //fire mac sync server cmd
#define GPRS_F_CMD_WARN_ACK	0x05            //fire warn server cmd
#define GPRS_F_CMD_TRAN		0x04			//dev cmd
#define GPRS_F_CMD_REQUST_SYNC 0x06         //dev request fire mac sync dev cmd

#define GPRS_TRAN	0xef
#define GPRS_DSC	0xea
#define GPRS_SRC	0x1a
#define GPRS_EB		0xeb
#define GPRS_DSC1B	0x1b

/*
	GPRS_F_COMMON_L
	---------------
	u_char ubHead;							1byte
	u_char ubSyn;							1byte
	u_char ubCmd;							1byte
	u_char ubSeqL;							1byte
	u_char ubSeqH;							1byte
	u_char ubDataLenL;						1byte
	u_char ubDataLenH;						1byte
	u_char ubaRouterMac[GRRS_MAC_LEN];		4bytes
	------------------
*/
#define GPRS_F_COMMON_L		11	
/*
	u_char ubHead;							1byte
	u_char ubSyn;							1byte
	u_char ubCmd;							1byte
	u_char ubSeqL;							1byte
	u_char ubSeqH;							1byte
	u_char ubDataLenL;						1byte
	u_char ubDataLenH;						1byte
*/


#define GPRS_F_FIX_LEN		10		//CRC 2 + 7 bytes + 1 end

/*
	crc scope, include data, not include frame crc and frame end
	u_char ubSyn;
	u_char ubCmd;
	u_char ubSeqL;
	u_char ubSeqH;
	u_char ubDataLenL;
	u_char ubDataLenH;
	u_char ubaMac[GPRS_F_MAC_LEN];
	u_char ubaData[]; 
	
*/
typedef struct _gprs_protocol
{
	u_char ubHead;
	u_char ubSyn;
	u_char ubCmd;
	u_char ubSeqL;
	u_char ubSeqH;
	u_char ubDataLenL;
	u_char ubDataLenH;
	/*--mac is in data scope--*/
	u_char ubaMac[GPRS_F_MAC_LEN];
	u_char ubaData[]; /*data + 2 byte crc + frame end*/
}GPRS_PROTOCOL;


typedef struct _gprs_warn_phone
{
	u_char ubaPhoneA[20];
	u_char ubaPhoneB[20];
}GPRS_WARN_PHONE;



extern int gprsProtocolFrameFill(u_char *pioBuf, u_char ubCmd, u_short uwSeq, const u_char *pcMAC, const u_char *pcData, u_short uwdataL);
#endif

