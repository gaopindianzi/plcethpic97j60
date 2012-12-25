#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hal_io_interface.h"
#include "serial_comm_packeter.h"
#include "compiler.h"

#ifdef STACK_TCP_MODBUS



#define THIS_INFO  0
#define THIS_ERROR 0

#define  TCP_MODBUS_RX_MAX_LEN      32

#define  FORCE_SINGLE_COIL          0x05


unsigned int ForceSingleCoil(unsigned char * buffer,unsigned int len)
{
	typedef struct _force_single_coils_t
	{
		unsigned char slave_addr;
		unsigned char function;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char reg_munber_hi;
		unsigned char reg_number_lo;
		unsigned char crc_hi;
		unsigned char crc_lo;
	} force_single_coils_t;
	force_single_coils_t * pf = (force_single_coils_t *)buffer;
	unsigned char crc = HSB_BYTES_TO_WORD(&pf->crc_hi);
	if(len < sizeof(force_single_coils_t)) {
		//return 0;
	}
	if(CRC16(buffer,sizeof(force_single_coils_t)-2) != crc) {
		return 0;
	}
	if(pf->reg_munber_hi == 0xFF) {
	    io_out_set_bits(0,&code_msk[0],1);
	} else {
		io_out_set_bits(0,&code_msk[7],1);
	}
	return sizeof(force_single_coils_t);
}

unsigned int ModbusCmdPrase(unsigned char * buffer,unsigned int len)
{
	typedef struct _modbus_head_t
	{
		unsigned char slave_addr;
		unsigned char function;
	} modbus_head_t;
	modbus_head_t * ph = (modbus_head_t *)buffer;
	if(len < sizeof(modbus_head_t)) {
		return 0;
	}
	if(ph->slave_addr != 0x01) {
		return 0;
	}
	switch(ph->function)
	{
	case FORCE_SINGLE_COIL: return ForceSingleCoil(buffer,len);
	default:
		return 0;
	}
}

void ModbusTcpRxHandle(TCP_SOCKET MySocket)
{
	BYTE RX_Buffer[TCP_MODBUS_RX_MAX_LEN];
	WORD wMaxPut, wMaxGet;
	WORD w = sizeof(RX_Buffer);
	wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
	if(wMaxGet == 0) {
		return ;
	}
	wMaxGet = (w >= wMaxGet)?wMaxGet:w;
	if(wMaxGet > 0) {
	    TCPGetArray(MySocket, &RX_Buffer[0], wMaxGet);
		wMaxGet = ModbusCmdPrase((void *)RX_Buffer,(unsigned int)wMaxGet); //解析和TCP包,返回一定长度的应答包，然后返回给客户端
    	wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
	    if(wMaxGet > 0 && wMaxPut >= wMaxGet) {
			TCPPutArray(MySocket, RX_Buffer, wMaxGet);
			TCPFlush(MySocket);
		}
	}
}


void ModbusCmdTask(void)
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

			putrsUART((ROM char*)"\r\n in modbus sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 502, TCP_PURPOSE_MODBUS_SERVER0);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 502, TCP_PURPOSE_MODBUS_SERVER0);
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

			ModbusTcpRxHandle(MySocket);

			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}



#endif

