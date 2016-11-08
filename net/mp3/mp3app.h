#ifndef _MP3APP_H
#define _MP3APP_H


#define MP3_HEAD		0x7e
#define MP3_END			0xef


/*
Frame format
BY1680-12P���ñ�׼UART�첽���ڽӿڣ�Ϊ3.3V TTL��ƽ�ӿڡ���ͨ��MAX3232оƬת����RS232��ƽ����ͨ��USBתTTLģ����PCͨѶ���е��ԡ�
ͨѶ���ݸ�ʽ�ǣ���ʼλ��1λ������λ��8λ����żλ���ޣ�ֹͣλ��1λ��ʹ�õ��Դ��ڵ������֣���Ҫ��ȷ���ô��ڵĲ�����
Э�������ʽ��
��ʼ��  ����  ������  ����  У���� ������
0X7E   ������ ������  ������ ������ 0XEF
ע�⣺ ����ȫ��Ϊʮ����������
�����ȡ���ָ������+������+����(��Щû�в�������Щ����λ����)+У����ĸ�����
��У���롱��ָ������<���>������<���>������ֵ���Ȱ�˳��ֱ�����ֵ��

У����ͻ���ͨ������������õ������磬��������ָ��Ϊ  7E 04 3119 2C EF
     ����04�������õ������ǡ�04������31������19������2C��4������
У����2C�������õ���
���ȴ򿪼�����ѡ�����Աģʽ��
Ȼ��ѡ��16���ơ�˫�֣�
��������м���  04 Xor 31 Xor19 = 2C
*/

/*
7.1 ָ���б�
ͨ�ſ���ָ�ָ��ͳɹ�����OK������������ֹͣ����STOP����
*/
//CMD���							��Ӧ����				����(ASCK��)
#define MP3_CMD_PLAY		0x11		//����						��
#define MP3_CMD_PAUSE		0x12		//��ͣ						��
#define MP3_CMD_NEXT		0x13		//��һ��
#define MP3_CMD_PREV		0x14		//��һ��
#define MP3_CMD_V_ADD		0x15		//������
#define MP3_CMD_V_SUB		0x16		//������
#define MP3_CMD_RST			0x19		//��λ
#define MP3_CMD_SPEED		0x1a		//���
#define MP3_CMD_NSPEED		0x1b		//����
#define MP3_CMD_PLAY_PAUSE	0x1c	//����/��ͣ
#define MP3_CMD_STOP		0x1e		//ֹͣ

//CMD���							��Ӧ����				����(8λHEX)
#define MP3_CMD_SET_V			0x31		//��������				0-30���ɵ�(�����������)
#define MP3_CMD_SET_EQ			0x32		//����EQ				0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS)
#define MP3_CMD_SET_LOOP_MODE	0x33	//����ѭ��ģʽ		0-4(ȫ��/�ļ���/����/���/��ѭ��
#define MP3_CMD_CHANGE_DIR		0x34	//�ļ����л�		0����һ�ļ��У���1(��һ�ļ���) ��flash��û�д˹���
#define MP3_CMD_CHANGE_DEV		0x35	//�豸�л�			0��U�̣���1��SD��
#define MP3_CMD_ADKEY_UP		0x36	//ADKEY�������		1��������10K���裩��0��������Ĭ��0
#define MP3_CMD_ADKEY_EN		0x37	//ADKEYʹ��			1����0�رգ�Ĭ��1
#define MP3_CMD_BUSY_LEVL		0x38	//BUSY��ƽ�л�  1Ϊ��������ߵ�ƽ��0Ϊ��������͵�ƽ��Ĭ��1

//CMD���					����(16hex)��16λ��
#define MP3_CMD_SELECT			0x41	//ѡ�񲥷���Ŀ		1-�����Ŀ
#define MP3_CMD_SELECT_FILE		0x42	//ָ���ļ�����Ŀ����	�߰�λΪ�ļ��к�(00-99)���Ͱ�λΪ��������(001-255)��flash��û�д˹���
#define MP3_CMD_INSERT_PLAY		0x43	//�岥����			1-�����Ŀ
#define MP3_CMD_INSERT_PLAY_FILE	0x44	//�岥ָ���ļ�������ĸ�������	�岥ָ���ļ�������ĸ�������



//ͨ�Ų�ѯ����
//	ע���������������������֮������20MS���ϣ���ϲ��Ź�������������6MS���ڡ�
//CMD���					���ز���(ASCK��)��16λ��
#define MP3_CMD_PLAY_STATE		0x20		//��ѯ����״̬			0(ֹͣ)1(����) 2(��ͣ) 3(���)4(����)
#define MP3_CMD_V_STATE			0x21		//��ѯ������С			0-30(�������)
#define MP3_CMD_EQ_STATE		0x22		//��ѯ��ǰ EQ			0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS
#define	MP3_CMD_CUR_PLAY_MODE	0x23		//��ѯ��ǰ����ģʽ		0-4(ȫ��/�ļ���/����/���/��ѭ��)
#define	MP3_CMD_CUR_VERSION		0x24		//��ѯ�汾��			1.0
#define MP3_CMD_FLASH_FILES		0x25		//��ѯFLASH�����ļ���	1-255
#define MP3_CMD_U_FILES			0x2a		//��ѯU�̵ĵ�ǰ��Ŀ		1-65536
#define MP3_CMD_PLAY_TIME		0x2c		//��ѯ��ǰ���Ÿ�����ʱ��	����ʱ�䣨�룩
#define MP3_CMD_PLAY_TOTAL_TIME	0x2d		//��ѯ��ǰ���Ÿ�����ʱ��	����ʱ�䣨�룩
#define MP3_CMD_PLAY_NAME		0x2e		//��ѯ��ǰ���Ÿ�������		���ظ�������ֻ�ܷ���ǰ��λ������flash��û�д˹���
#define MP3_CMD_CUR_PLAY_NUMS	0x2f		//��ѯ��ǰ�����ļ�����������	0-65536��flash��û�д˹���



typedef struct _mp3_common_head
{
	u_char ubHead;
	u_char ubLen;		//ublen 1 + opt code 1 + params L + check code 1
	u_char ubOPT;		//cmd
}MP3_COMMON_HEAD;

typedef struct _mp3_con_no_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_NO_PARAM;

//b one byte param
typedef struct _mp3_con_b_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubParam;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_B_PARAM;


//w two bytes param
typedef struct _mp3_con_w_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubParamH;
	u_char ubParamL;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_W_PARAM;

#endif
