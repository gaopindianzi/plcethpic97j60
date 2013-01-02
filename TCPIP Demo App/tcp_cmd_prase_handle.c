/************************************************************
 * 这个文件只处理协议，不管是什么接收到的数据，均在这里处理
 * 处理完毕，返回一定长度的数据，然后底层根据不同的接口，给
 * 主机返回一定长度的数据。
 * 与接口无关
 */

#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tcp_cmd_prase_handle.h"
#include "relay_cmd_definition.h"
#include "hal_io_interface.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "plc_prase.h"
#include "compiler.h"
#include "debug.h"

#define THISINFO           1
#define THISERROR          0


//uint8_t    command_state;



/*************************************************************
 * 功能：计算校验值
 * 输入：
 *     buffer  :  输入的数据
 *     len     :  属于的长度，字节单位
 * 输出：
 * 返回值：
 *     输出一个字节，代表校验和
 */
static unsigned char CalCheckSum(void * buffer,unsigned int len)
{
  unsigned char sum = 0;
  unsigned int  i;
  for(i=0;i<len;i++) {
    sum += ((unsigned char *)buffer)[i];
  }
  return sum;
}


/*************************************************************
 * 功能：翻转某些继电器
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdRevertIoOutIndex(CmdHead * cmd,unsigned int len)
{
    CmdIobitmap      *   io  = (CmdIobitmap *)GET_CMD_DATA(cmd);
    //
	if(THISINFO)putrsUART((ROM char*)"\r\n CmdRevertIoOutIndex().");

    if(len < sizeof(CmdHead)+sizeof(CmdIobitmap)) {
      if(THISERROR)putrsUART((ROM char*)"\r\n CmdRevertIoOutIndex() datasize error.");
	  cmd->cmd_option    = CMD_ACK_KO;
	  io_out_get_bits(0,io->io_msk,32);
      goto error;
    }
    //
    cmd->cmd_option    = CMD_ACK_OK;
	io_out_convert_bits(0,io->io_msk,32);
	io_out_get_bits(0,io->io_msk,32);
error:
    cmd->cmd_len       = sizeof(CmdIobitmap);
    cmd->data_checksum = 0;
	return sizeof(CmdHead)+sizeof(CmdIobitmap);
}
/*************************************************************
 * 功能：设置某些继电器
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdSetIoOutValue(CmdHead * cmd,unsigned int len)
{
    CmdIoValue    *   io  = (CmdIoValue *)GET_CMD_DATA(cmd);
    if(len < sizeof(CmdHead)+sizeof(CmdIoValue)) {
        cmd->cmd_option  = CMD_ACK_KO;
		io->io_count = io_out_get_bits(0,io->io_value,32);
		goto error;
    }
	io_out_set_bits(0,io->io_value,io->io_count);
	io->io_count = io_out_get_bits(0,io->io_value,32); //32个继电器，这个指令固定支持32位比特数量。
	cmd->cmd_option    = CMD_ACK_OK;
    cmd->cmd_len       = sizeof(CmdIoValue);
    cmd->data_checksum = 0;
error:
    return sizeof(CmdHead)+sizeof(CmdIoValue);
}

/*************************************************************
 * 功能：读某些继电器的状态
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */

unsigned int CmdGetIoOutValue(CmdHead * cmd,unsigned int len)
{
	CmdIoValue    *  sio  = (CmdIoValue *)GET_CMD_DATA(cmd);
	sio->io_count = io_out_get_bits(0,sio->io_value,32);
	{
		unsigned int i;
		sio->io_count = (sio->io_count >= 32)?32:sio->io_count;
		for(i=0;i<sio->io_count;i++) {
			unsigned char bitval = get_bitval(IO_OUTPUT_BASE+i);
			SET_BIT(sio->io_value,i,bitval);
		}
	}

    cmd->cmd_option    = CMD_ACK_OK;
    cmd->cmd_len       = sizeof(CmdIoValue);
    cmd->data_checksum = 0;
	return sizeof(CmdHead)+sizeof(CmdIoValue);
}

