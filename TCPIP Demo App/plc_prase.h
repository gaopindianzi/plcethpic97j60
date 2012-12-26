#ifndef __PLC_PRASE_H__
#define __PLC_PRASE_H__

//--------------------------��ַ���-------����-----------
#define  IO_INPUT_BASE          0
#define  IO_INPUT_COUNT                     9
#define  IO_OUTPUT_BASE         256  //0x01,0x00
#define  IO_OUTPUT_COUNT                    7
#define  AUXI_RELAY_BASE        512  //0x02,0x00
#define  AUXI_RELAY_COUNT                   100
#define  AUXI_HOLDRELAY_BASE    1024 //0x04,0x00
#define  AUXI_HOLDRELAY_COUNT               120
//
#define  TIMING100MS_EVENT_BASE  2048  //0x08,0x00
#define  TIMING100MS_EVENT_COUNT            24

#define  TIMING1S_EVENT_BASE     3072  //0x0C,0x00
#define  TIMING1S_EVENT_COUNT               24

#define  COUNTER_EVENT_BASE      4096  //0x10,0x00
#define  COUNTER_EVENT_COUNT                24
//�ֽڱ���
#define  REG_BASE                0
#define  REG_COUNT                          128
//ʱ���ַ
#define  REG_RTC_BASE            4096
#define  REG_RTC_COUNT                      7    //�����գ�ʱ���룬����
#define  REG_TEMP_BASE           4103
#define  REG_TEMP_COUNT                     2    //�¶ȸߵ�λ

//http://192.168.1.223/mpfsupload

extern void PlcInit(void);
extern void PlcProcess(void);
extern void plc_timing_tick_process(void);
extern unsigned char plc_write_delay(void);
extern void plc_set_busy(unsigned char busy);
extern void set_bitval(unsigned int index,unsigned char bitval);
extern unsigned char get_bitval(unsigned int index);

extern void plc_code_test_init(void);

extern void sys_lock(void);
extern void sys_unlock(void);


#endif