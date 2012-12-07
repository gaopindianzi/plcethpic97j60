#ifndef __DS18B20_H
#define __DS18B20_H

#define uchar			unsigned char
#define uint			unsigned int

#define DQ_OUT				0
#define DQ_IN				1
//#define TH					0xd0
//#define TL					0xf5
//#define ConfigWord			0x7f
//ROM FUNCTION COMMANDS
#define ReadROM				0x33
#define MatchROM			0x55
#define SkipROM				0xcc
#define SearchROM			0xf0
#define AlarmSearch			0xec
//MEMORY COMMAND FUNCTIONS
#define WriteScratchpad		0x4e
#define ReadScratchpad		0xbe
#define CopyScratchpad		0x48
#define ConvertT			0x44
#define RecallE2			0xb8
#define ReadPowerSupply		0xb4
//#define Multiple_DS18B20
void delay(uchar n);
uchar Bus_reset(void);
uchar Read_bit(void);
void Write_bit(uchar bit_value);
uchar Read_byte(void);
void Write_byte(uchar bytevalue);
void Read_ROMID(void);
void Send_matchRom(void);
void Skip_Rom(void);
void Read_scratchPad(uchar *buffer);
void Write_scratchPad(void);
void Convert_T(void);
void Read_Temperature(void);
void DS18B20_Init(void);

#endif	//__DS18B20_H