/*************************************************************
 * 功能：设置或清零或反转一个IO口的输出
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 *    setmode    :  设置模式：0表示设置，1表示清零，2表示反转
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdSetClrVerIoOutOneBit(CmdHead * cmd,unsigned int len,unsigned char setmode)
{
    unsigned char reg;
	unsigned int  num;
	CmdIoOneBit      *   io  =  (CmdIoOneBit *)GET_CMD_DATA(cmd);
    CmdIoValue       *  sio   = (CmdIoValue *)GET_CMD_DATA(cmd);
	//
    if(len < sizeof(CmdHead) + sizeof(CmdIoOneBit)) {
	  cmd->cmd_option    = CMD_ACK_KO;
	  sio->io_count = io_out_get_bits(0,sio->io_value,32);
      goto error;
    }
    //
    cmd->cmd_option    = CMD_ACK_OK;	
	num = io->io_bitnum[1];
	num <<= 8;
	num += io->io_bitnum[0];
	if(setmode == 0) {
		reg = 0x01;
	    io_out_set_bits(num,&reg,1);
		//set_bitval(IO_OUTPUT_BASE+num,1);
	} else if(setmode == 1) {
		reg = 0x00;
	    io_out_set_bits(num,&reg,1);
		//set_bitval(IO_OUTPUT_BASE+num,0);
	} else if(setmode == 2) {
		reg = 0x01;
		io_out_convert_bits(num,&reg,1);
		//set_bitval(IO_OUTPUT_BASE+num,!get_bitval(IO_OUTPUT_BASE+num));
	}
	sio->io_count = io_out_get_bits(0,sio->io_value,32);
	//{
	//	unsigned int i;
	//	sio->io_count = (sio->io_count >= 32)?32:sio->io_count;
	//	for(i=0;i<sio->io_count;i++) {
	//		unsigned char bitval = get_bitval(IO_OUTPUT_BASE+i);
	//		SET_BIT(sio->io_value,i,bitval);
	//	}
	//}
error:
    cmd->cmd_len       = sizeof(CmdIoValue);
    cmd->data_checksum = 0;
    return sizeof(CmdHead)+sizeof(CmdIoValue);
}

/*************************************************************
 * 功能：读取数字量的输入状态
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdGetIoInValue(CmdHead * cmd,unsigned int len)
{
    CmdIoValue    *  sio  = (CmdIoValue *)GET_CMD_DATA(cmd);
	sio->io_count = io_in_get_bits(0,sio->io_value,32);
	{
		unsigned int i;
		sio->io_count = (sio->io_count >= 32)?32:sio->io_count;
		for(i=0;i<sio->io_count;i++) {
			unsigned char bitval = get_bitval(IO_INPUT_BASE+i);
			SET_BIT(sio->io_value,i,bitval);
		}
	}

    cmd->cmd_option    = CMD_ACK_OK;
    cmd->cmd_len       = sizeof(CmdIoValue);
    cmd->data_checksum = 0;
	return sizeof(CmdHead)+sizeof(CmdIoValue);
}

/*************************************************************
 * 功能：一些不支持的指令，直接调用默认的函数返回即可
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdDefaultAck(CmdHead * cmd,unsigned int len)
{
	cmd->cmd_option    = CMD_ACK_KO;
    cmd->cmd_len       = 0;
    cmd->data_checksum = 0;
	return sizeof(CmdHead);
}
/*************************************************************
 * 功能：写寄存器，实现通用接口
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdWriteRegister(CmdHead * cmd,unsigned int len)
{
	unsigned int  base;
	unsigned int  addr;
	unsigned int  datlen;
	CmdRegister * preg = (CmdRegister *)GET_CMD_DATA(cmd);
	if(len < (sizeof(CmdHead) + sizeof(CmdRegister) - 1)) {
		cmd->cmd_option    = CMD_ACK_KO;
		cmd->cmd_len       = 0;
		goto error;
	}
	addr = preg->reg_addr_hi;
	addr <<= 8;
	addr += preg->reg_addr_lo;
	datlen = preg->reg_len_hi;
	datlen <<= 8;
	datlen += preg->reg_len_lo;
#if 0
	if(addr == 0) {
		if(datlen >= sizeof(CmdIoValue)) {
			CmdIoValue * sio = (CmdIoValue *)preg;
			sio->io_count = io_out_get_bits(0,sio->io_value,32);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(CmdIoValue);
		}
	}
	if(addr == 1) {
		if(datlen >= sizeof(CmdIoValue)) {
			CmdIoValue * sio = (CmdIoValue *)preg;
			sio->io_count = io_in_get_bits(0,sio->io_value,32);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(CmdIoValue);
		}
	}
	if(addr == 2) {
		if(datlen >= sizeof(DS1302_VAL)) {
			//读DS1302寄存器
			ReadRTC(preg);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(DS1302_VAL);
		}
	}
#endif
	if(addr == 3) {
		RtcRamWrite(0,&(preg->reg_base),datlen);
		cmd->cmd_option    = CMD_ACK_OK;
		RtcRamRead(0,preg,31);
		cmd->cmd_len = 31;
	}
    base = GET_OFFSET_MEM_OF_STRUCT(My_APP_Info_Struct,plc_programer);
	if(addr >= base && addr < sizeof(My_APP_Info_Struct)) {
		//可以写入内存
		unsigned long i;
		unsigned char * pbuf = &(preg->reg_base);
	    for(i=0;i<datlen;i++) {
		    XEEBeginWrite(addr+i);
		    XEEWrite(pbuf[i]);
		    XEEEndWrite();
	    }
		XEEBeginRead(addr);
		for(i=0;i<datlen;i++) {
			pbuf[i] = XEERead();
		}
		XEEEndRead();
		cmd->cmd_option = CMD_ACK_OK;
		cmd->cmd_len = sizeof(CmdRegister) - 1 + datlen;
	}
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + sizeof(CmdRegister) - 1 + datlen;
}
/*************************************************************
 * 功能：读寄存器，实现通用接口
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdReadRegister(CmdHead * cmd,unsigned int len)
{
	unsigned int  base;
	unsigned int  addr;
	unsigned int  datlen;
	CmdRegister * preg = (CmdRegister *)GET_CMD_DATA(cmd);
	if(len < (sizeof(CmdHead) + sizeof(CmdRegister) - 1)) {
		cmd->cmd_option    = CMD_ACK_KO;
		cmd->cmd_len       = 0;
		goto error;
	}
	addr = preg->reg_addr_hi;
	addr <<= 8;
	addr += preg->reg_addr_lo;
	datlen = preg->reg_len_hi;
	datlen <<= 8;
	datlen += preg->reg_len_lo;
	if(addr == 0) {
		if(datlen >= sizeof(CmdIoValue)) {
			CmdIoValue * sio = (CmdIoValue *)preg;
			sio->io_count = io_out_get_bits(0,sio->io_value,32);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(CmdIoValue);
		}
	}
	if(addr == 1) {
		if(datlen >= sizeof(CmdIoValue)) {
			CmdIoValue * sio = (CmdIoValue *)preg;
			sio->io_count = io_in_get_bits(0,sio->io_value,32);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(CmdIoValue);
		}
	}
	if(addr == 2) {
		if(datlen >= sizeof(DS1302_VAL)) {
			//读DS1302寄存器
			ReadRTC(preg);
			cmd->cmd_option    = CMD_ACK_OK;
			cmd->cmd_len = sizeof(DS1302_VAL);
		}
	}
	if(addr == 3) {
		RtcRamRead(0,preg,31);
		cmd->cmd_option    = CMD_ACK_OK;
		cmd->cmd_len = 31;
	}
	if(addr == 4) {
		//读出来的温度是摄氏度
		unsigned int TP_temp = ReadTemperatureXX_XC();
		unsigned char * pb = preg;
		pb[0] = TP_temp & 0xFF;
		pb[1] = TP_temp >> 8;
		cmd->cmd_option    = CMD_ACK_OK;
		cmd->cmd_len = 2;
		Convert_T();
	}
    base = GET_OFFSET_MEM_OF_STRUCT(My_APP_Info_Struct,plc_programer);
	if(addr >= base && addr < sizeof(My_APP_Info_Struct)) {
		//可以写入内存
		unsigned long i;
		unsigned char * pbuf = &(preg->reg_base);
		XEEBeginRead(addr);
		for(i=0;i<datlen;i++) {
			pbuf[i] = XEERead();
		}
		XEEEndRead();
		cmd->cmd_option = CMD_ACK_OK;
		cmd->cmd_len = sizeof(CmdRegister) - 1 + datlen;
	}
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + sizeof(CmdRegister) - 1 + datlen;
}
/*************************************************************
 * 功能：设置实时时钟
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdSetNewRtcValue(CmdHead * cmd,unsigned int len)
{
	time_type_e *  pt  = (time_type *)GET_CMD_DATA(cmd);
	if(len < (sizeof(CmdHead) + sizeof(time_type))) {
		cmd->cmd_option    = CMD_ACK_KO;
		cmd->cmd_len       = 0;
		goto error;
	}
	do{
		DS1302_VAL tval;
		tval.SEC = pt->sec;
		tval.MIN = pt->min;
		tval.HR  = pt->hour;
		tval.DATE = pt->day;
		tval.MONTH = pt->mon;
		tval.YEAR = pt->year;
		if(len >= (sizeof(CmdHead) + sizeof(time_type_e))) {
			tval.DAY = pt->week;
		}
		Hex2BCD(&tval,sizeof(tval));
		UpdataRTC(&tval);
	}while(0);
	cmd->cmd_option    = CMD_ACK_OK;
	cmd->cmd_len       = 0;
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
}
/*************************************************************
 * 功能：读取实时时钟
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdGetRtcValue(CmdHead * cmd,unsigned int len)
{
	time_type_e *  pt  = (time_type_e *)GET_CMD_DATA(cmd);
	DS1302_VAL tval;
	ReadRTC(&tval);
	BCD2Hex(&tval,sizeof(tval));
	pt->sec = tval.SEC;
	pt->min = tval.MIN;
	pt->hour = tval.HR;
	pt->day = tval.DATE;
	pt->mon = tval.MONTH;
	pt->year = tval.YEAR;
	pt->week = tval.DAY;
	cmd->cmd_option    = CMD_ACK_OK;
	cmd->cmd_len       = sizeof(time_type_e);
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
}

/*************************************************************
 * 功能：设置继电器断电保持
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdSetIoOutPownDownHold(CmdHead * cmd,unsigned int len)
{
	CmdMode *  pt  = (CmdMode *)GET_CMD_DATA(cmd);
	if(len < sizeof(CmdHead) + sizeof(CmdMode)) {
		cmd->cmd_option    = CMD_ACK_KO;
		cmd->cmd_len       = 0;
		goto error;
	}
	set_io_out_power_down_hold(pt->mode);
	pt->mode = get_io_out_power_down_hold();
	cmd->cmd_option    = CMD_ACK_OK;
	cmd->cmd_len       = sizeof(CmdMode);
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
}
/*************************************************************
 * 功能：读取继电器断电保持
 * 输入：
 *     cmd       :  控制协议命令头指针，数据紧跟其后
 *     len       :  接收到的指令数据总长度，没有经过验证长度是否有效
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 */
unsigned int CmdGetIoOutPownDownHold(CmdHead * cmd,unsigned int len)
{
	CmdMode *  pt  = (CmdMode *)GET_CMD_DATA(cmd);
	pt->mode = get_io_out_power_down_hold();
	cmd->cmd_option    = CMD_ACK_OK;
	cmd->cmd_len       = sizeof(CmdMode);
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
}

