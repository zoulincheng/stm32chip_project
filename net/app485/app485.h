#ifndef _APP_485_H
#define _APP_485_H


#define TB_FRAME_HEADER_LEN			4
#define TB_SOH_FIX_LEN				6
#define TB_SAKED_FIX_LEN			4
#define TB_BSCOH_FIX_LEN			4
#define TB_ACK_FIX_LEN				4
#define TB_COMMON_FIX_LEN			3

/*
485 �� 232 ��������·����Э����ͬ������ʼλ��������У��λ��ֹͣλ��ɡ���������Ϊ����ʼ
λ 1 λ������λ 8 λ��У��λ 1 λżУ�飬ֹͣλ 1 λ�������� 9600��
*/
#define TB_SYN		0xaa			//ͬ���ַ������ֽ������������������ϵ�ͬ���ַ���SYN��ʱ����Ϊ��һ�����ݰ���ʼ
#define TB_EOT		0xaf			//�����ַ������ֽ����г��ֽ����ַ���EOT��ʱ����Ϊ�����ݰ����ݽ�����־��
#define TB_DLE		0xa0
/*
ת���ַ������ֽ��������г���ͬ���ַ���SYN��, �����ַ���EOT����ת���ַ���DLE��
��Ҫ����ʱ����Ҫ�ڸ��ַ�ǰ����ת���ַ���DLE��������ʱ������ת���ַ������������ַ���
��ͨ���ݴ���
*/
#define TB_DLE		0xa0	


/*
���Ӳ㴫��Э�������ѭ����ͨѶ���ƣ�
1. ���ڷ��ͷ���ÿһ�����ݰ������շ����ݽ��յ����ݱ������ͷ�������Ӧ���ݰ�������ȷ�ϻ�
���շ�ȷ�ϣ���
2. ���ڶ�����ݴ��䣬�����ݷ�����Ч���Ŵ� 1 ��ʼ�����ݰ���Ϊ 0 �� ACK ������ʵ�ʶ�����ݵ�
���ʹ�����̡����� PKG_NO �� SOH �����ݣ����շ���Ӧ PKG_NO �� ACK�����ͷ��յ� PKG_NO
�� ACK�����ͷ����� PKG_NO+1 �Ű����ݡ�
3. �����շ���⵽���ݳ���ʱ�����ͷ����ͷ�ȷ�� NAK ���ݰ���
4. ���ͷ�������һ�����ݰ����� 20ms ��û���յ����շ���ȷ�ϻ��ȷ���źţ��ط������ݰ�����
������Ϊ 3��
5. Ϊ���������ݷ��͹��̣����ͷ����ȷ��� ENQ ���ݰ����ȴ����շ������� PGK_NO=0 �� ACK ȷ
�ϰ������ͷ����յ� PKG_NO=0 �� ACK ȷ�ϰ��󣬽��� SOH ���ݰ�����״̬��
*/

/*
485 ��ͨѶ������֮�֣���֮��Ӧ��Э��Ҳ������֮�֡��� 5-1 Ϊ���Ӳ����������ַ��Լ���Ӧ��ͨ
Ѷ���ʽ��
�� 5-1
ͨѶ�庬�� �����ַ� ͨѶ���ʽ
���� 0xE0 (SOH) SOH SRCADDR DESTADDR PKG_NO LEN TYPE DATA
�㲥���� 0xBB (BCSOH) BCSOH SRCADDR =0 DESTADDR =0 CMD_NO DATA
�˳� 0xE1 (EXT) EXT SRCADDR DESTADDR
����ȷ�� 0xE2 (ACK) ACK SRCADDR DESTADDR PKG_NO
���շ�ȷ�� 0xE4 (NAK) NAK SRCADDR DESTADDR
���� 0xE8 (NUL) NUL SRCADDR DESTADDR
����ȷ�� 0xE9 (NULACK) NULACK SRCADDR DESTADDR
��ѯ 0xE7 (ENQ) ENQ SRCADDR DESTADDR
���� 0xD0 (SAK) SAK SRCADDR DESTADDR
����ȷ�� 0xDF (SAKED) SAKED SRCADDR DESTADDR TYPE
���� 0xF1 (LINK) LINK SRCADDR DESTADDR
����ȷ�� 0xF4 (LINKED) LINKED SRCADDR DESTADDR
��ֹ���� 0xF2 (UNLINK) UNLINK SRCADDR DESTADDR
��ֹ����ȷ�� 0xF8 (UNLINKED) UNLINKED SRCADDR DESTADDR
*/
/*
xor check sum 
exapmple
		 H	DATA	 END XOR
AA AA AA AA D0 00 1E AF CE
*/

