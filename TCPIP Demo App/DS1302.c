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
//			ds_addr：寄存器地址
//			ds_data: 待写入的数据
//
//Output:	NULL
//			
//Overview: 向DS1302寄存器写数据
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
//			ds_addr：寄存器地址
//
//Output:	寄存器数据
//			
//Overview: 从DS1302寄存器读数据
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
//Overview: DS1302初始化
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
		WriteDS1302( Add_CONTROL,0x00 );		//关闭写保护
		WriteDS1302( Add_MIN,0x08 );			//预置分钟时间
		WriteDS1302( Add_HR,0x20 );				//预置小时，24小时制
		WriteDS1302( Add_DATE,0x08 );			//预置日期
		WriteDS1302( Add_MONTH,0x08 );			//预置月份
		WriteDS1302( Add_DAY,0x05 );			//预置星期
		WriteDS1302( Add_YEAR,0x08 );			//预置年份
		WriteDS1302( Add_CHARGER,0x0a6 );		//写充电控制寄存器，1010 0110，I=1mA
		WriteDS1302( Add_SEC,0x08 );			//启动时钟
		WriteDS1302( Add_RAM0,0x5A);			//RAM0单元写入0x55，防止再次初始化
		WriteDS1302( Add_CONTROL,0x80 );		//打开写保护 
	}
#endif
}

//////////////////////////////////////////////////////////////////////
//Function void ReadRTC(BYTE *buffer)
//Intput:
//			缓冲区指针
//
//Output:	NULL
//			
//Overview: 从DS1302连续读数据，并把数据保存到指定的缓冲区
//SEC MIN HR DATE MONTH DAY YEAR CONTROL	返回BCD码
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
//			缓冲区指针
//
//Output:	NULL
//			
//Overview: 把缓冲区的数据连续写人DS1302，更新实时时钟
//SEC MIN HR DATE MONTH DAY YEAR CONTROL	输入BCD码
void UpdataRTC(BYTE *buffer)
{
	BYTE i,j;
	BYTE ds_addr,ds_data;
	WriteDS1302( Add_CONTROL,0x00 );		//关闭写保护 
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
//	WriteDS1302( Add_SEC,0x30 );			//启动时钟
	WriteDS1302( Add_CONTROL,0x80 );		//打开写保护
}

//////////////////////////////////////////////////////////////////////
//Function void Hex2BCD(BYTE *data,BYTE amount)
//Intput:
//			*data： 缓冲区指针
//			amount: 数据个数
//Output:	NULL
//			
//Overview: 把缓冲区的十六进制数据转成BCD码
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
//			*data： 缓冲区指针
//			amount: 数据个数
//Output:	NULL
//			
//Overview: 把缓冲区的BCD码转成十六进制数据
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
