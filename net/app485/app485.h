#ifndef _APP_485_H
#define _APP_485_H


#define TB_FRAME_HEADER_LEN			4
#define TB_SOH_FIX_LEN				6
#define TB_SAKED_FIX_LEN			4
#define TB_BSCOH_FIX_LEN			4
#define TB_ACK_FIX_LEN				4
#define TB_COMMON_FIX_LEN			3

/*
485 ºÍ 232 µÄÎïÀíÁ´Â·´«ÊäĞ­ÒéÏàÍ¬£¬ÓÉÆğÊ¼Î»£¬Êı¾İÓò£¬Ğ£ÑéÎ»£¬Í£Ö¹Î»×é³É¡£¾ßÌåÃèÊöÎª£ºÆğÊ¼
Î» 1 Î»£¬Êı¾İÎ» 8 Î»£¬Ğ£ÑéÎ» 1 Î»Å¼Ğ£Ñé£¬Í£Ö¹Î» 1 Î»£¬²¨ÌØÂÊ 9600¡£
*/
#define TB_SYN		0xaa			//Í¬²½×Ö·û£¬µ±×Ö½ÚÁ÷³öÏÖÁ¬ĞøÁ½¸öÒÔÉÏµÄÍ¬²½×Ö·û£¨SYN£©Ê±£¬ÈÏÎªÊÇÒ»¸öÊı¾İ°ü¿ªÊ¼
#define TB_EOT		0xaf			//½áÊø×Ö·û£¬µ±×Ö½ÚÁ÷ÖĞ³öÏÖ½áÊø×Ö·û£¨EOT£©Ê±£¬ÈÏÎªÊÇÊı¾İ°üÊı¾İ½áÊø±êÖ¾¡£
#define TB_DLE		0xa0
/*
×ªÒâ×Ö·û£¬µ±×Ö½ÚÁ÷Êı¾İÖĞ³öÏÖÍ¬²½×Ö·û£¨SYN£©, ½áÊø×Ö·û£¨EOT£©£¬×ªÒâ×Ö·û£¨DLE£©
ĞèÒª´«ÊäÊ±£¬ĞèÒªÔÚ¸Ã×Ö·ûÇ°¼ÓÉÏ×ªÒâ×Ö·û£¨DLE£©£¬½ÓÊÕÊ±£¬ºöÂÔ×ªÒâ×Ö·û£¬ÆäºóµÄÌØÕ÷×Ö·û°´
ÆÕÍ¨Êı¾İ´¦Àí
*/
#define TB_DLE		0xa0	


/*
Á¬½Ó²ã´«ÊäĞ­Òé±ØĞë×ñÑ­ÒÔÏÂÍ¨Ñ¶»úÖÆ£º
1. ¶ÔÓÚ·¢ËÍ·½µÄÃ¿Ò»¸öÊı¾İ°ü£¬½ÓÊÕ·½¸ù¾İ½ÓÊÕµÄÊı¾İ±ØĞëÏò·¢ËÍ·½·¢ËÍÏìÓ¦Êı¾İ°ü£¨½ÓÊÕÈ·ÈÏ»ò
½ÓÊÕ·ÇÈ·ÈÏ£©¡£
2. ¶ÔÓÚ¶à°üÊı¾İ´«Êä£¬ÆäÊı¾İ·¢ËÍÓĞĞ§°üºÅ´Ó 1 ¿ªÊ¼¡£Êı¾İ°üºÅÎª 0 µÄ ACK ½«´¥·¢Êµ¼Ê¶à°üÊı¾İµÄ
·¢ËÍ´«Êä¹ı³Ì¡£·¢ËÍ PKG_NO ºÅ SOH °üÊı¾İ£¬½ÓÊÕ·½ÏìÓ¦ PKG_NO ºÅ ACK£¬·¢ËÍ·½ÊÕµ½ PKG_NO
ºÅ ACK£¬·¢ËÍ·½·¢ËÍ PKG_NO+1 ºÅ°üÊı¾İ¡£
3. µ±½ÓÊÕ·½¼ì²âµ½Êı¾İ³ö´íÊ±£¬Ïò·¢ËÍ·½·¢ËÍ·ÇÈ·ÈÏ NAK Êı¾İ°ü¡£
4. ·¢ËÍ·½·¢ËÍÍêÒ»¸öÊı¾İ°üºó£¬ÔÚ 20ms ºóÃ»ÓĞÊÕµ½½ÓÊÕ·½µÄÈ·ÈÏ»ò·ÇÈ·ÈÏĞÅºÅ£¬ÖØ·¢´ËÊı¾İ°ü£¬ÖØ
·¢´ÎÊıÎª 3¡£
5. ÎªÁËÆô¶¯Êı¾İ·¢ËÍ¹ı³Ì£¬·¢ËÍ·½Ê×ÏÈ·¢ËÍ ENQ Êı¾İ°ü£¬µÈ´ı½ÓÊÕ·¢·½·¢ËÍ PGK_NO=0 µÄ ACK È·
ÈÏ°ü£¬·¢ËÍ·½ÔÚÊÕµ½ PKG_NO=0 µÄ ACK È·ÈÏ°üºó£¬½øÈë SOH Êı¾İ°ü·¢ËÍ×´Ì¬¡£
*/

