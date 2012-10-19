#include "TCPIP Stack/TCPIP.h"

/*
使用宗旨：
满足调试要求，输出简单字符串，输出复杂字符串
输出字符，输出数字
后期实现格式化输出
但首先是输出一个字符和字符串
*/

unsigned char debug_tx[255];
unsigned char debug_tx_count;
#define   DEBUG_PORT        30304



void DebugTaskInit(void)
{
    debug_tx_count = 0;
}

void DebugPrintChar(char ch)
{
    if(debug_tx_count < sizeof(debug_tx)) {
        debug_tx[debug_tx_count++] = ch;
    }
}

void DebugPrintString(const char * ramstr)
{
	while(*ramstr) {
		DebugPrintChar(*ramstr++);
	}
}


void DebugTask(void)
{
	static enum {
		DEBUG_HOME = 0,
		DEBUG_RUN
	} DebugSM = DEBUG_HOME;

	static UDP_SOCKET	MySocket;
	static TICK         StartTime;


	switch(DebugSM)
	{
		case DEBUG_HOME:
			{
				DebugTaskInit();
				DebugSM = DEBUG_RUN;
				StartTime = TickGet();
			}
			break;
		case DEBUG_RUN:
			{
				if(TickGet() - StartTime > TICK_SECOND) {
					StartTime = TickGet();
					DebugPrintString("haha\r\n");
				} else {
					break;
				}

				RUN_LED_IO = !RUN_LED_IO;


				if(debug_tx_count == 0) {
					break;
				}


				


				//开始用UDP进行调试信息广播，广播端口号是0xdddd
			    MySocket = UDPOpen(0, NULL, DEBUG_PORT); //随即分配内部端口号，广播模式，对DEBUG_PORT端口进行广播
	            if(MySocket == INVALID_UDP_SOCKET)
		            break;
	
	            // 确定UDP写准备好的。
	            while(!UDPIsPutReady(MySocket));

				//一次性写完
				UDPPutArray((BYTE*)debug_tx,debug_tx_count);
				//UDPPutROMString((ROM BYTE*)"my udp debug test.\r\n");
				debug_tx_count = 0;
	            UDPFlush();
	
	            // 关闭，以便其他地方可以使用
	            UDPClose(MySocket);
			}
			break;
		default:
			DebugSM = DEBUG_HOME;
			break;
	}
}
