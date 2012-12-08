#ifndef __HAL_IO_INTERFACE_H__
#define __HAL_IO_INTERFACE_H__

extern unsigned int io_out_convert_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_out_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern unsigned int io_in_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount);
extern void Io_Out_PowerInit(void);

#endif
