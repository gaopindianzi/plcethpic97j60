#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp_cmd_prase_handle.h"
#include "relay_cmd_definition.h"
#include "serial_comm_packeter.h"


#define   THISINFO         0
#define   THISERROR        0

//#define  DEBUG_TCP_CLIENT

#define TCP_RX_MAX_LEN        RELAY_CMD_MAX_PACKET_LEN

void GetTcpRxHandle(TCP_SOCKET MySocket)
{
	BYTE RX_Buffer[TCP_RX_MAX_LEN];
	WORD wMaxPut, wMaxGet;
	WORD w = sizeof(RX_Buffer);
	wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
	if(wMaxGet == 0) {
		return ;
	}
	if(w >= wMaxGet) {
	    TCPGetArray(MySocket, &RX_Buffer[0], wMaxGet);

		wMaxGet = CmdRxPrase((void *)RX_Buffer,(unsigned int)wMaxGet); //解析和TCP包,返回一定长度的应答包，然后返回给客户端

    	wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space

	    if(wMaxGet > 0 && wMaxPut >= wMaxGet) {
			TCPPutArray(MySocket, RX_Buffer, wMaxGet);
			TCPFlush(MySocket);
		}
	}
}


void Tcp0CmdTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER0);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 2000, TCP_PURPOSE_CMD_SERVER0);
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

			GetTcpRxHandle(MySocket);

			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}

void Tcp1CmdTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER1);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 2000, TCP_PURPOSE_CMD_SERVER1);
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

			GetTcpRxHandle(MySocket);

			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}

void Tcp2CmdTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER2);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 2000, TCP_PURPOSE_CMD_SERVER2);
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


			GetTcpRxHandle(MySocket);

			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}

