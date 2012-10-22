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


void DebugStringNum(const char * str,unsigned int num,const char * tail)
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
	DebugString(tail);
}


void DebugStringHex(const char * str,unsigned long num,const char * tail)
{
	unsigned char buffer[20];
	unsigned char i;
	DebugString(str);
	i = 0;
	while(1) {
		char n = (char)(num  & 0x0F);
		if(n > 9) {
			buffer[i++] = n + 'A';
		} else {
			buffer[i++] = n + '0';
		}
		num >>= 4;
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
	DebugString(tail);
}


void DebugTask(void)
{
	static enum {
		DEBUG_HOME = 0,
		DEBUG_DNS_IP,
		DEBUG_DNS_WAIT,
		DEBUG_DNS_CLOSE,
		DEBUG_RUN
	} DebugSM = DEBUG_HOME;

	static UDP_SOCKET	MySocket;
	static TICK         StartTime;
	static IP_ADDR      hostip;

	switch(DebugSM)
	{
		case DEBUG_HOME:
			{
				DebugTaskInit();
				DebugSM = DEBUG_DNS_IP;
				RELAY_OUT_0 = 0;
			}
			break;
		case DEBUG_DNS_IP:
			{
				char buffer[32];
				RELAY_OUT_1 = 0;
				if(!DNSBeginUsage()) {
					break;
				}
				//strcpypgm2ram(buffer,(ROM BYTE*)"www.qq.com");
				//DNSResolve(buffer, DNS_TYPE_A);
				DNSResolveROM((ROM BYTE*)"www.qq.com", DNS_TYPE_A);
				DebugSM = DEBUG_DNS_WAIT;
			}
			break;
		case DEBUG_DNS_WAIT:
			{
				RELAY_OUT_2 = 0;
				if(!DNSIsResolved(&hostip)) {
					break;
				}
				DebugStringNum("hostip=",hostip.v[0],".");
				DebugStringNum("",hostip.v[1],".");
				DebugStringNum("",hostip.v[2],".");
				DebugStringNum("",hostip.v[3],",  ");
				DebugSM = DEBUG_DNS_CLOSE;
			}
			break;
		case DEBUG_DNS_CLOSE:
			{
				RELAY_OUT_3 = 0;
				if(!DNSEndUsage()) {
					break;
				}
				StartTime = TickGet();
				DebugSM = DEBUG_RUN;
			}
			break;
		case DEBUG_RUN:
			{
				RELAY_OUT_4 = 0;
				if(TickGet() - StartTime > TICK_SECOND) {
					StartTime = TickGet();
					//DebugPrintString("haha\r\n");
					DebugStringNum("IP��ַ��",50505,"");
					DebugStringHex("Ip��ַ�ǡ�����",0x333422,"\r\n");
				} else {
					break;
				}

				
				RELAY_OUT_5 = 0;

				DebugSM = DEBUG_DNS_IP;

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
