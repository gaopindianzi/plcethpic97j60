#ifndef __DEBUG_H__
#define __DEBUG_H__


#define DEBUG_ON            0

#define DEBUG_INFO          0
#define DEBUG_WARN          0
#define DEBUG_ERROR         0
#define ASSERT_ON           0


extern void DebugString(const char * ramstr);
extern void DebugStringNum(const char * str,unsigned int num);
extern void DebugTask(void);
extern void ResetTask(void);
extern void TaskLedFlash(void);
extern void set_led_flash(unsigned int on_time,unsigned int off_time,unsigned int times);


void dumpstrhex(ROM char *str,unsigned char * buffer,unsigned int len);
void PrintStringNum(ROM char *str,unsigned int num);

//extern void DebugTcpTask(void);
extern void DiscoverTask(void);


#define  DEBUGINFO(str)    if(THISINFO)do{ DebugString##str; }while(0)


#endif
