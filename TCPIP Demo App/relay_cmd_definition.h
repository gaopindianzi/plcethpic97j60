#ifndef __RELAY_CMD_DEFINITION_H__
#define __RELAY_CMD_DEFINITION_H__

typedef   BYTE   u8_t;
typedef   BYTE   uint8_t;

typedef   WORD   u16_t;
typedef   WORD   uint16_t;

typedef   DWORD  u32_t;
typedef   DWORD  uint32_t;


//�ɹ����ʵ����ֻռһλ
#define   CMD_ACK_OK                  0x01
#define   CMD_ACK_KO                  0x00

#define  CMD_REQ_START             0
#define  CMD_REQ_NEXT              1
#define  CMD_REQ_DONE              2

#define  CMD_CURRENT_START        0
#define  CMD_CURRENT_DOING        1
#define  CMD_CURRENT_DONE         2

//extern uint8_t    command_state;

#define   CMD_GET_IO_OUT_VALUE           1
#define   CMD_GET_IO_IN_VALUE            2
#define   CMD_SET_IO_OUT_VALUE           3
#define   CMD_GET_IO_NAME                4
#define   CMD_SET_IO_NAME                5
#define   CMD_GET_TIMING_INFO            6
#define   CMD_SET_TIMING_INFO            7
#define   CMD_GET_RTC_VALUE              8
#define   CMD_SET_RTC_VALUE              9
#define   CMD_SET_TIMING_ON_MSK          10
#define   CMD_GET_TIMING_ON_MSK          11
#define   CMD_SET_INPUT_CTL_ON_MSK       12
#define   CMD_GET_INPUT_CTL_ON_MSK       13
#define   CMD_SET_HOST_ADDRESS           14
#define   CMD_GET_HOST_ADDRESS           15
#define   CMD_SET_SYSTEM_RESET           16
#define   CMD_REV_IO_SOME_BIT            36
#define   CMD_SET_IO_ONE_BIT             37 //
#define   CMD_CLR_IO_ONE_BIT             38 //
#define   CMD_REV_IO_ONE_BIT             39 //
#define   CMD_SET_IP_CONFIG              33
#define   CMD_GET_IP_CONFIG              34
#define   CMD_GET_INPUT_CTL_MODE_INDEX   40
#define   CMD_SET_INPUT_CTL_MODE_INDEX   41
#define   CMD_SET_INPUT_CTL_MODE         42
#define   CMD_GET_INPUT_CTL_MODE         43
//������ʱ���ӿ�
#define   CMD_READ_REGISTER              80
#define   CMD_WRITE_REGISTER             81



#if 0
typedef struct _CmdHead
{
  uint8_t  cmd;
  uint16_t cmd_index;
  uint8_t  cmd_option;
  uint16_t cmd_len;
  uint8_t  data_checksum;
} CmdHead;
#else
typedef struct _CmdHead
{
  uint8_t  cmd;
  uint16_t cmd_index;
  uint8_t  cmd_option;
#if 1  //mfc�汾
  uint8_t  pad;
  uint8_t  cmd_len;
#else  //qt�汾
  uint8_t  cmd_len;
  uint8_t  pad;
#endif
  uint8_t  data_checksum;
} CmdHead;
#endif

//2b 00 00 01 09 44 11    00 07 00 0a 00 00 00 00 00 

//��д�Ĵ�����ͨ�ýӿ�
typedef struct _CmdRegister
{
  uint8_t  reg_addr_hi;
  uint8_t  reg_addr_lo;
  uint8_t  reg_len_hi;
  uint8_t  reg_len_lo;
  uint8_t  reg_base;
} CmdRegister;

//��д�¿���ģʽר�����ݽṹ
typedef struct _CmdInputControl
{
	unsigned char index;
	unsigned char mode;
	unsigned char input_filter_time_hi;
	unsigned char input_filter_time_lo;
	unsigned char input_trig_front_time_hi;
	unsigned char input_trig_front_time_lo;
	unsigned char input_trig_after_time_hi;
	unsigned char input_trig_after_time_lo;
	unsigned char input_trig_io_number;
} CmdInputControl;

