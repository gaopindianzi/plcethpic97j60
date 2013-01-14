#include "TCPIP Stack/TCPIP.h"

#include "DS18B20.h"
#include "ds18b20_io.h"

#if  defined(STACK_USE_DS18B20)

char Set_Buff[3] = {0xd0,0xf5,0x7f};
unsigned int TP;

//uchar ScratchPad[9]={0};
struct _ScratchPad
{
	BYTE TP_L;
	BYTE TP_M;
	BYTE TH;
	BYTE TL;
	BYTE CONFIG;
	BYTE RESERVED1;
	BYTE RESERVED2;
	BYTE RESERVED3;
	BYTE CRC;
} ScratchPad;
#if defined (Multiple_DS18B20)
uchar ROMID[8] = {0};
#endif

void delay(uchar n)			//delay(0)    13us
{							//delay(1)    20us
	uchar i;				//delay(2)    27us
	for(i = 0; i < n; i++)	//
	{
		Nop();				//delay(50)   73us
	}
							//delay(255)  360us
}
/**********************Reset*********************/
uchar Bus_reset(void)
{
	uchar presence;
	set_temp_io_low();  //Temp_DQ_OUT = 0;
	set_temp_io_dir_out();  //Temp_DQ_TRIS = DQ_OUT;
	delay(100);
	delay(255);
	delay(255);
	delay(255);			//master keep 480~960 us
	set_temp_io_in_high(); //Temp_DQ_IN = 1;
	set_temp_io_dir_in(); //Temp_DQ_TRIS = DQ_IN;
	delay(85);				// DS18B20 wait 15~60 us ,then active low
	presence = get_temp_io_val(); //Temp_DQ_IN;
	delay(110);
	delay(200);			// delay 60~240 us
	return presence;
}
/**********************Read one bit*********************/
uchar Read_bit(void)
{
	uchar bit_value;
	Temp_DQ_OUT = 0;
	Temp_DQ_TRIS = DQ_OUT;
	delay(13);				// delay 0~15us
	Temp_DQ_IN = 1;
	Temp_DQ_TRIS = DQ_IN;
	bit_value = Temp_DQ_IN;
	delay(72);				//delay 55us
	return bit_value;
}

/********************Write one bit*************************/
void Write_bit(uchar bit_value)
{
	Temp_DQ_OUT = 0;
	Temp_DQ_TRIS = DQ_OUT;
	delay(13);
	if(bit_value ==	1)
	Temp_DQ_OUT = 1;
	delay(72);				//delay 58us
	Temp_DQ_TRIS = DQ_IN;
}

/********************Read one byte*******/
uchar Read_byte(void)
{
	uchar i;
	uchar value = 0;
	for(i = 0; i < 8; i++)
	{
		if(Read_bit())
		value = value | (0x01 << i);			//ds18b20的数据输出从第0位开始输出
	}
	delay(2);
	return value;
}

/**********************Write one byte*********************/
void Write_byte(uchar bytevalue)
{
	uchar i;
	uchar temp;
	for(i = 0; i < 8; i++)
	{
		temp = bytevalue >> i;
		temp = temp&0x01;
		Write_bit(temp);
	}
	delay(2);
}
/*************************Read ROMID***************************/
#if defined (Multiple_DS18B20)
void Read_ROMID(void)
{
	uchar i;
	Write_byte(ReadROM);
	for(i = 0; i < 8; i++)
	{
		ROMID[i] = Read_byte();
	}
}
#endif
/*******************Send match ROM****************************/
#if defined (Multiple_DS18B20)
void Send_matchRom(void)
{
	uchar i;
	i = 10;
	while(Bus_reset() && i)
		i--;
	Write_byte(MatchROM);
	for(i = 0; i < 8; i++)
	Write_byte(ROMID[i]);
}
#endif
/***********************Skip rom*******************************/
void Skip_Rom(void)
{
	uchar i;
	i = 2;
	while(Bus_reset() && (i > 0))
		i--;
	Write_byte(SkipROM);
}

/*******************Read scratchPad**********************/
void Read_scratchPad(uchar *buffer)
{
	uchar i;
	Skip_Rom();
//	Send_matchRom();
	Write_byte(ReadScratchpad);  
	for(i = 0; i < 9; i++)
	*buffer++ = Read_byte();
}

/***************************Write TH，TL Config ***********************/
void Write_scratchPad(void)
{
	uchar i;
	Skip_Rom();
//	Send_matchRom();
	Write_byte(WriteScratchpad);
	for(i = 0; i < 3; i++)
	Write_byte(Set_Buff[i]);
}

/**********************Convert T*****************************************/
void Convert_T(void)
{
	Skip_Rom();
//	Send_matchRom();
	Write_byte(ConvertT);
}

/**********************Read Temperature**********************************/
unsigned int Read_Temperature(void)
{
	Read_scratchPad( &ScratchPad );
	TP = ScratchPad.TP_M;
	TP = TP << 8;
	TP |= ScratchPad.TP_L;
	return TP;
}

/************************************************************************
 * 功能:  读取温度，精度是0.01摄氏度
 */
unsigned int ReadTemperatureXX_XC(void)
{
	unsigned long TP_temp;
	Convert_T();
	TP_temp = Read_Temperature();
	TP_temp *= 100;
	TP_temp /= 16;
	return (unsigned int)TP_temp;
}

/************************************************************************
 * 功能:  读取温度，精度是0.01摄氏度
 */
unsigned int ReadTemperatureChannel(unsigned char index)
{
	unsigned long TP_temp;
	set_temp_channel(index);
	Convert_T();
	TP_temp = Read_Temperature();
	TP_temp *= 100;
	TP_temp /= 16;
	return (unsigned int)TP_temp;
}

 /***********************Initial DS18B20*********************************/
void DS18B20_Init(void)
{
	uchar i;

	set_temp_channel(0);

	i = 2;
	while(Bus_reset() && (i > 0))
		i--;
//	Read_ROMID();
	Write_scratchPad();
	Convert_T();
}

#endif