/*************************************************************
 * 功能：PLC程序读
 */
unsigned int CmdPlcRead(CmdHead * cmd,unsigned int len)
{
	typedef struct _plc_rw
	{
		unsigned char cmd;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char data_len_hi;
		unsigned char data_len_lo;
		//后面接着是数据
		//unsigned char data_base;
	} plc_rw;
	if(len >= sizeof(plc_rw)) {
		plc_rw * pw = (plc_rw *)cmd;
		plc_rw * pack = (plc_rw *)pw;
		unsigned char *pbuf = ((unsigned char *)pack) + sizeof(plc_rw);
		unsigned int  base = HSB_BYTES_TO_WORD(&pw->start_addr_hi);
		unsigned datlen = HSB_BYTES_TO_WORD(&pw->data_len_hi);
		if(datlen > RELAY_CMD_MAX_PACKET_LEN - sizeof(plc_rw)) {
			datlen = RELAY_CMD_MAX_PACKET_LEN - sizeof(plc_rw);
		}
		if(datlen > 0) {
		    datlen = read_plc_programer(base,pbuf,datlen);
			pack->data_len_hi = datlen >> 8;
			pack->data_len_lo = datlen & 0xFF;
		} else {
			pack->data_len_hi = 0;
			pack->data_len_lo = 0;
		}
	    //把数据返回
	    return sizeof(plc_rw)+datlen;
	} else {
		//无效的指令
		return 0;
	}

}