#define GET_CMD_DATA(pCmd)         ((void *)(((char *)pCmd)+sizeof(CmdHead)))
#define GET_CMD_OK(pcmd)           (((pcmd)->cmd_option)&CMD_ACK_OK)
#define SET_CMD_OK(pcmd,ok)        do{if(!ok){(pcmd)->cmd_option &= ~CMD_ACK_OK;}else{(pcmd)->cmd_option |= CMD_ACK_OK;}}while(0)
#define GET_CMD_STATE(pcmd)        ((((pcmd)->cmd_option)&0xF0)>>4)
#define SET_CMD_STATE(pcmd,mode)   do{(pcmd)->cmd_option &= ~0xF0;(pcmd)->cmd_option |= ((mode)&0xF)<<4; }while(0)

typedef struct _CmdIoOneBit
{
	uint8_t  io_bitnum[2];
} CmdIoOneBit;


typedef struct _CmdIoIndex
{
  uint8_t    io_msk[4];
} CmdIobitmap;

typedef struct _CmdMode
{
  uint8_t    mode;
} CmdMode;

typedef struct _CmdInputMode
{
  uint8_t    mode[8];
} CmdInputMode;

typedef struct _CmdInputModeIndex
{
	uint8_t    index;
    uint8_t    mode;
} CmdInputModeIndex;

#define INPUT_TRIGGER_FLIP_MODE          0x00  //������תģʽ
#define INPUT_SINGLE_TRIGGER_MODE        0x01  //������ģʽ
#define INPUT_TRIGGER_TO_OPEN_MODE       0x02  //������ͨģʽ
#define INPUT_TRIGGER_TO_OFF_MODE        0x03  //�����ر�ģʽ
#define INPUT_EDGE_TRIG_MODE             0x04  //���ش���ģʽ
#define INPUT_LEVEL_CTL_ON_MODE          0x05  //�����ƽ���ƿ�ģʽ
#define INPUT_LEVEL_CTL_OFF_MODE         0x06  //�����ƽ���ƹ�ģʽ
#define INPUT_TRIGGER_OFF_MODE           0x07  //���ƹر�

typedef struct _CmdIoValidMsk
{
	uint8_t   valid_msk[4];
	uint8_t   invalid_msk[4];
} CmdIoValidMsk;

typedef struct _CmdIpConfigData
{
  uint8_t   ipaddr[4];
  uint8_t   netmask[4];
  uint8_t   gateway[4];
  uint8_t   dns[4];
  uint16_t  port;
  uint16_t  webport;
} CmdIpConfigData;


//���ܹ���
typedef struct _CmdMcuId
{
  uint8_t  IdLength;
  uint8_t  IdValue[1];
} CmdMcuId;

typedef struct _CmdIoValue
{
  uint8_t    io_count;
  uint8_t    io_value[4];
} CmdIoValue;


typedef struct _CmdIoBit
{
  uint8_t    io_index;
  uint8_t    io_enable;
} CmdIoBit;

typedef struct _CmdBoardInfo
{
	uint8_t  version;
	uint8_t  option; //�Ƿ����
	uint8_t  board_name[20];
	uint8_t  password[20];
} CmdBoardInfo;

typedef struct _CmdIoName
{
	uint8_t   io_addr[2];
	uint8_t   io_name[20];
} CmdIoName;

