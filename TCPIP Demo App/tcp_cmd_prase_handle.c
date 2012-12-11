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

#include "tcp_cmd_prase_handle.h"
#include "relay_cmd_definition.h"
#include "hal_io_interface.h"
#include "DS1302.h"
#include "DS18B20.h"

#define THISINFO           0
#define THISERROR          0


uint8_t    command_state;



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
	} else if(setmode == 1) {
		reg = 0x00;
	    io_out_set_bits(num,&reg,1);
	} else if(setmode == 2) {
		reg = 0x01;
		io_out_convert_bits(num,&reg,1);
	}
	sio->io_count = io_out_get_bits(0,sio->io_value,32);
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
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
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
error:
    cmd->data_checksum = 0;
	return sizeof(CmdHead) + cmd->cmd_len;
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
 */

unsigned int CmdRxPrase(void * pdat,unsigned int len)
{
	CmdHead * rcmd = (CmdHead * )pdat;
	if(len < sizeof(CmdHead)) { //小于规定的头大小
		return 0;
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
	default: 
		return CmdDefaultAck(rcmd,len);
	}
}
