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

		wMaxGet = CmdRxPrase((void *)RX_Buffer,(unsigned int)wMaxGet); //������TCP��,����һ�����ȵ�Ӧ�����Ȼ�󷵻ظ��ͻ���

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
	WORD wMaxPut, wMaxGet;
	unsigned char buffer[PACK_MAX_RX_SIZE];
	DATA_RX_PACKET_T * prx;
	DATA_TX_PACKET_T * ptx;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //�ͻ���
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER0);
			#else //������
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


			prx =  GetFinishedPacket();
			if(prx != NULL) {
				if(prx->finished) {
					prx->look_up_times++;
					if(prx->index > 3) {
						unsigned int port = prx->buffer[0];
						port <<= 8; port |= prx->buffer[1];
						if(port == 2000 && prx->buffer[2] == 0) {
							port = prx->index - 3;
							wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
							if(wMaxPut >= port) {
								//һ���Է������
								TCPPutArray(MySocket, &(prx->buffer[3]), port);
								TCPFlush(MySocket);
							} else {
								//�����������˰�����������
								if(THISERROR)putrsUART((ROM char*)"\r\rx is too large,,throw it. 0");
							}
							prx->finished = 0;
						}
					} else {
						//û���ж�
						if(THISERROR)putrsUART((ROM char*)"\r\0:prx->index <= 3 error");
					}
					if(THISINFO)putrsUART((ROM char *)"\r\ntcp 0 look at it");
					//prx->finished = 0;  //������ 
				}
			}
			PIE1bits.RCIE = 1;
			



			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
			if(wMaxGet > 0) {
				DATA_TX_PACKET_T * ptx = find_next_empty_tx_buffer();
				if(ptx != NULL) {
					wMaxGet = (wMaxGet > (PACK_MAX_RX_SIZE-3))?(PACK_MAX_RX_SIZE-3):wMaxGet;
				    TCPGetArray(MySocket,(BYTE *)&buffer[3],wMaxGet);
				    //��˽������
				    buffer[0] = 2000 >> 8;
				    buffer[1] = 2000 & 0xFF;
				    buffer[2] = 0;
				    ptx = prase_in_buffer(buffer,wMaxGet+3);
				    if(ptx != NULL) {
					    if(ptx->index > 0) {
						    //��������
						    PIE1bits.TXIE = 1;
					    }
				    }
				}
			} else {
				//�յ�������̫���ˣ�������,�Լ���PLC�ļ�����жϹ�����
				//���SOCKET�����ڴ�
				TCPDiscard(MySocket);
			}


			//GetTcpRxHandle(MySocket);

			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
	//���ʲô��û����Ҳ����֪ͨ�����߸ò����ͷ�
	prx =  GetFinishedPacket();
	if(prx != NULL) {
		if(prx->finished) {
			prx->look_up_times++;
		}
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
	WORD wMaxPut, wMaxGet;
	unsigned char buffer[PACK_MAX_RX_SIZE];
	DATA_RX_PACKET_T * prx;
	DATA_TX_PACKET_T * ptx;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //�ͻ���
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER1);
			#else //������
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
			//GetTcpRxHandle(MySocket);





			prx =  GetFinishedPacket();
			if(prx != NULL) {
				if(prx->finished) {
					if(prx->index > 3) {
						unsigned int port = prx->buffer[0];
						port <<= 8; port |= prx->buffer[1];
						if(port == 2000 && prx->buffer[2] == 1) {
							port = prx->index - 3;
							wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
							if(wMaxPut >= port) {
								//һ���Է������
								TCPPutArray(MySocket, &(prx->buffer[3]), port);
								TCPFlush(MySocket);
							} else {
								//�����������˰�����������
								if(THISERROR)putrsUART((ROM char*)"\r\rx is too large,,throw it. 1");
							}
							prx->finished = 0;
						}
					} else {
						//û���ж�
						if(THISERROR)putrsUART((ROM char*)"\r\1:prx->index <= 3 error");
					}
					prx->look_up_times++;
					//prx->finished = 0;  //������ 
					if(THISINFO)putrsUART((ROM char *)"\r\ntcp 1 look at it");
				}
			}
			PIE1bits.RCIE = 1;
			



			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
			if(wMaxGet > 0 && wMaxGet <= (PACK_MAX_RX_SIZE-3)) {
				DATA_TX_PACKET_T * ptx = find_next_empty_tx_buffer();
				if(ptx != NULL) {
				    TCPGetArray(MySocket,(BYTE *)&buffer[3],wMaxGet);
				    //��˽������
				    buffer[0] = 2000 >> 8;
				    buffer[1] = 2000 & 0xFF;
				    buffer[2] = 1;
				    ptx = prase_in_buffer(buffer,wMaxGet+3);
				    if(ptx != NULL) {
					    if(ptx->index > 0) {
						    //��������
						    PIE1bits.TXIE = 1;
					    }
				    }
				}
			} else {
				//�յ�������̫���ˣ�������,�Լ���PLC�ļ�����жϹ�����
				//���SOCKET�����ڴ�
				TCPDiscard(MySocket);
			}




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
	WORD wMaxPut, wMaxGet;
	unsigned char buffer[PACK_MAX_RX_SIZE];
	DATA_RX_PACKET_T * prx;
	DATA_TX_PACKET_T * ptx;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //�ͻ���
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER2);
			#else //������
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
			///GetTcpRxHandle(MySocket);





			prx =  GetFinishedPacket();
			if(prx != NULL) {
				if(prx->finished) {
					if(prx->index > 3) {
						unsigned int port = prx->buffer[0];
						port <<= 8; port |= prx->buffer[1];
						if(port == 2000 && prx->buffer[2] == 2) {
							port = prx->index - 3;
							wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
							if(wMaxPut >= port) {
								//һ���Է������
								TCPPutArray(MySocket, &(prx->buffer[3]), port);
								TCPFlush(MySocket);
							} else {
								//�����������˰�����������
								if(THISERROR)putrsUART((ROM char*)"\r\rx is too large,,throw it. 2");
							}
							prx->finished = 0;
						}
					} else {
						//û���ж�
						if(THISERROR)putrsUART((ROM char*)"\r\2 prx->index <= 3 error");
					}
					prx->look_up_times++;
					//prx->finished = 0;  //������ 
					if(THISINFO)putrsUART((ROM char *)"\r\ntcp 2 look at it");
				}
			}
			PIE1bits.RCIE = 1;
			



			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
			if(wMaxGet > 0 && wMaxGet <= (PACK_MAX_RX_SIZE-3)) {
				TCPGetArray(MySocket,(BYTE *)&buffer[3],wMaxGet);
				//��˽������
				buffer[0] = 2000 >> 8;
				buffer[1] = 2000 & 0xFF;
				buffer[2] = 2;
				ptx = prase_in_buffer(buffer,wMaxGet+3);
				if(ptx != NULL) {
					if(ptx->index > 0) {
						//��������
						PIE1bits.TXIE = 1;
					}
				}
			} else {
				//�յ�������̫���ˣ�������,�Լ���PLC�ļ�����жϹ�����
				//���SOCKET�����ڴ�
				TCPDiscard(MySocket);
			}







			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}