/*
485 µÄÍ¨Ñ¶ÓĞÖ÷´ÓÖ®·Ö£¬ÓëÖ®¶ÔÓ¦µÄĞ­ÒéÒ²ÓĞÖ÷´ÓÖ®·Ö¡£±í 5-1 ÎªÁ¬½Ó²ãËùÓÃÌØÕ÷×Ö·ûÒÔ¼°ÏàÓ¦µÄÍ¨
Ñ¶èå¸ñÊ½¡£
±í 5-1
Í¨Ñ¶èåº¬Òå ÌØÕ÷×Ö·û Í¨Ñ¶èå¸ñÊ½
Êı¾İ 0xE0 (SOH) SOH SRCADDR DESTADDR PKG_NO LEN TYPE DATA
¹ã²¥Êı¾İ 0xBB (BCSOH) BCSOH SRCADDR =0 DESTADDR =0 CMD_NO DATA
ÍË³ö 0xE1 (EXT) EXT SRCADDR DESTADDR
½ÓÊÕÈ·ÈÏ 0xE2 (ACK) ACK SRCADDR DESTADDR PKG_NO
½ÓÊÕ·ÇÈ·ÈÏ 0xE4 (NAK) NAK SRCADDR DESTADDR
½áÊø 0xE8 (NUL) NUL SRCADDR DESTADDR
½áÊøÈ·ÈÏ 0xE9 (NULACK) NULACK SRCADDR DESTADDR
²éÑ¯ 0xE7 (ENQ) ENQ SRCADDR DESTADDR
ÎÕÊÖ 0xD0 (SAK) SAK SRCADDR DESTADDR
ÎÕÊÖÈ·ÈÏ 0xDF (SAKED) SAKED SRCADDR DESTADDR TYPE
Á¬½Ó 0xF1 (LINK) LINK SRCADDR DESTADDR
Á¬½ÓÈ·ÈÏ 0xF4 (LINKED) LINKED SRCADDR DESTADDR
ÖÕÖ¹Á¬½Ó 0xF2 (UNLINK) UNLINK SRCADDR DESTADDR
ÖÕÖ¹Á¬½ÓÈ·ÈÏ 0xF8 (UNLINKED) UNLINKED SRCADDR DESTADDR
*/
/*
xor check sum 
exapmple
		 H	DATA	 END XOR
AA AA AA AA D0 00 1E AF CE
*/

#define TB_SOH		0xe0			//Êı¾İ
#define TB_BCSOH	0xbb			//¹ã²¥Êı¾İ
#define TB_EXT		0xe1			//ÍË³ö
#define TB_ACK		0xe2			//½ÓÊÕÈ·ÈÏ
#define TB_NAK		0xe4			//½ÓÊÕ·ÇÈ·ÈÏ
#define TB_NUL		0xe8			//½áÊø
#define TB_NULACK	0xe9			//½áÊøÈ·ÈÏ
#define TB_ENQ		0xe7			//²éÑ¯
#define TB_SAK		0xd0			//ÎÕÊÖ
#define TB_SAKED	0xdf			//ÎÕÊÖÈ·ÈÏ
#define TB_LINK		0xf1			//Á¬½Ó
#define TB_LINKED	0xf4			//Á¬½ÓÈ·ÈÏ
#define TB_UNLINK	0xf2			//ÖÕÖ¹Á¬½Ó
#define TB_UNLINKED	0xf8			//ÖÕÖ¹Á¬½ÓÈ·ÈÏ

