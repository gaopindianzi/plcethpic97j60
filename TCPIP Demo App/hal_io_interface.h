#ifndef __HAL_IO_INTERFACE_H__
#define __HAL_IO_INTERFACE_H__


#define REAL_IO_OUT_NUM          7
#define REAL_IO_INPUT_NUM        9

#define BITS_TO_BS(bit_num)    (((bit_num)+7)/8)

extern unsigned char io_out[BITS_TO_BS(REAL_IO_OUT_NUM)];

extern unsigned int io_out_convert_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_in_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern void Io_Out_PowerInit(void);
extern void set_io_out_power_down_hold(unsigned char hold_on);
extern unsigned char get_io_out_power_down_hold(void);

#define  GET_MEMBER_BASE_OF_STRUCT(StructType,MemberName)    (&(((StructType *)0)->MemberName) - ((StructType *)0))

typedef struct __attribute__((__packed__)) _My_APP_Info_Struct
{
	APP_CONFIG     oapp;
	unsigned char  io_out_hold;
} My_APP_Info_Struct; 


#endif
