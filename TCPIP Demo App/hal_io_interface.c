#include "TCPIP Stack/TCPIP.h"

#include "MainDemo.h"
#include "DS1302.h"
#include "hal_io_interface.h"
#include "plc_prase.h"
#include "compiler.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>






unsigned char io_out[BITS_TO_BS(REAL_IO_OUT_NUM)] = {0};


const unsigned char  code_msk[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
/*************************************************************
 * 功能：初始化IO继电器的状态
 * 输入：
 *     startbits  :  起始位
 *     iobits     :  位变量数组
 *     bitcount   :  位的数量
 * 输出：    
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */

void Io_Out_PowerInit(void)
{
	unsigned int base = GET_MEMBER_BASE_OF_STRUCT(My_APP_Info_Struct,io_out_hold);
	XEEBeginRead(base);
	base = XEERead();
	XEEEndRead();
	if(base&0x01) {
	    //RtcRamRead(0,io_out,sizeof(io_out));
	   // io_out_set_bits(0,io_out,REAL_IO_OUT_NUM);
	} else {
		//memset(io_out,0,sizeof(io_out));
	}
	//io_out_set_bits(0,io_out,REAL_IO_OUT_NUM);
}

void set_io_out_power_down_hold(unsigned char hold_on)
{
	unsigned int base = GET_MEMBER_BASE_OF_STRUCT(My_APP_Info_Struct,io_out_hold);
	XEEBeginWrite(base);
	XEEWrite(hold_on?0x01:0x00);
	XEEEndWrite();
}

unsigned char get_io_out_power_down_hold(void)
{
	unsigned int base = GET_MEMBER_BASE_OF_STRUCT(My_APP_Info_Struct,io_out_hold);
	XEEBeginRead(base);
	base = XEERead();
	XEEEndRead();
	return base & 0x01;
}

/*************************************************************
 * 功能：翻转某些输出位
 * 输入：
 *     startbits  :  起始位
 *     iobits     :  位变量数组
 *     bitcount   :  位的数量
 * 输出：    
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */

unsigned int io_out_convert_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i;
	//参数必须符合条件
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//开始设置
	for(i=0;i<bitcount;i++) {
		if(BIT_IS_SET(iobits,i)) {
	       unsigned char ch = get_bitval(AUXI_RELAY_BASE+startbits+i);
	       set_bitval(AUXI_RELAY_BASE+startbits+i,!ch);
		}
	}
	return bitcount;
}


/*************************************************************
 * 功能：设置某些输出位
 * 输入：
 *     startbits  :  起始位
 *     iobits     :  位变量数组
 *     bitcount   :  位的数量
 * 输出：    
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */
unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i;
	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//参数必须符合条件
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//开始设置
	//printf("io_out_set_bits startbits = %d , bit count = %d\r\n",startbits,bitcount);
	for(i=0;i<bitcount;i++) {
	   unsigned char ch = BIT_IS_SET(iobits,i);
	   set_bitval(AUXI_RELAY_BASE+startbits+i,ch);
	}
	//返回
	return bitcount;
}

/*************************************************************
 * 功能：设置某些输出位
 * 输入：
 *     startbits  :  起始位
 *     iobits     :  位变量数组
 *     bitcount   :  位的数量
 * 输出：    
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */
unsigned int phy_io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i,index;
	unsigned char Bb,Bi;
	//uint32_t tmp;
	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//参数必须符合条件
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//开始设置
	//printf("io_out_set_bits startbits = %d , bit count = %d\r\n",startbits,bitcount);
	index = 0;
	for(i=startbits;i<startbits+bitcount;i++) {
	    Bb = index / 8;
	    Bi = index % 8;
		if(iobits[Bb]&code_msk[Bi]) {
			io_out[i/8] |=  code_msk[i%8];
		} else {
			io_out[i/8] &= ~code_msk[i%8];
		}
		index++;
	}
#if   BARED_TYPE == BOARD_R7OUT_D9IN_1TMP
	//设置IO口
	RELAY_OUT_0 = (io_out[0]&0x01)?0:1;
	RELAY_OUT_1 = (io_out[0]&0x02)?0:1;
	RELAY_OUT_2 = (io_out[0]&0x04)?0:1;
	RELAY_OUT_3 = (io_out[0]&0x08)?0:1;
	RELAY_OUT_4 = (io_out[0]&0x10)?0:1;
	RELAY_OUT_5 = (io_out[0]&0x20)?0:1;
	RELAY_OUT_6 = (io_out[0]&0x40)?0:1;
