#ifndef __DS1302_H
#define __DS1302_H



//typedef unsigned char		BYTE;				// 8-bit unsigned

#define Add_SEC				0x80
#define Add_MIN				0x82
#define Add_HR				0x84
#define Add_DATE			0x86
#define Add_MONTH			0x88
#define Add_DAY				0x8a
#define Add_YEAR			0x8c
#define Add_CONTROL			0x8e
#define Add_CHARGER			0x90	
#define Add_CLOCKBURST		0xbe
#define Add_RAM0			0xc0
#define Add_RAM30			0xfc
#define Add_RAMBURST		0xfe

//#define Nop()               	asm("NOP");
/*
typedef union _BYTE_VAL
{
    BYTE Val;
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
    } bits;
} BYTE_VAL;
*/
//SEC MIN HR DATE MONTH DAY YEAR CONTROL

typedef struct _DS1302_VAL
{
	BYTE SEC;
	BYTE MIN;
	BYTE HR;
	BYTE DATE; 
	BYTE MONTH;
	BYTE DAY;
	BYTE YEAR;
	BYTE CONTROL;
} DS1302_VAL;

void DS1302_Init(void);
void WriteDS1302( BYTE ds_addr, BYTE ds_data );
BYTE ReadDS1302( BYTE ds_addr );
void DS1302Initial(void);
void ReadRTC(BYTE *buffer);
void UpdataRTC(BYTE *buffer);
void delay1us(void);
void Hex2BCD(BYTE *data, BYTE amount);
void BCD2Hex(BYTE *data, BYTE amount);
#endif
