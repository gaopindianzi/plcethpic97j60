#include "TCPIP Stack/TCPIP.h"

#include "TCPIPConfig.h"
#include "DS1302.h"


#ifdef STACK_USE_DS1302


void delay1us(void)
{
	Nop();
	Nop();
	Nop();
}

//////////////////////////////////////////////////////////////////////
//Function void WriteDS1302( BYTE ds_addr, BYTE ds_data )
//Intput:
//			ds_addr���Ĵ�����ַ
//			ds_data: ��д�������
//
//Output:	NULL
//			
//Overview: ��DS1302�Ĵ���д����
void WriteDS1302( BYTE ds_addr, BYTE ds_data )
{
	BYTE i;
	RTC_SCL_IO = 0;
	RTC_RST_IO = 1;
	RTC_SDA_TRIS = 0;
	delay1us();
	for(i = 0; i < 8; i++)
	{
		RTC_SCL_IO = 0;
		RTC_SDA_OUT = ds_addr & 0x01;
		delay1us();
		RTC_SCL_IO = 1;
		ds_addr = ds_addr >> 1;
	}
	for(i = 0; i < 8; i++)
	{
		RTC_SCL_IO = 0;
		RTC_SDA_OUT = ds_data & 0x01;
		delay1us();	
		RTC_SCL_IO = 1;
		ds_data = ds_data >> 1;
	}
	RTC_SCL_IO = 0;
	delay1us();
	RTC_RST_IO = 0;
	RTC_SDA_TRIS = 1;
}

//////////////////////////////////////////////////////////////////////
//Function BYTE ReadDS1302( BYTE ds_addr )
//Intput:
//			ds_addr���Ĵ�����ַ
//
//Output:	�Ĵ�������
//			
//Overview: ��DS1302�Ĵ���������
BYTE ReadDS1302( BYTE ds_addr )
{
	BYTE i;
	BYTE_VAL DS1302DATA;
	RTC_SCL_IO = 0;
	RTC_RST_IO = 1;
	RTC_SDA_TRIS = 0;
	delay1us();
	for(i = 0; i < 8; i++)
	{
		RTC_SCL_IO = 0;
		RTC_SDA_OUT = ds_addr & 0x01;
		delay1us();
		RTC_SCL_IO = 1;
		ds_addr = ds_addr >> 1;
	}
	RTC_SDA_TRIS = 1;
	for(i = 0; i < 7; i++)
	{
		RTC_SCL_IO = 0;
		DS1302DATA.bits.b7 = RTC_SDA_IN;
		RTC_SCL_IO = 1;
		delay1us();
		DS1302DATA.Val = DS1302DATA.Val >> 1;
		RTC_SCL_IO = 0;
		DS1302DATA.bits.b7 = RTC_SDA_IN;
	}
	RTC_SCL_IO = 0;
	delay1us();
	RTC_RST_IO = 0;
	return DS1302DATA.Val;
}

//////////////////////////////////////////////////////////////////////
//Function void DS1302_Init(void)
//Intput:
//			NULL
//
//Output:	NULL
//			
//Overview: DS1302��ʼ��
void DS1302_Init(void)
{
	//BYTE i;
	RTC_RST_IO = 0;
	RTC_SCL_IO = 0;
	RTC_RST_TRIS = 0;
	RTC_SCL_TRIS = 0;
	RTC_SDA_TRIS = 1;
#if 0
	i = ReadDS1302(Add_RAM0);
	if(0) //i != 0x5A)
	{
		WriteDS1302( Add_CONTROL,0x00 );		//�ر�д����
		WriteDS1302( Add_MIN,0x08 );			//Ԥ�÷���ʱ��
		WriteDS1302( Add_HR,0x20 );				//Ԥ��Сʱ��24Сʱ��
		WriteDS1302( Add_DATE,0x08 );			//Ԥ������
		WriteDS1302( Add_MONTH,0x08 );			//Ԥ���·�
		WriteDS1302( Add_DAY,0x05 );			//Ԥ������
		WriteDS1302( Add_YEAR,0x08 );			//Ԥ�����
		WriteDS1302( Add_CHARGER,0x0a6 );		//д�����ƼĴ�����1010 0110��I=1mA
		WriteDS1302( Add_SEC,0x08 );			//����ʱ��
		WriteDS1302( Add_RAM0,0x5A);			//RAM0��Ԫд��0x55����ֹ�ٴγ�ʼ��
		WriteDS1302( Add_CONTROL,0x80 );		//��д���� 
	}
#endif
}