#define TB_SOH		0xe0			//����
#define TB_BCSOH	0xbb			//�㲥����
#define TB_EXT		0xe1			//�˳�
#define TB_ACK		0xe2			//����ȷ��
#define TB_NAK		0xe4			//���շ�ȷ��
#define TB_NUL		0xe8			//����
#define TB_NULACK	0xe9			//����ȷ��
#define TB_ENQ		0xe7			//��ѯ
#define TB_SAK		0xd0			//����
#define TB_SAKED	0xdf			//����ȷ��
#define TB_LINK		0xf1			//����
#define TB_LINKED	0xf4			//����ȷ��
#define TB_UNLINK	0xf2			//��ֹ����
#define TB_UNLINKED	0xf8			//��ֹ����ȷ��

#define TB_WARN_FIRST_SEQ		0x01	//��һ�������������


#define TB_BSCOH_RSG		0x00
#define TB_BSCOH_SILENCE	0x01
#define TB_BSCOH_TIMESYNC	0x0e


#define TB_SOH_DATA_TYPE_FIRE			0x20
#define TB_SOH_DATA_TYPE_WRONG			0x40
#define TB_SOH_DATA_TYPE_MODULE_START	0x24

#define M_T_FIRE		0x02
#define M_T_WRONG		0x05
#define M_T_START		0x04

#define S_T_HASFIRE				0x01	//��<����>����		�����ͣ� 0x02�� CMD 0x20
#define S_T_NOFIRE				0x81	//��<����>��ʧ��		�����ͣ� 0x02��	CMD 0x20

#define S_T_DIS					0x01	//��ʧ					�����ͣ� 0x05�� CMD 0x40
#define S_T_NDIS				0x81	//��ʧ��ʧ				�����ͣ� 0x05��	CMD 0x40
#define S_T_M_OPEN				0x02	//ģ�鸺�ؿ�·			�����ͣ� 0x05��	CMD 0x40
#define S_T_M_NOPEN				0x82	//ģ�鸺�ؿ�·��ʧ		�����ͣ� 0x05��	CMD 0x40
#define S_T_M_SHORT				0x04	//ģ�鸺�ض�·			�����ͣ� 0x05��	CMD 0x40
#define S_T_M_NSHORT			0x84	//ģ�鸺�ض�·��ʧ		�����ͣ� 0x05��	CMD 0x40
#define S_T_KZJ_LD_WRONG		0x05	//KZJ_LD �豸����		�����ͣ� 0x05��	CMD 0x40
#define S_T_KZJ_LD_OK			0x85	// KZJ_LD �豸������ʧ	�����ͣ� 0x05��	CMD 0x40
#define S_T_MAKE_MATCH_WRONG	0x06	//�������Ͳ�ƥ��		�����ͣ� 0x05��	CMD 0x40
#define S_T_MAKE_MATCH_OK		0x86	//�������Ͳ�ƥ����ʧ	�����ͣ� 0x05��	CMD 0x40
#define S_T_IOB_SHORT			0x10	//�ӿڰ��·��·		�����ͣ� 0x05��	CMD 0x40
#define S_T_IOB_NSHORT			0x90	//�ӿڰ��·��·��ʧ	�����ͣ� 0x05��	CMD 0x40
#define S_T_IOB_NCON			0x50	//�ӿڰ�ͨѶ����		�����ͣ� 0x05��	CMD 0x40
#define S_T_IOB_CON				0xd0	//�ӿڰ�ͨѶ������ʧ	�����ͣ� 0x05��	CMD 0x40
#define S_T_HOST_NCON			0x51	//����ͨѶ����		�����ͣ� 0x05��		CMD 0x40
#define S_T_HOST_CON			0xd1	//����ͨѶ������ʧ	�����ͣ� 0x05��		CMD 0x40
#define S_T_DISP_WRONG			0x52	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40
#define S_T_DISP_OK				0xd2	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40
#define S_T_PRINT_WRONG			0x53	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40
#define S_T_PRINT_OK			0xd3	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40
#define S_T_POWER_WRONG			0x54	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40
#define S_T_POWER_OK			0xd4	//��ʾ��ͨѶ����	�����ͣ� 0x05��		CMD 0x40

#define S_T_M_START				0x01	//ģ����������		�����ͣ� 0x04��		CMD 0x24
#define S_T_M_STOP				0x81	//ģ����������		�����ͣ� 0x04��		CMD 0x24



#define TB_SOH_MASTER_NUM				0x01
#define TB_SOH_INTERFACE_NUM			0x00
#define TB_SOH_DEV_ADDR					


#define TB_MAX_FRAME_LEN	64

typedef struct _app_485_data
{
	u_char ubLen;
	u_char ubaData[127];
}APP_485_DATA;


