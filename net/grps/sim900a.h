#ifndef _SIM900A_H
#define _SIM900A_H

#define SIM900A_CHECK_AT		0x00		//��������ģ��AT
#define SIM900A_CLOSE_ECHO		0x01		//�رջ���
#define SIM900A_CHECK_SIM		0x02		//���sim���Ƿ���
#define SIM900A_TCPUDP			0x03		//tcp



#define SIM900A_INIT_ERROR			0x00
#define SIM900A_CHECK_AND_SIM		0x01
#define SIM900A_CFG_GPRS_PARAM		0x02
#define SIM900A_CFG_TCPUDP_PARAM	0x03
#define SIM900A_NOT_SURPORT_B		0x04		//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ�������
#define SIM900A_PDP_ERROR			0x05		//����PDP������,��������Э��,��������Ϣ����
#define SIM900A_GPRS_ERROR			0x06		//����GPRS����
#define SIM900A_GPRS_CONNECT_ERROR	0x07	//����ΪGPRS����ģʽ
#define SIM900A_IP_HEAD_ERROR		0x08		//���ý���������ʾIPͷ(�����ж�������Դ)
#define SIM900A_TCPUDP_CLOSE_T		0x09
#define SIM900A_TCPUDP_CONNECT		0x0a


#define SIM900A_DATA_TCP			0x01
#define SIM900A_DATA_RESP			0x02
#define SIM900A_DATA_NONE			0x04

#define SIM900A_RESP_STATUS			0x01
#define SIM900A_RESP_HEART			0x02

#define SIM900A_SEND_START			0x01
#define SIM900A_SEND_FINISH			0x02
#define SIM900A_SEND_NONE			0x03


#define NET_CONNECT_SIM900			0x01
#define NET_CONNECT_ETH				0x02
#define NET_CONNECT_NONE			0x00



enum GPRS_STATE {
  SIM900A_AT_OK = 0,		//gprs model ok
  SIM900A_SIM_OK = 1,		//sim card ok
  SIM900A_TCPUDP_OK = 2,	//tcpudp connect ok
  SIM900A_TCPUDP_CLOSE = 3,	//tcpudp close
  SIM900A_ERROR		= 4		//error, can not communicattion
};

#define SIMG900A_MAX_TCPUDP_DATA_LEN	1352

typedef struct _sim900a_msg
{
	int  nLen;									//���ݳ���
	u_char ubamsg[SIMG900A_MAX_TCPUDP_DATA_LEN];
	struct etimer *petimer;
	u_char ubsrc;								//������Դ
}SIM900A_MSG;


typedef struct _net_mode
{
	u_char netMode;
	u_long lastRxTime;
}NET_MODE;


void uart4_send_char(u_char ch);


#endif