//��ʱ�Ķ�дʱ������
//���嶨ʱʱ�����
typedef struct _time_type
{
	uint8_t    year;
	uint8_t    mon;
	uint8_t    day;
	uint8_t    hour;
	uint8_t    min;
	uint8_t    sec;
} time_type;
typedef struct _timing_node
{
	uint8_t    addr[2];  //��ʱ�˿ڵ�ַ���루��������ڵ�ַ��ΧΪ[0][0] ~ [0][16]������5·�������addr[0]=4,addr[1]=0
	uint16_t   option;   //ѡ���0λ:1��ʾ��ʱ��Ч��0��ʾ��Ч,��1λ:1��ʾСѭ��������0�رա���[8:11]λ��ʾѭ�����ͣ�����ֵ����
	//
	time_type  start_time;  //ѭ������ʱ�䣨��ȷ���룬�԰���ʵʱʱ��Ϊ׼��
	time_type  end_time;    //ѭ���ر�ʱ�䣨��ȷ���룬�԰���ʵʱʱ��Ϊ׼��
	//
	uint32_t   duty_cycle;  //Сѭ���д򿪼̵�����ʱ�����ڣ���ʹ��ͨʱ�䳤��,����Ϊ��λ����С10���룬���ʮ�죨49�죩
	uint32_t   period;     //Сѭ��һ�����ڵ�ʱ�䳤��,����Ϊ��λ����С10���룬���ʮ�죨49�죩
	//��¼״̬
	uint8_t    state;  //�ڲ�ʹ��,д0
} timing_node;
//��ʱ�ڵ�ĸ�����
typedef struct _timint_info
{
	uint16_t    time_max_count;
} timint_info;
//�����д��ʱ��ʱ��������
typedef struct _CmdTimingNode
{
	uint16_t    index;
	timing_node node;
} CmdTimingNode;
//��ʱֻ��ʱ�䵽��ʱ�򴥷�������ƣ�ֻ���ڿ���ص�˲�䷢������
//�����ʱ�ڵ��ͬһ·������ƣ������һ·������д����Ϊ˳�򣩶�ʱ�ڵ�Ĵ����źŻḲ��ǰ��ڵ㴥����״̬��
//��ʱʱ���趨������дtimint_info����
//Ȼ����������дCmdTimingNode���ֱ�����Ϊֹ.
//CmdTimingNode���ֶ�index�ķ�Χ��0-(time_max_count-1)
//д�Ĺ��̷������׶Σ�
//��һ��SendCommand(CMD_SET_TIMING_INFO,CMD_REQ_START,data),data���������ģ�����Ϊtimint_info
//�ڶ��׶Σ�SendCommand(CMD_SET_TIMING_INFO,CMD_REQ_NEXT,data),datas�����ģ�����ΪCmdTimingNode,����д��time_max_count������
//�����׶�:SendCommand(CMD_SET_TIMING_INFO,CMD_REQ_DONE,data);data�����ģ�����Ϊ�㣬֪ͨ��λ�����д����
//����CMD_REQ_START�Ǳ���ͷ��cmd_option�ĵ�[4:7]λ����������
//#define  CMD_REQ_START             0
//#define  CMD_REQ_NEXT              1
//#define  CMD_REQ_DONE              2
//Ӧ������Ӧ�ģ�Ҳ��cmd_option��������[4:7]λ��������һ�����״̬
//��C����������ʾ���£�
//#define GET_CMD_STATE(pcmd)        ((((pcmd)->cmd_option)&0xF0)>>4)
//#define SET_CMD_STATE(pcmd,mode)   do{(pcmd)->cmd_option &= ~0xF0;(pcmd)->cmd_option |= ((mode)&0xF)<<4; }while(0)
//-----------------
//����ʱ���д��ʱʱ���൱��Ҳ�������׶Σ�����״̬��ֻ���������ͬ���������ݳ���Ϊ����ѡ�
//������������:
//1,SendCommand(CMD_GET_TIMING_INFO,CMD_REQ_START,data);
//2,SendCommand(CMD_GET_TIMING_INFO,CMD_REQ_NEXT,data);
//3,SendCommand(CMD_GET_TIMING_INFO,CMD_REQ_DONE,data);
//ע�⣬��ʱ����Ӧ���������ݵĸ�ʽ��timing_node,������CmdTimingNode
//�����Ǹ��������Ķ���
//timing_node��option�ֶ�
#define  VALID       1
#define  INVALID     0
//state
#define  TIME_NO_COME    0
#define  TIME_DOING      1
#define  TIME_SUBDO_ON   2
#define  TIME_SUBDO_OFF  3
#define  TIME_OVER       4
//

