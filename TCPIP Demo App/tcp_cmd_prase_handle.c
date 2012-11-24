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






uint8_t    command_state;





/*************************************************************
 * ���ܣ���תĳЩ�̵���
 * ���룺
 *     MySocket  :  TCP socket
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
#if 0
	int rc = -1;
	const uint8_t outsize = sizeof(CmdIobitmap);
	uint8_t  buffer[sizeof(CmdHead)+outsize];
    CmdHead          * tcmd  = (CmdHead *)buffer;
    CmdIobitmap      *  sio  = (CmdIobitmap *)GET_CMD_DATA(tcmd);
    CmdIobitmap      *   io  = (CmdIobitmap *)GET_CMD_DATA(cmd);
    //
    if(datasize < sizeof(CmdIobitmap)) {
      if(THISERROR)printf("ERROR:Cmd CmdSetIoOutBit Datasize ERROR\r\n");
	  tcmd->cmd_option    = CMD_ACK_KO;
      goto error;
    }
    //
    tcmd->cmd_option    = CMD_ACK_OK;	
	//rc = _ioctl(_fileno(sys_varient.iofile), IO_SIG_BITMAP, io->io_msk);
	//rc = _ioctl(_fileno(sys_varient.iofile), IO_OUT_GET, sio->io_msk);
	io_out_convert_bits(0,io->io_msk,32);
	io_out_get_bits(0,sio->io_msk,32);
	rc = 0;

error:
    tcmd->cmd           = CMD_REV_IO_SOME_BIT;
    tcmd->cmd_index     = cmd->cmd_index;
    tcmd->cmd_len       = outsize;
    tcmd->data_checksum = CalCheckSum(sio,outsize);
    //
    NutTcpSend(sock,tcmd,(int)(sizeof(CmdHead)+outsize));
    if(THISINFO)printf("CmdSetIoOutBit()!\r\n");
    return rc;
#endif
	return len;
}
/*************************************************************
 * ���ܣ���ĳЩ�̵�����״̬
 * ���룺
 *     MySocket  :  TCP socket
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
#if 0
	int rc = -1;
	const uint8_t outsize = sizeof(CmdIoValue);
	uint8_t  buffer[sizeof(CmdHead)+outsize];
	CmdHead       * tcmd  = (CmdHead *)buffer;
	CmdIoValue    *  sio  = (CmdIoValue *)GET_CMD_DATA(tcmd);
	//
	//uint32_t tmp;
    datasize = datasize;
    //
    //sio->io_count    = 32;
    //sio->io_value[0] = (uint8_t)((relay_msk >> 0) & 0xFF);
    //sio->io_value[1] = (uint8_t)((relay_msk >> 8) & 0xFF);;
    //sio->io_value[2] = (uint8_t)((relay_msk >> 16) & 0xFF);;
    //sio->io_value[3] = (uint8_t)((relay_msk >> 24) & 0xFF);;
	//rc = _ioctl(_fileno(sys_varient.iofile), GET_OUT_NUM, &tmp);
	//sio->io_count = (unsigned char)tmp;
	//rc = _ioctl(_fileno(sys_varient.iofile), IO_OUT_GET, sio->io_value);
	//
	sio->io_count = io_out_get_bits(0,sio->io_value,32);
	rc = 0;
    //
    tcmd->cmd_option    = CMD_ACK_OK;
    tcmd->cmd           = CMD_GET_IO_OUT_VALUE;
    tcmd->cmd_index     = cmd->cmd_index;
    tcmd->cmd_len       = outsize;
    tcmd->data_checksum = CalCheckSum(sio,outsize);
    
    NutTcpSend(sock,tcmd,(int)(sizeof(CmdHead)+outsize));
    if(THISINFO)printf("CmdGetIoOutValue OK\r\n");
    return rc;
#endif
	return len;
}


typedef unsigned int      (*CmdCallFunc)(CmdHead * cmd,unsigned int len);
typedef struct _CmdCallArryMap
{
  unsigned char       cmd_id;
  CmdCallFunc         CmdCall;
} CmdCallArryMap;

//CmdCallFunc abc = CmdRevertIoOutIndex;

#if 0
ROM CmdCallArryMap cell_may_array[] = 
{
	//{CMD_READ_REGISTER,CmdReadRegister},
	//{CMD_WRITE_REGISTER,CmdWriteRegister},
//	{CMD_GET_IO_OUT_VALUE,CmdGetIoOutValue},
	//{CMD_SET_IO_OUT_VALUE,CmdSetIoOutValue},
	//{CMD_REV_IO_SOME_BIT,(CmdCallFunc)CmdRevertIoOutIndex},
	//{CMD_SET_IO_ONE_BIT,CmdSetIoOutOneBit},
	//{CMD_CLR_IO_ONE_BIT,CmdClrIoOutOneBit},
	//{CMD_REV_IO_ONE_BIT,CmdRevIoOutOneBit},
	//{CMD_GET_IO_IN_VALUE,CmdGetIoInValue},
    //{CMD_SET_IP_CONFIG,CmdSetIpConfig},
    //{CMD_GET_IP_CONFIG,CmdGetIpConfig},
    //{CMD_GET_INPUT_CTL_MODE_INDEX,CmdGetInputCtlModeIndex},
    //{CMD_SET_INPUT_CTL_MODE_INDEX,CmdSetInputCtlModeIndex},
	//{CMD_GET_INPUT_CTL_MODE,CmdReadInputControl},
	//{CMD_SET_INPUT_CTL_MODE,CmdWriteInputControl},
	//{CMD_GET_IO_NAME,CmdGetIoName},
	//{CMD_SET_IO_NAME,CmdSetIoName},
#ifdef APP_TIMEIMG_ON
	//{CMD_GET_TIMING_INFO,CmdGetIoTimingInfo},
	//{CMD_SET_TIMING_INFO,CmdSetIoTimingInfo},
#endif
	//{CMD_SET_RTC_VALUE,CmdSetNewRtcValue},
	//{CMD_GET_RTC_VALUE,CmdGetRtcValue},
	//{CMD_SET_INPUT_CTL_ON_MSK,CmdSetInputValidMsk},
	//{CMD_GET_INPUT_CTL_ON_MSK,CmdGetInputValidMsk},
#ifdef APP_TIMEIMG_ON
	//{CMD_SET_TIMING_ON_MSK,CmdSetTimingValidMsk},
	//{CMD_GET_TIMING_ON_MSK,CmdGetTimingValidMsk},
#endif
	//{CMD_SET_HOST_ADDRESS,CmdSetRemoteHostAddress},
	//{CMD_GET_HOST_ADDRESS,CmdGetRemoteHostAddress},
	//{CMD_SET_SYSTEM_RESET,CmdSetSystemReset},
	{0,NULL}
};

#endif



unsigned int CmdRxPrase(void * pdat,unsigned int len)
{
	return len / 2;
}



