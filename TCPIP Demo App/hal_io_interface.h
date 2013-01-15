#ifndef __HAL_IO_INTERFACE_H__
#define __HAL_IO_INTERFACE_H__

#include "board_bsp_info.h"

#define REAL_IO_OUT_NUM         PHY_RELAY_OUT_NUM
#define REAL_IO_INPUT_NUM       PHY_DIG_IN_NUM

#define BITS_TO_BS(bit_num)    (((bit_num)+7)/8)

extern unsigned char io_out[BITS_TO_BS(REAL_IO_OUT_NUM)];
extern const unsigned char  code_msk[8];



extern unsigned int io_out_convert_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int phy_io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_in_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern void Io_Out_PowerInit(void);
extern void set_io_out_power_down_hold(unsigned char hold_on);
extern unsigned char get_io_out_power_down_hold(void);
extern unsigned int read_plc_programer(unsigned int index,unsigned char * buffer,unsigned int len);
extern unsigned int write_plc_programer(unsigned int index,unsigned char * buffer,unsigned int len);

#define  GET_MEMBER_BASE_OF_STRUCT(StructType,MemberName)    (&(((StructType *)0)->MemberName) - ((StructType *)0))

typedef struct _My_APP_Info_Struct
{
	APP_CONFIG     oapp;
	unsigned char  io_out_hold;
	unsigned char  uart2_baud_lo2;   //波特率最高字节
	unsigned char  uart2_baud_lo1;
	unsigned char  uart2_baud_lo0;
	unsigned char  uart2_pack_timeout;  //毫秒为单位
	unsigned char  plc_programer;
	unsigned char  plc_programer2[1024*4-1];
} My_APP_Info_Struct; 



#endif
