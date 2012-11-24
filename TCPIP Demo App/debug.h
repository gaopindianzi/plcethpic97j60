#ifndef __DEBUG_H__
#define __DEBUG_H__


#define DEBUG_ON            1

#define DEBUG_INFO          1
#define DEBUG_WARN          1
#define DEBUG_ERROR         1
#define ASSERT_ON           1


extern void DebugString(const char * ramstr);
extern void DebugStringNum(const char * str,unsigned int num);
extern void DebugTask(void);
extern void ResetTask(void);
extern void TaskLedFlash(void);
extern void set_led_flash(unsigned int on_time,unsigned int off_time,unsigned int times);


//extern void DebugTcpTask(void);
extern void DiscoverTask(void);


#define  DEBUGINFO(str)    if(THISINFO)do{ DebugString##str; }while(0)


#endif
