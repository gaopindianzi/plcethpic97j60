#ifndef __DS18B20_IO_H__
#define __DS18B20_IO_H__


void set_temp_channel(unsigned char index);
void set_temp_io_high(void);
void set_temp_io_low(void);
void set_temp_io_dir_in(void);
void set_temp_io_dir_out(void);
unsigned char get_temp_io_val(void);
void set_temp_io_in_low(void);
void set_temp_io_in_high(void);


#endif