#endif
#if   BARED_TYPE == BOARD_R8OUT_D8IN_8TMP
	//设置IO口
	RELAY_OUT_0 = (io_out[0]&0x01)?0:1;
	RELAY_OUT_1 = (io_out[0]&0x02)?0:1;
	RELAY_OUT_2 = (io_out[0]&0x04)?0:1;
	RELAY_OUT_3 = (io_out[0]&0x08)?0:1;
	RELAY_OUT_4 = (io_out[0]&0x10)?0:1;
	RELAY_OUT_5 = (io_out[0]&0x20)?0:1;
	RELAY_OUT_6 = (io_out[0]&0x40)?0:1;
	RELAY_OUT_7 = (io_out[0]&0x80)?0:1;
#endif
	//RtcRamWrite(0,io_out,sizeof(io_out));
	//返回
	return bitcount;
}


/*************************************************************
 * 功能：读取继电器的输出值
 * 输入：
 *     startbits  :  起始位
 *     iobits     :  位变量数组
 *     bitcount   :  位的数量
 * 输出：
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */
unsigned int io_out_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i,index;
	unsigned char Bb,Bi;
	//uint32_t tmp;

	memset(iobits,0,(bitcount+7)/8);

	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//参数必须符合条件

	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//printf("io_out_get_bits startbits = %d , bit count = %d\r\n",startbits,bitcount);
	//开始设置
	for(i=0;i<bitcount;i++) {
		unsigned char on = get_bitval(AUXI_RELAY_BASE+startbits+i);
		SET_BIT(iobits,i,on);
	}
	return bitcount;
}


/*************************************************************
 * 功能：读取数字量的输入值
 * 输入：
 *     startbits  :  起始位
 *     bitcount   :  位的数量
 * 输出：
 *     iobits     :  输出缓冲区，把输出的IO输入值位变量放到这个数组中
 * 返回值：
 *     输出一个整形，代表输出位的数量
 */
unsigned int io_in_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int  i,index;
	unsigned char Bb,Bi;
	unsigned char buffer[2] = {0,0};

	memset(iobits,0,(bitcount+7)/8);

	if(startbits >= REAL_IO_INPUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_INPUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_INPUT_NUM - startbits;
	}
	//开始设置
	index = 0;
	//
	Bi = 0;
	Bi |= DIG_INPUT_IO_0?0x01:0x00;
	Bi |= DIG_INPUT_IO_1?0x02:0x00;
	Bi |= DIG_INPUT_IO_2?0x04:0x00;
	Bi |= DIG_INPUT_IO_3?0x08:0x00;
	Bi |= DIG_INPUT_IO_4?0x10:0x00;
	Bi |= DIG_INPUT_IO_5?0x20:0x00;
	Bi |= DIG_INPUT_IO_6?0x40:0x00;
	Bi |= DIG_INPUT_IO_7?0x80:0x00;
	buffer[0] = Bi;
	Bi = 0;
	Bi |= DIG_INPUT_IO_8?0x01:0x00;
	buffer[1] = Bi;

	for(i=startbits;i<startbits+bitcount;i++) {
	    Bb = index / 8;
	    Bi = index % 8;
		if(buffer[i/8]&code_msk[i%8]) {
			iobits[Bb] |=  code_msk[Bi];
		} else {
			iobits[Bb] &= ~code_msk[Bi];
		}
		index++;
	}
	return bitcount;
}


unsigned int read_plc_programer(unsigned int index,unsigned char * buffer,unsigned int len)
{
	unsigned int i = len;
	unsigned int base = index + GET_MEMBER_BASE_OF_STRUCT(My_APP_Info_Struct,plc_programer);
	XEEBeginRead(base);
	while(i--) {
		*buffer++ = XEERead();
	}
	XEEEndRead();
	return len;
}

unsigned int write_plc_programer(unsigned int index,unsigned char * buffer,unsigned int len)
{
	unsigned int i = len;
	unsigned int base = index + GET_MEMBER_BASE_OF_STRUCT(My_APP_Info_Struct,plc_programer);
	for(i=0;i<len;i++) {
	    XEEBeginWrite(base+i);
		XEEWrite(buffer[i]);
	    XEEEndWrite();
	}
	return len;
}



/********************************************************
 * 温度通道
 */

#define DQ_OUT				0
#define DQ_IN				1


static unsigned char temp_channel = 0;

void set_temp_channel(unsigned char index)
{
	temp_channel = index;
}