/*************************************************************
 * 功能：PLC程序写
 */
unsigned int CmdPlcWrite(CmdHead * cmd,unsigned int len)
{
	typedef struct _plc_rw
	{
		unsigned char cmd;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char data_len_hi;
		unsigned char data_len_lo;
		//后面接着是数据
		//unsigned char data_base;
	} plc_rw;
	if(len >= sizeof(plc_rw)) {
		plc_rw * pw = (plc_rw *)cmd;
		plc_rw * pack = (plc_rw *)pw;
		unsigned char *pbuf = ((unsigned char *)pack) + sizeof(plc_rw);
		unsigned int  base = HSB_BYTES_TO_WORD(&pw->start_addr_hi);
		unsigned int  datlen = HSB_BYTES_TO_WORD(&pw->data_len_hi);
		if(datlen > (len - sizeof(plc_rw))) {
			datlen = len - sizeof(plc_rw);
		}
		if(datlen > RELAY_CMD_MAX_PACKET_LEN - sizeof(plc_rw)) {
			datlen = RELAY_CMD_MAX_PACKET_LEN - sizeof(plc_rw);
		}
		if(datlen > 0) {
		    len = write_plc_programer(base,pbuf,datlen);
			pack->data_len_hi = datlen >> 8;
			pack->data_len_lo = datlen & 0xFF;
		} else {
			pack->data_len_hi = 0;
			pack->data_len_lo = 0;
		}
	    //把数据返回
	    return sizeof(plc_rw)+datlen;
	} else {
		//无效的指令
		return 0;
	}
}
/*************************************************************
 * 功能：命令解析函数
 *       解析和TCP包,返回一定长度的应答包，然后返回和发送给客户端
 * 输入：
 *     pdat      :  输入的当前socket数据
 *     len       :  接收到的指令数据总长度
 * 输出：
 *     cmd       :  输出的数据也保存在cmd指针的长度里面，长度最大不能
 *                  超过预定的缓冲区(传入的缓冲数组大小：RELAY_CMD_MAX_PACKET_LEN)
 * 返回值：
 *     输出一个整形，代表输出数据的长度，如果长度为0，则表示没有输出，
 *     如果长度大于0，则表示有数据输出，则底层数据必须返回此函数输出
 *     的数据。
 * 添加字符串命令
 * SCMD DIGW 0  12 10011101 
 * SCMD DIGR 0  12 
 */