//////////////////////////////////////////////////////////////////////
//Function void ReadRTC(BYTE *buffer)
//Intput:
//			������ָ��
//
//Output:	NULL
//			
//Overview: ��DS1302���������ݣ��������ݱ��浽ָ���Ļ�����
//SEC MIN HR DATE MONTH DAY YEAR CONTROL	����BCD��
void ReadRTC(BYTE *buffer)
{
	BYTE i,j;
	BYTE ds_addr;
	BYTE_VAL DS1302DATA;
	ds_addr = Add_CLOCKBURST + 0x01;
	RTC_SCL_IO = 0;
	RTC_RST_IO = 1;
	RTC_SDA_TRIS = 0;
	delay1us();
	delay1us();
	for(i = 0; i < 8; i++)
	{
		RTC_SCL_IO = 0;
		RTC_SDA_OUT = ds_addr & 0x01;
		delay1us();
		RTC_SCL_IO = 1;
		ds_addr = ds_addr >> 1;
	}
	RTC_SDA_TRIS = 1;
	for(j = 0; j < 8; j++)
	{
		RTC_SCL_IO = 1;
		for(i = 0; i < 7; i++)
		{
			RTC_SCL_IO = 0;
			DS1302DATA.bits.b7 = RTC_SDA_IN;
			RTC_SCL_IO = 1;
			delay1us();
			DS1302DATA.Val = DS1302DATA.Val >> 1;
			RTC_SCL_IO = 0;
			DS1302DATA.bits.b7 = RTC_SDA_IN;
		}
		*buffer++ = DS1302DATA.Val;
		DS1302DATA.Val = 0;	
	}
	RTC_SCL_IO = 0;
	delay1us();
	RTC_RST_IO = 0;
}

//////////////////////////////////////////////////////////////////////
//Function void UpdataRTC(BYTE *buffer)
//Intput:
//			������ָ��
//
//Output:	NULL
//			
//Overview: �ѻ���������������д��DS1302������ʵʱʱ��
//SEC MIN HR DATE MONTH DAY YEAR CONTROL	����BCD��
void UpdataRTC(BYTE *buffer)
{
	BYTE i,j;
	BYTE ds_addr,ds_data;
	WriteDS1302( Add_CONTROL,0x00 );		//�ر�д���� 
	ds_addr = Add_CLOCKBURST;
	RTC_SCL_IO = 0;
	RTC_RST_IO = 1;
	RTC_SDA_TRIS = 0;
	delay1us();
	delay1us();
	for(i = 0; i < 8; i++)
	{
		RTC_SDA_OUT = ds_addr & 0x01;
		RTC_SCL_IO = 0;
		delay1us();
		RTC_SCL_IO = 1;
		ds_addr = ds_addr >> 1;
	}
	for(j = 0; j < 8; j++)
	{
		ds_data = *buffer++;
		for(i = 0; i < 8; i++)
		{
			RTC_SDA_OUT = ds_data & 0x01;
			RTC_SCL_IO = 0;
			delay1us();
			RTC_SCL_IO = 1;
			ds_data = ds_data >> 1;
		}
	}
	RTC_SCL_IO = 0;
	delay1us();
	RTC_RST_IO = 0;
	RTC_SDA_TRIS = 1;
//	WriteDS1302( Add_SEC,0x30 );			//����ʱ��
	WriteDS1302( Add_CONTROL,0x80 );		//��д����
}

//////////////////////////////////////////////////////////////////////
//Function void Hex2BCD(BYTE *data,BYTE amount)
//Intput:
//			*data�� ������ָ��
//			amount: ���ݸ���
//Output:	NULL
//			
//Overview: �ѻ�������ʮ����������ת��BCD��
void Hex2BCD(BYTE *data, BYTE amount)
{
	BYTE i,j,temp1,temp2;
	for(i = 0; i < amount; i++)
	{
		temp1 = *data / 10 ;
		temp2 = *data % 10;	
		*data++ = (temp1 << 4) | temp2;
	} 
}

//////////////////////////////////////////////////////////////////////
//Function void BCD2Hex(BYTE *data,BYTE amount)
//Intput:
//			*data�� ������ָ��
//			amount: ���ݸ���
//Output:	NULL
//			
//Overview: �ѻ�������BCD��ת��ʮ����������
void BCD2Hex(BYTE *data, BYTE amount)
{
	BYTE i,j,temp1,temp2;	
	for(i = 0; i < amount; i++)
	{
		temp1 = *data;
		temp2 = temp1 >> 4;
		temp2 = temp2 & 0x0f;
		*data++ = temp2 * 10 + (temp1 & 0x0f);
	}
}


#endif
