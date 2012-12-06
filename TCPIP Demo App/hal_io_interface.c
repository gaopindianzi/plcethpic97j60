#include "TCPIP Stack/TCPIP.h"

#include "MainDemo.h"


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define REAL_IO_OUT_NUM          7
#define REAL_IO_INPUT_NUM        8


#define BITS_TO_BS(bit_num)    (((bit_num)+7)/8)

unsigned char io_out[BITS_TO_BS(REAL_IO_OUT_NUM)] = {0};


const unsigned char  code_msk[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};


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
	unsigned int i,index;
	unsigned char Bb,Bi;
	//参数必须符合条件
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//进一步判断是否符合条件
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//开始设置
	index = 0;
	for(i=startbits;i<startbits+bitcount;i++) {
	    Bb = index / 8;
	    Bi = index % 8;
		if(iobits[Bb]&code_msk[Bi]) {
			io_out[i/8] ^= code_msk[i%8];
		}
		index++;
	}
	//设置IO口
	RELAY_OUT_0 = (io_out[0]&0x01)?0:1;
	RELAY_OUT_1 = (io_out[0]&0x02)?0:1;
	RELAY_OUT_2 = (io_out[0]&0x04)?0:1;
	RELAY_OUT_3 = (io_out[0]&0x08)?0:1;
	RELAY_OUT_4 = (io_out[0]&0x10)?0:1;
	RELAY_OUT_5 = (io_out[0]&0x20)?0:1;
	RELAY_OUT_6 = (io_out[0]&0x40)?0:1;
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
unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
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
	//设置IO口
	RELAY_OUT_0 = (io_out[0]&0x01)?0:1;
	RELAY_OUT_1 = (io_out[0]&0x02)?0:1;
	RELAY_OUT_2 = (io_out[0]&0x04)?0:1;
	RELAY_OUT_3 = (io_out[0]&0x08)?0:1;
	RELAY_OUT_4 = (io_out[0]&0x10)?0:1;
	RELAY_OUT_5 = (io_out[0]&0x20)?0:1;
	RELAY_OUT_6 = (io_out[0]&0x40)?0:1;
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
	index = 0;
	
	for(i=startbits;i<startbits+bitcount;i++) {
	    Bb = index / 8;
	    Bi = index % 8;
		if(io_out[i/8]&code_msk[i%8]) {
			iobits[Bb] |=  code_msk[Bi];
		} else {
			iobits[Bb] &= ~code_msk[Bi];
		}
		index++;
	}
	return bitcount;
}