char StringMatchHead(const char * des,ROM char * head)
{
	while(1) {
		if(*des != *head) {
			return 0;
		}
		des++;
		head++;
    if(*head == '\0') {
			return 1;
		}
		if(*des == '\0') {
			return 0;
		}
	}
}

unsigned char GetStringDiviseBy(const char * src,ROM char *ch,
						  char * des,unsigned char len,unsigned char index)
{
	unsigned char k,i;
	const char * pch;
	if(*ch == '\0') {
		return 0;
	}
	k = 0;
	while(k<index) {
		pch = ch;
		while(1) {
			if(*pch == '\0') {
				break;
			}
			if(*src == *pch) {
				k++;
				break;
			} else {
				pch++;
			}
		}
		if(*src == '\0') {
			return 0;
		}
		src++;
	}
	k = 0;
	while(1) {
		if(*src == '\0') {
			goto finish;
		}
		pch = ch;
		while(1) {
			if(*pch == '\0') {
				break;
			}
			if(*src == *pch) {
				goto finish;
			}
			pch++;
		}
		if(k<len) {
			*des = *src;
			des++;
		}
		++k;
		src++;
	}
finish:	
	i = k;
	while(i<len) {
		*des++ = '\0';
		++i;
	}
	return k;
}

void digital_write(unsigned int io_num_base,char * binval,unsigned int num)
{
	unsigned int i;
	for(i=0;i<num;i++) {
		set_bitval(io_num_base+i,(binval[i]=='1')?1:0);
	}
}

void digital_read(unsigned int io_num_base,char * binval,unsigned int num)
{
	unsigned int i;
	for(i=0;i<num;i++) {
		binval[i] = get_bitval(io_num_base+i)?'1':'0';
	}
	binval[i] = 0;
}