//option
#define  CYCLE_YEAR      0
#define  CYCLE_MONTH     1
#define  CYCLE_DAY       2
#define  CYCLE_HOUR      3
#define  CYCLE_MINITH    4
#define  CYCLE_SECOND    5
#define  CYCLE_WEEK      6
#define  CYCLE_USER      7
#define  CYCLE_ONCE      8
//option
#define  TIME_VALID      0x01
#define  TIME_SUBCYCLE   0x02
#define  TIME_HOLD_IO    0x04
#define  TIME_DONE       0x08


//-------------------------------------------------------------
#define  SET_IO_TIME_VALID(ptime,valid)  \
	do{  \
	  if(valid) { (ptime)->option |=  TIME_VALID; } else { (ptime)->option &= ~TIME_VALID; }   \
	} while(0)
#define  GET_IO_TIME_VALID(ptime)   \
	     (((ptime)->option)&TIME_VALID)
//-------------------------------------------------------------
//-------------------------------------------------------------
#define  SET_IO_TIME_SUBCYCLEON(ptime,valid)  \
	do{  \
	  if(valid) { (ptime)->option |=  TIME_SUBCYCLE; } else { (ptime)->option &= ~TIME_SUBCYCLE; }   \
    } while(0)
#define  GET_IO_TIME_SUBCYCLEON(ptime)   \
	     ((((ptime)->option)&TIME_SUBCYCLE)?1:0)
//-------------------------------------------------------------
#define  SET_IO_TIME_HOLD_IO(ptime,valid)  \
	do{  \
	  if(valid) { (ptime)->option |=  TIME_HOLD_IO; } else { (ptime)->option &= ~TIME_HOLD_IO; }   \
    } while(0)
#define  GET_IO_TIME_HOLD_IO(ptime)   \
	     (((ptime)->option)&TIME_MS_TICK)
//-------------------------------------------------------------
//-------------------------------------------------------------
#define  SET_IO_TIME_CYCLE_TYPE(ptime,type)  \
	do{ (ptime)->option &= ~(0xF<<8);(ptime)->option |= (((type)&0xF)<<8); } while(0)
#define  GET_IO_TIME_CYCLE_TYPE(ptime)   \
	     ((((ptime)->option)>>8)&0xF)
//-------------------------------------------------------------
#define  SET_IO_TIME_IO_ON_OFF(ptime,valid)  \
	do{  \
	  if(valid) { (ptime)->option |=  TIME_ON_OFF; } else { (ptime)->option &= ~TIME_ON_OFF; }   \
    } while(0)
#define  GET_IO_TIME_IO_ON_OFF(ptime)   \
	     (((ptime)->option)&TIME_ON_OFF)
//-------------------------------------------------------------
#define  SET_IO_TIME_DONE(ptime,valid)  \
	do{  \
	  if(valid) { (ptime)->option |=  TIME_DONE; } else { (ptime)->option &= ~TIME_DONE; }   \
    } while(0)
#define  GET_IO_TIME_DONE(ptime)   \
	     (((ptime)->option)&TIME_DONE)


typedef struct _host_address
{
	u8_t     type;  //type = 0:192.168.1.111....,type = 1 : www.hostaddr.com
	char     host_address[64];
	u16_t    port;
} host_address;

#define  HOST_ADDR_IS_IP_MODE       0
#define  HOST_ADDR_IS_DOMAIN_MODE   1
//
#define  HOST_IS_UDP                0
#define  HOST_IS_TCP_SERVER         1
#define  HOST_IS_TCP_CLIENT         2



#define RELAY_CMD_MAX_PACKET_LEN      64



#endif