void set_temp_io_high(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_OUT = 1;
		break;
	case 1:
		Temp_DQ_OUT1 = 1;
		break;
	case 2:
		Temp_DQ_OUT2 = 1;
		break;
	case 3:
		Temp_DQ_OUT3 = 1;
		break;
	case 4:
		Temp_DQ_OUT4 = 1;
		break;
	case 5:
		Temp_DQ_OUT5 = 1;
		break;
	case 6:
		Temp_DQ_OUT6 = 1;
		break;
	case 7:
		Temp_DQ_OUT7 = 1;
		break;
	default:
		break;
	}
}

void set_temp_io_low(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_OUT = 0;
		break;
	case 1:
		Temp_DQ_OUT1 = 0;
		break;
	case 2:
		Temp_DQ_OUT2 = 0;
		break;
	case 3:
		Temp_DQ_OUT3 = 0;
		break;
	case 4:
		Temp_DQ_OUT4 = 0;
		break;
	case 5:
		Temp_DQ_OUT5 = 0;
		break;
	case 6:
		Temp_DQ_OUT6 = 0;
		break;
	case 7:
		Temp_DQ_OUT7 = 0;
		break;
	default:
		break;
	}
}


void set_temp_io_dir_in(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_TRIS = DQ_IN;
		break;
	case 1:
		Temp_DQ_TRIS1 = DQ_IN;
		break;
	case 2:
		Temp_DQ_TRIS2 = DQ_IN;
		break;
	case 3:
		Temp_DQ_TRIS3 = DQ_IN;
		break;
	case 4:
		Temp_DQ_TRIS4 = DQ_IN;
		break;
	case 5:
		Temp_DQ_TRIS5 = DQ_IN;
		break;
	case 6:
		Temp_DQ_TRIS6 = DQ_IN;
		break;
	case 7:
		Temp_DQ_TRIS7 = DQ_IN;
		break;
	default:
		break;
	}
}
void set_temp_io_dir_out(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_TRIS = DQ_OUT;
		break;
	case 1:
		Temp_DQ_TRIS1 = DQ_OUT;
		break;
	case 2:
		Temp_DQ_TRIS2 = DQ_OUT;
		break;
	case 3:
		Temp_DQ_TRIS3 = DQ_OUT;
		break;
	case 4:
		Temp_DQ_TRIS4 = DQ_OUT;
		break;
	case 5:
		Temp_DQ_TRIS5 = DQ_OUT;
		break;
	case 6:
		Temp_DQ_TRIS6 = DQ_OUT;
		break;
	case 7:
		Temp_DQ_TRIS7 = DQ_OUT;
		break;
	default:
		break;
	}
}

void set_temp_io_in_low(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_IN = 0;
		break;
	case 1:
		Temp_DQ_IN1 = 0;
		break;
	case 2:
		Temp_DQ_IN2 = 0;
		break;
	case 3:
		Temp_DQ_IN3 = 0;
		break;
	case 4:
		Temp_DQ_IN4 = 0;
		break;
	case 5:
		Temp_DQ_IN5 = 0;
		break;
	case 6:
		Temp_DQ_IN6 = 0;
		break;
	case 7:
		Temp_DQ_IN7 = 0;
		break;
	default:
		break;
	}
}
void set_temp_io_in_high(void)
{
	switch(temp_channel)
	{
	case 0:
		Temp_DQ_IN = 1;
		break;
	case 1:
		Temp_DQ_IN1 = 1;
		break;
	case 2:
		Temp_DQ_IN2 = 1;
		break;
	case 3:
		Temp_DQ_IN3 = 1;
		break;
	case 4:
		Temp_DQ_IN4 = 1;
		break;
	case 5:
		Temp_DQ_IN5 = 1;
		break;
	case 6:
		Temp_DQ_IN6 = 1;
		break;
	case 7:
		Temp_DQ_IN7 = 1;
		break;
	default:
		break;
	}
}
unsigned char get_temp_io_val(void)
{
	switch(temp_channel)
	{
	case 0:
		return Temp_DQ_IN;
	case 1:
		return Temp_DQ_IN1;
	case 2:
		return Temp_DQ_IN2;
	case 3:
		return Temp_DQ_IN3;
	case 4:
		return Temp_DQ_IN4;
	case 5:
		return Temp_DQ_IN5;
	case 6:
		return Temp_DQ_IN6;
	case 7:
		return Temp_DQ_IN7;
	default:
		return 1;
	}
	return 1;
}

