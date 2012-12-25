#ifndef __PLC_PRASE_H__
#define __PLC_PRASE_H__

//--------------------------µØÖ·±àºÅ-------ÊýÁ¿-----------
#define  IO_INPUT_BASE          0
#define  IO_INPUT_COUNT                     9
#define  IO_OUTPUT_BASE         256  //0x00,0x01
#define  IO_OUTPUT_COUNT                    7
#define  AUXI_RELAY_BASE        512  //0x00,0x02
#define  AUXI_RELAY_COUNT                   100
//
#define  TIMING100MS_EVENT_BASE  2048  //0x08,0x00
#define  TIMING100MS_EVENT_COUNT            24

#define  TIMING1S_EVENT_BASE     3072  //0x0C,0x00
#define  TIMING1S_EVENT_COUNT               24

#define  COUNTER_EVENT_BASE      4096  //0x10,0x00
#define  COUNTER_EVENT_COUNT                24


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