#define TB_WARN_FIRST_SEQ		0x01	//µÚÒ»°ü±¨¾¯Êı¾İĞòºÅ


#define TB_BSCOH_RSG		0x00
#define TB_BSCOH_SILENCE	0x01
#define TB_BSCOH_TIMESYNC	0x0e


#define TB_SOH_DATA_TYPE_FIRE			0x20
#define TB_SOH_DATA_TYPE_WRONG			0x40
#define TB_SOH_DATA_TYPE_MODULE_START	0x24

#define M_T_FIRE		0x02
#define M_T_WRONG		0x05
#define M_T_START		0x04

#define S_T_HASFIRE				0x01	//»ğ¾¯<·´À¡>·¢Éú		Ö÷ÀàĞÍ£¨ 0x02£© CMD 0x20
#define S_T_NOFIRE				0x81	//»ğ¾¯<·´À¡>ÏûÊ§£©		Ö÷ÀàĞÍ£¨ 0x02£©	CMD 0x20

#define S_T_DIS					0x01	//¶ªÊ§					Ö÷ÀàĞÍ£¨ 0x05£© CMD 0x40
#define S_T_NDIS				0x81	//¶ªÊ§ÏûÊ§				Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_M_OPEN				0x02	//Ä£¿é¸ºÔØ¿ªÂ·			Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_M_NOPEN				0x82	//Ä£¿é¸ºÔØ¿ªÂ·ÏûÊ§		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_M_SHORT				0x04	//Ä£¿é¸ºÔØ¶ÌÂ·			Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_M_NSHORT			0x84	//Ä£¿é¸ºÔØ¶ÌÂ·ÏûÊ§		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_KZJ_LD_WRONG		0x05	//KZJ_LD Éè±¸¹ÊÕÏ		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_KZJ_LD_OK			0x85	// KZJ_LD Éè±¸¹ÊÕÏÏûÊ§	Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_MAKE_MATCH_WRONG	0x06	//Éú²úÀàĞÍ²»Æ¥Åä		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_MAKE_MATCH_OK		0x86	//Éú²úÀàĞÍ²»Æ¥ÅäÏûÊ§	Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_IOB_SHORT			0x10	//½Ó¿Ú°å»ØÂ·¶ÌÂ·		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_IOB_NSHORT			0x90	//½Ó¿Ú°å»ØÂ·¶ÌÂ·ÏûÊ§	Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_IOB_NCON			0x50	//½Ó¿Ú°åÍ¨Ñ¶¹ÊÕÏ		Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_IOB_CON				0xd0	//½Ó¿Ú°åÍ¨Ñ¶¹ÊÕÏÏûÊ§	Ö÷ÀàĞÍ£¨ 0x05£©	CMD 0x40
#define S_T_HOST_NCON			0x51	//Ö÷»úÍ¨Ñ¶¹ÊÕÏ		Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_HOST_CON			0xd1	//Ö÷»úÍ¨Ñ¶¹ÊÕÏÏûÊ§	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_DISP_WRONG			0x52	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_DISP_OK				0xd2	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_PRINT_WRONG			0x53	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_PRINT_OK			0xd3	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_POWER_WRONG			0x54	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40
#define S_T_POWER_OK			0xd4	//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏ	Ö÷ÀàĞÍ£¨ 0x05£©		CMD 0x40

#define S_T_M_START				0x01	//Ä£¿éÆô¶¯ÇëÇó		Ö÷ÀàĞÍ£¨ 0x04£©		CMD 0x24
#define S_T_M_STOP				0x81	//Ä£¿éÆô¶¯ÇëÇó		Ö÷ÀàĞÍ£¨ 0x04£©		CMD 0x24



#define TB_SOH_MASTER_NUM				0x01
#define TB_SOH_INTERFACE_NUM			0x00
#define TB_SOH_DEV_ADDR					


#define TB_MAX_FRAME_LEN	64

typedef struct _app_485_data
{
	u_char ubLen;
	u_char ubaData[127];
}APP_485_DATA;


