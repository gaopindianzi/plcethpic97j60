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

#include "tcp_cmd_prase_handle.h"
#include "relay_cmd_definition.h"
#include "hal_io_interface.h"


#define THISINFO           1
#define THISERROR          1


uint8_t    command_state;



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
 */

unsigned int CmdRxPrase(void * pdat,unsigned int len)
{
	CmdHead * rcmd = (CmdHead * )pdat;
	if(len < sizeof(CmdHead)) { //С�ڹ涨��ͷ��С
		return 0;
	}
	//�������ǵ�Э��ͷ��С
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
		/*�պ�����޸�*/
	case CMD_GET_IO_NAME:              return CmdDefaultAck(rcmd,len);
	case CMD_SET_IO_NAME:              return CmdDefaultAck(rcmd,len);
		/* ʱ��ָ��*/
	case CMD_GET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);
	case CMD_SET_TIMING_INFO:          return CmdDefaultAck(rcmd,len);

	case CMD_SET_RTC_VALUE:            return CmdDefaultAck(rcmd,len);
	case CMD_GET_RTC_VALUE:            return CmdDefaultAck(rcmd,len);
	case CMD_SET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);
	case CMD_GET_INPUT_CTL_ON_MSK:     return CmdDefaultAck(rcmd,len);

        /* ��ʱָ��*/
	case CMD_SET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);
	case CMD_GET_TIMING_ON_MSK:  return CmdDefaultAck(rcmd,len);

	case CMD_SET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_GET_HOST_ADDRESS:   return CmdDefaultAck(rcmd,len);
	case CMD_SET_SYSTEM_RESET:   return CmdDefaultAck(rcmd,len);
	default: 
		return CmdDefaultAck(rcmd,len);
	}
}
