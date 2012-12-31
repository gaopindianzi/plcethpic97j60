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
 * ���ܣ���ʼ��IO�̵�����״̬
 * ���룺
 *     startbits  :  ��ʼλ
 *     iobits     :  λ��������
 *     bitcount   :  λ������
 * �����    
 * ����ֵ��
 *     ���һ�����Σ��������λ������
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
 * ���ܣ���תĳЩ���λ
 * ���룺
 *     startbits  :  ��ʼλ
 *     iobits     :  λ��������
 *     bitcount   :  λ������
 * �����    
 * ����ֵ��
 *     ���һ�����Σ��������λ������
 */

unsigned int io_out_convert_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i;
	//���������������
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//��һ���ж��Ƿ��������
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//��ʼ����
	for(i=0;i<bitcount;i++) {
		if(BIT_IS_SET(iobits,i)) {
	       unsigned char ch = get_bitval(AUXI_RELAY_BASE+startbits+i);
	       set_bitval(AUXI_RELAY_BASE+startbits+i,!ch);
		}
	}
	return bitcount;
}


/*************************************************************
 * ���ܣ�����ĳЩ���λ
 * ���룺
 *     startbits  :  ��ʼλ
 *     iobits     :  λ��������
 *     bitcount   :  λ������
 * �����    
 * ����ֵ��
 *     ���һ�����Σ��������λ������
 */
unsigned int io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i;
	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//���������������
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//��һ���ж��Ƿ��������
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//��ʼ����
	//printf("io_out_set_bits startbits = %d , bit count = %d\r\n",startbits,bitcount);
	for(i=0;i<bitcount;i++) {
	   unsigned char ch = BIT_IS_SET(iobits,i);
	   set_bitval(AUXI_RELAY_BASE+startbits+i,ch);
	}
	//����
	return bitcount;
}

/*************************************************************
 * ���ܣ�����ĳЩ���λ
 * ���룺
 *     startbits  :  ��ʼλ
 *     iobits     :  λ��������
 *     bitcount   :  λ������
 * �����    
 * ����ֵ��
 *     ���һ�����Σ��������λ������
 */
unsigned int phy_io_out_set_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i,index;
	unsigned char Bb,Bi;
	//uint32_t tmp;
	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//���������������
	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//��һ���ж��Ƿ��������
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//��ʼ����
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
	//����IO��
	RELAY_OUT_0 = (io_out[0]&0x01)?0:1;
	RELAY_OUT_1 = (io_out[0]&0x02)?0:1;
	RELAY_OUT_2 = (io_out[0]&0x04)?0:1;
	RELAY_OUT_3 = (io_out[0]&0x08)?0:1;
	RELAY_OUT_4 = (io_out[0]&0x10)?0:1;
	RELAY_OUT_5 = (io_out[0]&0x20)?0:1;
	RELAY_OUT_6 = (io_out[0]&0x40)?0:1;

	//RtcRamWrite(0,io_out,sizeof(io_out));
	//����
	return bitcount;
}


/*************************************************************
 * ���ܣ���ȡ�̵��������ֵ
 * ���룺
 *     startbits  :  ��ʼλ
 *     iobits     :  λ��������
 *     bitcount   :  λ������
 * �����
 * ����ֵ��
 *     ���һ�����Σ��������λ������
 */
unsigned int io_out_get_bits(unsigned int startbits,unsigned char * iobits,unsigned int bitcount)
{
	unsigned int i,index;
	unsigned char Bb,Bi;
	//uint32_t tmp;

	memset(iobits,0,(bitcount+7)/8);

	//_ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//���������������

	if(startbits >= REAL_IO_OUT_NUM || bitcount == 0) {
		return 0;
	}
	//��һ���ж��Ƿ��������
	if((REAL_IO_OUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_OUT_NUM - startbits;
	}
	//printf("io_out_get_bits startbits = %d , bit count = %d\r\n",startbits,bitcount);
	//��ʼ����
	for(i=0;i<bitcount;i++) {
		unsigned char on = get_bitval(AUXI_RELAY_BASE+startbits+i);
		SET_BIT(iobits,i,on);
	}
	return bitcount;
}


/*************************************************************
 * ���ܣ���ȡ������������ֵ
 * ���룺
 *     startbits  :  ��ʼλ
 *     bitcount   :  λ������
 * �����
 *     iobits     :  ������������������IO����ֵλ�����ŵ����������
 * ����ֵ��
 *     ���һ�����Σ��������λ������
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
	//��һ���ж��Ƿ��������
	if((REAL_IO_INPUT_NUM - startbits) < bitcount) {
		bitcount = REAL_IO_INPUT_NUM - startbits;
	}
	//��ʼ����
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
	XEEBeginWrite(base);	
	while(i--) {
		XEEWrite(*buffer++);
	}
	XEEEndWrite();
	return len;
}