unsigned int CmdRxPrase(void * pdat,unsigned int len)
{
	CmdHead * rcmd = (CmdHead * )pdat;
	if(len == 0) {
		return 0;
	}
	if(rcmd->cmd < 82) {
    	if(len < sizeof(CmdHead)) { //小于规定的头大小
	    	return 0;
	    }
	}
	{
		char strbuf[64];
		char * pstr = (char *)pdat;
		if(StringMatchHead(pstr,"SCMD")) { //相等时
			//是字符串命令
			if(THISINFO)putrsUART((ROM char *)"SCMD Get it\r\n");
			GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),1);
			if(StringMatchHead(strbuf,"DIGW")) { //相等时
				unsigned int dig_base,dig_num;
				if(THISINFO)putrsUART((ROM char *)"DIGW Get it\r\n");
				GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),2);
				dig_base = atoi(strbuf);
				GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),3);
				dig_num = atoi(strbuf);
				GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),4);
				digital_write(dig_base,strbuf,dig_num);
				return len;
			} else if(StringMatchHead(strbuf,"DIGR")) {
				unsigned int dig_base,dig_num;
				if(THISINFO)putrsUART((ROM char *)"DIGW Get it\r\n");
				GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),2);
				dig_base = atoi(strbuf);
				GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),3);
				dig_num = atoi(strbuf);
				digital_read(dig_base,strbuf,dig_num);
				strcpy(pstr,"SCMD DIGR ");
				strcat(pstr,strbuf);
				return strlen(pstr);
			} else {
				strcpy(pstr,"SCMD ERROR!");
				return 11;
			}
		}
	}
	//满足我们的协议头大小
	switch(rcmd->cmd)
	{
	case CMD_READ_REGISTER:    return CmdReadRegister(rcmd,len);
	case CMD_WRITE_REGISTER:   return CmdWriteRegister(rcmd,len);
	case CMD_GET_IO_OUT_VALUE: return CmdGetIoOutValue(rcmd,len);
	case CMD_SET_IO_OUT_VALUE: return CmdSetIoOutValue(rcmd,len);
	case CMD_REV_IO_SOME_BIT:  return CmdRevertIoOutIndex(rcmd,len);
	case CMD_SET_IO_ONE_BIT:   return CmdSetClrVerIoOutOneBit(rcmd,len,0);
	case CMD_CLR_IO_ONE_BIT:   return CmdSetClrVerIoOutOneBit(rcmd,len,1);
	case CMD_REV_IO_ONE_BIT:   return CmdSetClrVerIoOutOneBit(rcmd,len,2);
	case CMD_GET_IO_IN_VALUE:  return CmdGetIoInValue(rcmd,len);

	case CMD_SET_IP_CONFIG:    return CmdDefaultAck(rcmd,len);
	case CMD_GET_IP_CONFIG:    return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_MODE_INDEX: return CmdDefaultAck(rcmd,len);
	case CMD_SET_INPUT_CTL_MODE_INDEX: return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_MODE:       return CmdDefaultAck(rcmd,len);
	case CMD_SET_INPUT_CTL_MODE:       return CmdDefaultAck(rcmd,len);
		/*日后加以修改*/
	case CMD_GET_IO_NAME:              return CmdDefaultAck(rcmd,len);
	case CMD_SET_IO_NAME:              return CmdDefaultAck(rcmd,len);
		/* 时间指令*/
	case CMD_GET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);
	case CMD_SET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);

	case CMD_SET_RTC_VALUE:            return CmdSetNewRtcValue(rcmd,len);
	case CMD_GET_RTC_VALUE:            return CmdGetRtcValue(rcmd,len);
	case CMD_SET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);

        /* 定时指令*/
	case CMD_SET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);
	case CMD_GET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);

	case CMD_SET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_GET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_SET_SYSTEM_RESET:   return CmdDefaultAck(rcmd,len);
	case CMD_SET_IO_OUT_POWERDOWN_HOLD: return CmdSetIoOutPownDownHold(rcmd,len);
	case CMD_GET_IO_OUT_POWERDOWN_HOLD: return CmdGetIoOutPownDownHold(rcmd,len);
	case CMD_PLC_READ:           return CmdPlcRead(rcmd,len);
	case CMD_PLC_WRITE:          return CmdPlcWrite(rcmd,len);
	default: 
		return CmdDefaultAck(rcmd,len);
	}
}
