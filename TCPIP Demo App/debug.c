#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*
ʹ����ּ��
�������Ҫ��������ַ�������������ַ���
����ַ����������
����ʵ�ָ�ʽ�����
�����������һ���ַ����ַ���
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
					DebugStringNum("IP��ַ��",50505);
				} else {
					break;
				}

				RUN_LED_IO = !RUN_LED_IO;


				if(debug_tx_count == 0) {
					break;
				}


				//��ʼ��UDP���е�����Ϣ�㲥���㲥�˿ں���0xdddd
			    MySocket = UDPOpen(0, NULL, DEBUG_PORT); //�漴�����ڲ��˿ںţ��㲥ģʽ����DEBUG_PORT�˿ڽ��й㲥
	            if(MySocket == INVALID_UDP_SOCKET)
		            break;
	
	            // ȷ��UDPд׼���õġ�
	            while(!UDPIsPutReady(MySocket));

				//һ����д��
				UDPPutArray((BYTE*)debug_tx,debug_tx_count);
				//UDPPutROMString((ROM BYTE*)"my udp debug test.\r\n");
				debug_tx_count = 0;
	            UDPFlush();
	
	            // �رգ��Ա������ط�����ʹ��
	            UDPClose(MySocket);
			}
			break;
		default:
			DebugSM = DEBUG_HOME;
			break;
	}
}