//Ì½²âÆ÷»òÄ£¿é¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
//»ğ¾¯»ò·´À¡ĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
//±í 6-15 Ä£¿éµÄÆô¶¯»òÍ£Ö¹ĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
//cmd 0x20  ubStruMasterType 0x02  ubStruSalveType 0x01, 0x81
//cmd 0x40  ubStruMasterType 0x05  ubStruSalveType 0x01, 0x81, 0x02, 0x82,0x04, 0x84, 0x05, 0x85, 0x06, 0x86
typedef struct _tb_soh_warn_data
{
	u_char ubLength;				//³¤¶È 71Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubHostNo;				//Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£©
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubAddrNo[2];				//Éè±¸µØÖ·ºÅ £¨ ¸ß×Ö½ÚÎªµØÖ·µÄµÍÎ»£¬ µÍ×Ö½ÚÎªµØÖ·µÄ¸ßÎ»£©
	u_char ubMakeType;				//Éú²úÀàĞÍ	
	u_char ubEquipmentType;			//Éè±¸ÀàĞÍ£¨ÓÃÓÚÁª¶¯£¬ÏêÏ¸ÃèÊö¼ûºó£©			//	
	u_char ubZoneNo;				//ÇøºÅ£¨´Ó 0 ¿ªÊ¼ 0-19£©
	u_char ubBuildingNo;			//¶°ºÅ£¨´Ó 0 ¿ªÊ¼ 0-19£©
	u_char ubFloorNo;				//²ãºÅ£¨ 0-199 Îª 1-200 ²ã£©£¨ 0xFF-0xF6 Îª-1 ÖÁ-10 ²ã£©
	u_char ubRoomNo;				//·¿ºÅ£¨ 0-255£©
	u_char ubaPlaceDesc[41];		//Î»ÖÃÃèÊö£¬³¤¶ÈÎª 41£¬ÒÔ 0 ½áÎ²µÄ×Ö·û´®
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©
	u_char ubIsolateFlag;			//¸ôÀë±êÖ¾£¨ 0£ºÎ´¸ôÀë£¬ 1 ¸ôÀë£©
	u_char ubaaucEqbDesc[11];		//Éè±¸ÀàĞÍÃèÊö£¬³¤¶ÈÎª 11£¬ÒÔ 0 ½áÎ²µÄ×Ö·û´®
}TB_SOH_WARN_DATA;

//½Ó¿Ú°å»ØÂ·Í¨Ñ¶¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
typedef struct _tb_soh_iob_loob_wrong_data
{
	u_char ubLength;				//³¤¶È 52Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubCanFaultType;			//CAN ¹ÊÕÏÀàĞÍ
	u_char ubHostNo;				//Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£©
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubaaucDesc[41];			//¹ÊÕÏÃèÊö
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©
}TB_SOH_IOB_LOOP_WRONG_DATA;


//Ö÷»úÍ¨Ñ¶¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
typedef struct _tb_soh_host_commu_wrong_data
{
	u_char ubLength;				//³¤¶È 51Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubCanFaultType;			//CAN ¹ÊÕÏÀàĞÍ
	u_char ubHostNo;				//Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£©
	u_char ubaaucDesc[41];			//¹ÊÕÏÃèÊö
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©
}TB_SOH_HOST_COMMU_WRONG_DATA;


//ÏÔÊ¾ÅÌÍ¨Ñ¶¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
typedef struct _tb_soh_fsd_wrong_data
{
	u_char ubLength;				//³¤¶È 58Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubCanFaultType;			//CAN ¹ÊÕÏÀàĞÍ
	u_char ubHostNo;				// Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£© ¸ü¸ÄÎª À©ÈİÏäºÅ
	u_char ubFSDAddrNo;				//ÏÔÊ¾ÅÌºÅ£¨´Ó 1 ¿ªÊ¼£©

	u_char ubMakeType;				//Éú²úÀàĞÍ	
	u_char ubEquipmentType;			//Éè±¸ÀàĞÍ£¨ÓÃÓÚÁª¶¯£¬ÏêÏ¸ÃèÊö¼ûºó£©			//	
	u_char ubZoneNo;				//ÇøºÅ£¨´Ó 0 ¿ªÊ¼ 0-19£©
	u_char ubBuildingNo;			//¶°ºÅ£¨´Ó 0 ¿ªÊ¼ 0-19£©
	u_char ubFloorNo;				//²ãºÅ£¨ 0-199 Îª 1-200 ²ã£©£¨ 0xFF-0xF6 Îª-1 ÖÁ-10 ²ã£©
	u_char ubRoomNo;				//·¿ºÅ£¨ 0-255£©
	u_char ubaPlaceDesc[41];		//Î»ÖÃÃèÊö£¬³¤¶ÈÎª 41£¬ÒÔ 0 ½áÎ²µÄ×Ö·û´®
	
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©

}TB_SOH_FSD_WRONG_DATA;


