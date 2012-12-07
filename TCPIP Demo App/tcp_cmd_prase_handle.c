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


#define THISINFO           1
#define THISERROR          1


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
	case CMD_READ_REGISTER:  return CmdDefaultAck(rcmd,len);
	case CMD_WRITE_REGISTER: return CmdDefaultAck(rcmd,len);

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

	case CMD_SET_RTC_VALUE:            return CmdDefaultAck(rcmd,len);
	case CMD_GET_RTC_VALUE:            return CmdDefaultAck(rcmd,len);
	case CMD_SET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);

        /* 定时指令*/
	case CMD_SET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);
	case CMD_GET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);

	case CMD_SET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_GET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_SET_SYSTEM_RESET:   return CmdDefaultAck(rcmd,len);
	default: 
		return CmdDefaultAck(rcmd,len);
	}
}
