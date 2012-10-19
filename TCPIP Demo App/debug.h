#ifndef __DEBUG_H__
#define __DEBUG_H__


#define DEBUG_ON            1

#define DEBUG_INFO          1
#define DEBUG_WARN          1
#define DEBUG_ERROR         1
#define ASSERT_ON           1


extern void DebugPrintString(const char * ramstr);
extern void DebugTask(void);


#endif
