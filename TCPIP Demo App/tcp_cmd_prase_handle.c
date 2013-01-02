/************************************************************
 * ����ļ�ֻ����Э�飬������ʲô���յ������ݣ��������ﴦ��
 * ������ϣ�����һ�����ȵ����ݣ�Ȼ��ײ���ݲ�ͬ�Ľӿڣ���
 * ��������һ�����ȵ����ݡ�
 * ��ӿ��޹�
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
 * ���ܣ�����У��ֵ
 * ���룺
 *     buffer  :  ���������
 *     len     :  ���ڵĳ��ȣ��ֽڵ�λ
 * �����
 * ����ֵ��
 *     ���һ���ֽڣ�����У���
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
 * ���ܣ���תĳЩ�̵���
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ�����ĳЩ�̵���
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
	io->io_count = io_out_get_bits(0,io->io_value,32); //32���̵��������ָ��̶�֧��32λ����������
	cmd->cmd_option    = CMD_ACK_OK;
    cmd->cmd_len       = sizeof(CmdIoValue);
    cmd->data_checksum = 0;
error:
    return sizeof(CmdHead)+sizeof(CmdIoValue);
}

/*************************************************************
 * ���ܣ���ĳЩ�̵�����״̬
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ����û������תһ��IO�ڵ����
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 *    setmode    :  ����ģʽ��0��ʾ���ã�1��ʾ���㣬2��ʾ��ת
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ���ȡ������������״̬
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ�һЩ��֧�ֵ�ָ�ֱ�ӵ���Ĭ�ϵĺ������ؼ���
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
 */
unsigned int CmdDefaultAck(CmdHead * cmd,unsigned int len)
{
	cmd->cmd_option    = CMD_ACK_KO;
    cmd->cmd_len       = 0;
    cmd->data_checksum = 0;
	return sizeof(CmdHead);
}
/*************************************************************
 * ���ܣ�д�Ĵ�����ʵ��ͨ�ýӿ�
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
			//��DS1302�Ĵ���
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
		//����д���ڴ�
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
 * ���ܣ����Ĵ�����ʵ��ͨ�ýӿ�
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
			//��DS1302�Ĵ���
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
		//���������¶������϶�
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
		//����д���ڴ�
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
 * ���ܣ�����ʵʱʱ��
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ���ȡʵʱʱ��
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ����ü̵����ϵ籣��
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ���ȡ�̵����ϵ籣��
 * ���룺
 *     cmd       :  ����Э������ͷָ�룬���ݽ������
 *     len       :  ���յ���ָ�������ܳ��ȣ�û�о�����֤�����Ƿ���Ч
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
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
 * ���ܣ�PLC�����
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
		//�������������
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
	    //�����ݷ���
	    return sizeof(plc_rw)+datlen;
	} else {
		//��Ч��ָ��
		return 0;
	}

}

/*************************************************************
 * ���ܣ�PLC����д
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
		//�������������
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
	    //�����ݷ���
	    return sizeof(plc_rw)+datlen;
	} else {
		//��Ч��ָ��
		return 0;
	}
}
/*************************************************************
 * ���ܣ������������
 *       ������TCP��,����һ�����ȵ�Ӧ�����Ȼ�󷵻غͷ��͸��ͻ���
 * ���룺
 *     pdat      :  ����ĵ�ǰsocket����
 *     len       :  ���յ���ָ�������ܳ���
 * �����
 *     cmd       :  ���������Ҳ������cmdָ��ĳ������棬���������
 *                  ����Ԥ���Ļ�����(����Ļ��������С��RELAY_CMD_MAX_PACKET_LEN)
 * ����ֵ��
 *     ���һ�����Σ�����������ݵĳ��ȣ��������Ϊ0�����ʾû�������
 *     ������ȴ���0�����ʾ�������������ײ����ݱ��뷵�ش˺������
 *     �����ݡ�
 * ����ַ�������
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
    	if(len < sizeof(CmdHead)) { //С�ڹ涨��ͷ��С
	    	return 0;
	    }
	}
	{
		char strbuf[64];
		char * pstr = (char *)pdat;
		if(StringMatchHead(pstr,"SCMD")) { //���ʱ
			//���ַ�������
			if(THISINFO)putrsUART((ROM char *)"SCMD Get it\r\n");
			GetStringDiviseBy(pstr," ",strbuf,sizeof(strbuf),1);
			if(StringMatchHead(strbuf,"DIGW")) { //���ʱ
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
	//�������ǵ�Э��ͷ��С
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
		/*�պ�����޸�*/
	case CMD_GET_IO_NAME:              return CmdDefaultAck(rcmd,len);
	case CMD_SET_IO_NAME:              return CmdDefaultAck(rcmd,len);
		/* ʱ��ָ��*/
	case CMD_GET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);
	case CMD_SET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);

	case CMD_SET_RTC_VALUE:            return CmdSetNewRtcValue(rcmd,len);
	case CMD_GET_RTC_VALUE:            return CmdGetRtcValue(rcmd,len);
	case CMD_SET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);

        /* ��ʱָ��*/
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