#if 0
//±í 6-13 ´òÓ¡»ú¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//³¤¶È 52Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubPrintFaultType;		//´òÓ¡»ú¹ÊÕÏÀàĞÍ 0xF8 //´òÓ¡»ú¹ÊÕÏ
	u_char ubHostNo;				// Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£© ¸
	u_char ubPrinterNo;				//ÏÔÊ¾ÅÌºÅ£¨´Ó 1 ¿ªÊ¼£©

	u_char ubaaucDesc[41];			//¹ÊÕÏÃèÊö
	
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©

}TB_SOH_PRINT_WRONG_DATA;
#endif

//±í 6-13 µçÔ´¹ÊÕÏĞÅÏ¢ÓĞĞ§Êı¾İÏêÏ¸ÃèÊö
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//³¤¶È 52Êı¾İ£¬²»°üº¬datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//Ö÷ÀàĞÍ
	u_char ubStruSalveType;			//´ÓÀàĞÍ£¨ 0x01:»ğ¾¯<·´À¡>·¢Éú 0x81:»ğ¾¯<·´À¡>ÏûÊ§£©
	u_char ubPowerFaultType;		//µçÔ´¹ÊÕÏÀàĞÍ
	u_char ubHostNo;				// Ö÷»úºÅ£¨´Ó 1 ¿ªÊ¼£©
	u_char ubPowerNo;				//ÏÔÊ¾ÅÌºÅ£¨´Ó 1 ¿ªÊ¼£©

	u_char ubaaucDesc[41];			//¹ÊÕÏÃèÊö
	
	u_char ubYear;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubMonth;					//Äê£¨Á½Î» BCD Âë£©
	u_char ubDay;					//ÈÕ£¨Á½Î» BCD Âë£©
	u_char ubHour;					//Ê±£¨Á½Î» BCD Âë£©
	u_char ubMinute;				//·Ö£¨Á½Î» BCD Âë£©
	u_char ubSecond;				//Ãë£¨Á½Î» BCD Âë£©

}TB_SOH_PRIONT_WRONG_DATA;



/*
erery frame has this struct
*/
typedef struct _tb_frame_common
{
	u_char ubCmd;
	u_char ubSrcAddr;
	u_char ubDestAddr;
}TB_FRAME_COMMON;


/*
data frame format
*/
typedef struct _tb_frame_soh_data
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNO;
	u_char ubLen;
	u_char ubType;
	u_char ubaData[];	//include frame end and check sum use ^(xor) method
}TB_FRAME_SOH_DATA;


typedef struct _tb_warn_msg
{
	u_char ubLen;
	u_char ubType;
	u_char ubaData[];
}TB_WARN_MSG;

typedef struct _tb_soh_data_common
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNO;
	u_char ubLen;
	u_char ubType;
	u_char ubStruMasterType;
	u_char ubStruSalveType;
}TB_SOH_DATA_COMMON;


typedef struct _tb_soh_msg_common
{
	u_char ubLen;
	u_char ubType;
	u_char ubStruMasterType;
	u_char ubStruSalveType;	
}TB_SOH_MSG_COMMON;

/*
broadcast data frame format
*/
typedef struct _tb_frame_bsc_data
{
	TB_FRAME_COMMON tbCommon;
	u_char ubCmdNo;
	u_char ubaData[];	//include frame end and check sum use ^(xor) method
}TB_FRAME_BSC_DATA;


/*
receive conform ack frame format
*/
typedef struct _tb_frame_ack
{
	TB_FRAME_COMMON tbCommon;
	u_char ubPkgNo;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_ACK;



/*
SAKED frame format
*/
typedef struct _tb_frame_saked
{
	TB_FRAME_COMMON tbCommon;
	u_char ubType;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_SAKED;


typedef struct _tb_frame_common_frame
{
	TB_FRAME_COMMON tbCommon;
	u_char ubEot;
	u_char ubXor;
}TB_FRAME_COMMON_FRAME;


#endif
