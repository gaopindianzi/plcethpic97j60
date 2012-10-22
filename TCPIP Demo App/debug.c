#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*
使用宗旨：
满足调试要求，输出简单字符串，输出复杂字符串
输出字符，输出数字
后期实现格式化输出
但首先是输出一个字符和字符串
*/

unsigned char debug_tx[255];
unsigned char debug_tx_count;
#define   DEBUG_PORT        50505


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

void DebugString(const char * ramstr)
{
	while(*ramstr) {
		DebugPrintChar(*ramstr++);
	}
}


void DebugStringNum(const char * str,unsigned int num)
{
	unsigned char buffer[20];
	unsigned char i;
	DebugString(str);
	i = 0;
	while(1) {
		char n = num  % 10;
		buffer[i++] = n + '0';
		num /= 10;
		if(num == 0){
			break;
		}
	}
	if(i) {
		while(1) {
		    i--;
		    DebugPrintChar(buffer[i]);
			if(i == 0) {
				break;
			}
		}
	}
	DebugString("\r\n");
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
					//DebugPrintString("haha\r\n");
					DebugStringNum("IP地址是",50505);
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