//̽������ģ�������Ϣ��Ч������ϸ����
//�𾯻�����Ϣ��Ч������ϸ����
//�� 6-15 ģ���������ֹͣ��Ϣ��Ч������ϸ����
//cmd 0x20  ubStruMasterType 0x02  ubStruSalveType 0x01, 0x81
//cmd 0x40  ubStruMasterType 0x05  ubStruSalveType 0x01, 0x81, 0x02, 0x82,0x04, 0x84, 0x05, 0x85, 0x06, 0x86
typedef struct _tb_soh_warn_data
{
	u_char ubLength;				//���� 71���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubHostNo;				//�����ţ��� 1 ��ʼ��
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubAddrNo[2];				//�豸��ַ�� �� ���ֽ�Ϊ��ַ�ĵ�λ�� ���ֽ�Ϊ��ַ�ĸ�λ��
	u_char ubMakeType;				//��������	
	u_char ubEquipmentType;			//�豸���ͣ�������������ϸ��������			//	
	u_char ubZoneNo;				//���ţ��� 0 ��ʼ 0-19��
	u_char ubBuildingNo;			//���ţ��� 0 ��ʼ 0-19��
	u_char ubFloorNo;				//��ţ� 0-199 Ϊ 1-200 �㣩�� 0xFF-0xF6 Ϊ-1 ��-10 �㣩
	u_char ubRoomNo;				//���ţ� 0-255��
	u_char ubaPlaceDesc[41];		//λ������������Ϊ 41���� 0 ��β���ַ���
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩
	u_char ubIsolateFlag;			//�����־�� 0��δ���룬 1 ���룩
	u_char ubaaucEqbDesc[11];		//�豸��������������Ϊ 11���� 0 ��β���ַ���
}TB_SOH_WARN_DATA;

//�ӿڰ��·ͨѶ������Ϣ��Ч������ϸ����
typedef struct _tb_soh_iob_loob_wrong_data
{
	u_char ubLength;				//���� 52���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubCanFaultType;			//CAN ��������
	u_char ubHostNo;				//�����ţ��� 1 ��ʼ��
	u_char ubIOB_LoopNo;			//ucIOB_LoopNO
	u_char ubaaucDesc[41];			//��������
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩
}TB_SOH_IOB_LOOP_WRONG_DATA;


//����ͨѶ������Ϣ��Ч������ϸ����
typedef struct _tb_soh_host_commu_wrong_data
{
	u_char ubLength;				//���� 51���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubCanFaultType;			//CAN ��������
	u_char ubHostNo;				//�����ţ��� 1 ��ʼ��
	u_char ubaaucDesc[41];			//��������
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩
}TB_SOH_HOST_COMMU_WRONG_DATA;


//��ʾ��ͨѶ������Ϣ��Ч������ϸ����
typedef struct _tb_soh_fsd_wrong_data
{
	u_char ubLength;				//���� 58���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubCanFaultType;			//CAN ��������
	u_char ubHostNo;				// �����ţ��� 1 ��ʼ�� ����Ϊ �������
	u_char ubFSDAddrNo;				//��ʾ�̺ţ��� 1 ��ʼ��

	u_char ubMakeType;				//��������	
	u_char ubEquipmentType;			//�豸���ͣ�������������ϸ��������			//	
	u_char ubZoneNo;				//���ţ��� 0 ��ʼ 0-19��
	u_char ubBuildingNo;			//���ţ��� 0 ��ʼ 0-19��
	u_char ubFloorNo;				//��ţ� 0-199 Ϊ 1-200 �㣩�� 0xFF-0xF6 Ϊ-1 ��-10 �㣩
	u_char ubRoomNo;				//���ţ� 0-255��
	u_char ubaPlaceDesc[41];		//λ������������Ϊ 41���� 0 ��β���ַ���
	
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩

}TB_SOH_FSD_WRONG_DATA;


#if 0
//�� 6-13 ��ӡ��������Ϣ��Ч������ϸ����
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//���� 52���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubPrintFaultType;		//��ӡ���������� 0xF8 //��ӡ������
	u_char ubHostNo;				// �����ţ��� 1 ��ʼ�� �
	u_char ubPrinterNo;				//��ʾ�̺ţ��� 1 ��ʼ��

	u_char ubaaucDesc[41];			//��������
	
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩

}TB_SOH_PRINT_WRONG_DATA;
#endif

//�� 6-13 ��Դ������Ϣ��Ч������ϸ����
typedef struct _tb_soh_print_wrong_data
{
	u_char ubLength;				//���� 52���ݣ�������datatype length
	u_char ubDataType;				//cmd
	u_char ubStruMasterType;		//������
	u_char ubStruSalveType;			//�����ͣ� 0x01:��<����>���� 0x81:��<����>��ʧ��
	u_char ubPowerFaultType;		//��Դ��������
	u_char ubHostNo;				// �����ţ��� 1 ��ʼ��
	u_char ubPowerNo;				//��ʾ�̺ţ��� 1 ��ʼ��

	u_char ubaaucDesc[41];			//��������
	
	u_char ubYear;					//�꣨��λ BCD �룩
	u_char ubMonth;					//�꣨��λ BCD �룩
	u_char ubDay;					//�գ���λ BCD �룩
	u_char ubHour;					//ʱ����λ BCD �룩
	u_char ubMinute;				//�֣���λ BCD �룩
	u_char ubSecond;				//�루��λ BCD �룩

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
