#include "TCPIP Stack/TCPIP.h"

#include "MainDemo.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "hal_io_interface.h"
#include "serial_comm_packeter.h"
#include "modbus_imp.h"
#include "compiler.h"
#include "plc_prase.h"
#include "debug.h"



#ifdef STACK_TCP_MODBUS

#define THISINFO      1
#define THISERROR     1

#define  TCP_MODBUS_RX_MAX_LEN      32

#define  READ_COILS                 0x01
#define  READ_DISCRETES             0x02
#define  FORCE_SINGLE_COIL          0x05

typedef struct _crc_t
{
	unsigned char crc_hi;
	unsigned char crc_lo;
} crc_t;


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
	unsigned int crc = HSB_BYTES_TO_WORD(&pf->crc_hi);
	if(THISINFO)putrsUART((ROM char*)"\r\nmodbus force single coil.");
	if(len < sizeof(force_single_coils_t)) {
		if(THISERROR)putrsUART((ROM char*)"\r\nmodbus len error!");
		return 0;
	}
	if(CRC16(buffer,sizeof(force_single_coils_t)-2) != crc) {
		if(THISERROR)putrsUART((ROM char*)"\r\nmodbus crc error!");
		//return 0;
	}
	if(pf->reg_munber_hi == 0xFF) {
		set_bitval(HSB_BYTES_TO_WORD(&pf->start_addr_hi),1);
	} else {
		set_bitval(HSB_BYTES_TO_WORD(&pf->start_addr_hi),0);
	}
	return sizeof(force_single_coils_t);
}

unsigned int read_input_discretes(unsigned char * buffer,unsigned int len)
{
	typedef struct __modbus_read_coil_status
	{
		unsigned char slave_addr;
		unsigned char function;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char reg_munber_hi;
		unsigned char reg_number_lo;
		unsigned char crc_hi;
		unsigned char crc_lo;
	} modbus_read_coil_status;
	typedef struct __modbus_read_coil_status_ack
	{
		unsigned char slave_addr;
		unsigned char function;
		unsigned char byte_count;
		//unsigned char data_arry[]; //不定长的
		//unsigned char crc_hi;
		//unsigned char crc_lo;
	} modbus_read_coil_status_ack;
	modbus_read_coil_status * pm = (modbus_read_coil_status *)buffer;
	modbus_read_coil_status_ack * pack = (modbus_read_coil_status_ack *)buffer;
	unsigned int crc = HSB_BYTES_TO_WORD(&pm->crc_hi);
	if(len < sizeof(modbus_read_coil_status)) {
		if(THISERROR)putrsUART((ROM char *)"\r\n read input or coils data len error!");
		return 0;
	}
	if(CRC16(buffer,sizeof(modbus_read_coil_status)-2) != crc) {
		if(THISERROR)putrsUART((ROM char*)"\r\nmodbus crc error!");
		//return 0;
	}

	{
		unsigned int i;
		unsigned int start = HSB_BYTES_TO_WORD(&pm->start_addr_hi);
		unsigned int num = HSB_BYTES_TO_WORD(&pm->reg_munber_hi);
		num = (num > 32)?32:num; //限制一下，不要读太多，以免内存溢出错误
		pack->byte_count = BITS_TO_BS(num);
		memset((buffer+sizeof(modbus_read_coil_status_ack)),0,pack->byte_count);
		for(i=0;i<num;i++) {
			SET_BIT((buffer+sizeof(modbus_read_coil_status_ack)),i,get_bitval(start+i));
		}
		crc = CRC16(buffer,sizeof(modbus_read_coil_status_ack)+pack->byte_count);
		{
			crc_t * pc = (crc_t *)(buffer+sizeof(modbus_read_coil_status_ack)+pack->byte_count);
			pc->crc_hi = crc >> 8;
			pc->crc_lo = crc & 0xFF;
		}
		return sizeof(modbus_read_coil_status_ack)+pack->byte_count+2;
	}

	return 0;
}


unsigned int ModbusCmdPrase(unsigned char * buffer,unsigned int len)
{
	typedef struct _modbus_head_t
	{
		unsigned char slave_addr;
		unsigned char function;
	} modbus_head_t;
	modbus_head_t * ph = (modbus_head_t *)buffer;
	if(THISINFO)putrsUART((ROM char*)"\r\n+ModbusCmdPrase");
	if(len < sizeof(modbus_head_t)) {
		if(THISERROR)putrsUART((ROM char*)"\r\nmodbus head error!!");
		return 0;
	}
	if(ph->slave_addr != 0x01) {
		if(THISERROR)putrsUART((ROM char*)"\r\nmodbus slave addr error!!");
		return 0;
	}
	switch(ph->function)
	{
	case FORCE_SINGLE_COIL: return ForceSingleCoil(buffer,len);
	case READ_COILS:
	case READ_DISCRETES:    return read_input_discretes(buffer,len);
	default:
		return 0;
	}
}

void ModbusTcpRxHandle(TCP_SOCKET MySocket)
{
	BYTE RX_Buffer[TCP_MODBUS_RX_MAX_LEN];
	WORD wMaxPut, wMaxGet;
	wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
	if(wMaxGet == 0) {
		return ;
	}
	wMaxGet = (sizeof(RX_Buffer) >= wMaxGet)?wMaxGet:sizeof(RX_Buffer);
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

