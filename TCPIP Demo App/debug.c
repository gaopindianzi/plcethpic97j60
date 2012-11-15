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
				RELAY_OUT_5 = 1;
				RELAY_OUT_4 = 1;
				RELAY_OUT_3 = 1;
				RELAY_OUT_2 = 1;
				RELAY_OUT_1 = 0;
				if(!DNSBeginUsage()) {
					break;
				}
				StartTime = TickGet();
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
					if((TickGet() - StartTime) > TICK_SECOND*10) {
						DebugSM = DEBUG_DNS_IP;
					}
					break;
				}
				
				DebugSM = DEBUG_RUN;
			}
			break;
		case DEBUG_RUN:
			{
				RELAY_OUT_4 = 0;
				if(TickGet() - StartTime > TICK_SECOND) {
					StartTime = TickGet();
					//DebugPrintString("haha\r\n");
					DebugStringNum("IP地址是",50505,"");
					DebugStringHex("Ip地址是。。。",0x333422,"\r\n");
				} else {
					break;
				}

				
				RELAY_OUT_5 = 0;

				DebugSM = DEBUG_DNS_IP;

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

				DebugSM = DEBUG_DNS_IP;
			}
			break;
		default:
			DebugSM = DEBUG_HOME;
			break;
	}
}






//#define  DEBUG_TCP_CLIENT

void DebugTcpTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;
	static BYTE RX_Buffer[200];
	WORD wMaxPut, wMaxGet;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 2000, TCP_PURPOSE_CMD_SERVER);
			#endif			
			// Abort operation if no TCP socket of type TCP_PURPOSE_UART_2_TCP_BRIDGE is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)
				break;

			// Eat the first TCPWasReset() response so we don't 
			// infinitely create and reset/destroy client mode sockets
			TCPWasReset(MySocket);

			// We have a socket now, advance to the next state
			BridgeState = SM_SOCKET_OBTAINED;
			break;

		case SM_SOCKET_OBTAINED:
			if(TCPWasReset(MySocket))
			{
				// Optionally discard anything in the UART FIFOs
				//RXHeadPtr = vUARTRXFIFO;
				//RXTailPtr = vUARTRXFIFO;
				//TXHeadPtr = vUARTTXFIFO;
				//TXTailPtr = vUARTTXFIFO;
				
				// If we were a client socket, close the socket and attempt to reconnect
				#if defined(DEBUG_TCP_CLIENT)
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					BridgeState = SM_HOME;
					break;
				#endif
			}
			// Don't do anything if nobody is connected to us
			if(!TCPIsConnected(MySocket))
			{
				break;
			}
			while(1)
			{

				WORD w = sizeof(RX_Buffer);
				wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count

				if(w >= wMaxGet) {

				    TCPGetArray(MySocket, &RX_Buffer[0], wMaxGet);

					wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space

					if(wMaxPut >= wMaxGet) {
						TCPPutArray(MySocket, RX_Buffer, wMaxGet);
					} else {
					}
					TCPFlush(MySocket);
					break;

				} else {
					break;
				}

			}
			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}
