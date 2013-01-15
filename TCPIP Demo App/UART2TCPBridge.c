

#define __UART2TCPBRIDGE_C__

#include "TCPIP Stack/TCPIP.h"

#include "serial_comm_packeter.h"
#include "debug.h"

#define   THISINFO         0
#define   THISERROR        0

#define UART1TCPBRIDGE_PORT	   506
#define BAUD_RATE		       (9600)


#if defined(STACK_USE_UART2TCP_BRIDGE2)


// Comment this define out if we are the server.  
// Insert the appropriate address if we are the client.
//#define USE_REMOTE_TCP_SERVER	"192.168.1.115"

// Ring buffers for transfering data to and from the UART ISR:
//  - (Head pointer == Tail pointer) is defined as an empty FIFO
//  - (Head pointer == Tail pointer - 1), accounting for wraparound,
//    is defined as a completely full FIFO.  As a result, the max data 
//    in a FIFO is the buffer size - 1.
//static BYTE vUARTRXFIFO[64]; //串口是慢速设备，一般都够大了
//static BYTE vUARTTXFIFO[64];//TCP传进来的数据，一次性传的比较大，尽量大些，最大是多大1500字节呢
//static BYTE *RXHeadPtr = vUARTRXFIFO, *RXTailPtr = vUARTRXFIFO;
//static BYTE *TXHeadPtr = vUARTTXFIFO, *TXTailPtr = vUARTTXFIFO;

static DATA_TX_PACKET_T * pcurrent_tx;
static unsigned int       tx_index;


/*********************************************************************
 * Function:        void UART2TCPBridgeInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets up the UART peripheral for this application
 *
 * Note:            Uses interrupts
 ********************************************************************/
void UART2TCPBridgeInit2(void)
{
	// Initilize UART

	putrsUART((ROM char*)"\r\n IN UART2TCPBridgeInit()");

    TXSTA2 = 0x20;
    RCSTA2 = 0x90;




	#define CLOSEST_SPBRG_VALUE ((GetPeripheralClock()+2ul*BAUD_RATE)/BAUD_RATE/4-1)
	#define BAUD_ACTUAL (GetPeripheralClock()/(CLOSEST_SPBRG_VALUE+1))
	#if (BAUD_ACTUAL > BAUD_RATE)
		#define BAUD_ERROR (BAUD_ACTUAL-BAUD_RATE)
	#else
		#define BAUD_ERROR (BAUD_RATE-BAUD_ACTUAL)
	#endif


	#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)
	#if BAUD_ERROR_PRECENT > 2
		// Use high speed (Fosc/4) 16-bit baud rate generator
		//BAUDCONbits.BRG16 = 1;
		BRG162 = 1;
		//TXSTA2bits.BRGH = 1;
		BRGH2 = 1;
		SPBRGH2 = ((GetPeripheralClock()+BAUD_RATE/2)/BAUD_RATE-1)>>8 & 0xFF;
		SPBRG2 = ((GetPeripheralClock()+BAUD_RATE/2)/BAUD_RATE-1) & 0xFF;
	#else
		// See if we can use the high baud (Fosc/16) 8-bit rate setting
		#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
			SPBRG2 = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
			//TXSTA2bits.BRGH = 1;
			BRGH2 = 1;
		#else	// Use the low baud rate 8-bit setting
			SPBRG2 = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
		#endif
	#endif
	
	// Use high priority interrupt
	//IPR1bits.TXIP = 1;
	TX2IP = 1;

	RC2IE = 1;

}


/*********************************************************************
 * Function:        void UART2TCPBridgeTask(void)
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void UART2TCPBridgeTask2(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;
	WORD wMaxPut, wMaxGet;  //, w;
	//BYTE *RXHeadPtrShadow, *RXTailPtrShadow;
	//BYTE *TXHeadPtrShadow, *TXTailPtrShadow;
	unsigned char buffer[PACK_MAX_RX_SIZE];
	DATA_RX_PACKET_T * prx;
	DATA_TX_PACKET_T * ptx;

	switch(BridgeState)
	{
	default:
		case SM_HOME:

			putrsUART((ROM char*)"\r\n IN UART2TCPBridgeTask() home");

			#if defined(USE_REMOTE_TCP_SERVER)
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER, TCP_OPEN_ROM_HOST, UART1TCPBRIDGE_PORT, TCP_PURPOSE_UART_2_TCP_BRIDGE);
			#else
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, UART1TCPBRIDGE_PORT, TCP_PURPOSE_UART_2_TCP_BRIDGE);
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

			//RELAY_OUT_1 = 0;

			// Reset all buffers if the connection was lost
			if(TCPWasReset(MySocket))
			{
				// Optionally discard anything in the UART FIFOs
				//RXHeadPtr = vUARTRXFIFO;
				//RXTailPtr = vUARTRXFIFO;
				//TXHeadPtr = vUARTTXFIFO;
				//TXTailPtr = vUARTTXFIFO;
				
				// If we were a client socket, close the socket and attempt to reconnect
				#if defined(USE_REMOTE_TCP_SERVER)
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					BridgeState = SM_HOME;
					break;
				#endif
			}
		
			// Don't do anything if nobody is connected to us
			if(!TCPIsConnected(MySocket))
			{	
				LED7_IO = 0;
				break;
			}
			
			LED7_IO = 1;

			// Make sure to clear UART errors so they don't block all future operations

			if(OERR2) //RCSTAbits.OERR)
			{
				//RCSTAbits.CREN = 0;
				CREN2 = 0;
				//RCSTAbits.CREN = 1;
				CREN2 = 1;
				LED1_IO ^= 1;
			}
			if(FERR2) //RCSTAbits.FERR)
			{
				BYTE dummy = RCREG2; //RCREG;
				LED2_IO ^= 1;
			}



			prx =  GetFinishedPacket();
			if(prx != NULL) {
				if(prx->finished) {
					if(prx->index > 0) {
						wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
						wMaxPut = (prx->index > wMaxPut)?wMaxPut:prx->index;
						if(wMaxPut > 0) {
							TCPPutArray(MySocket, &(prx->buffer[0]), wMaxPut);
							prx->index = 0;
							prx->finished = 0;
							TCPFlush(MySocket);
						}
					}
					prx->index = 0;
					prx->finished = 0;  //测试用
					//if(THISINFO)putrsUART((ROM char *)"\r\UART Bridge look at it");
				}
			}
			//PIE1bits.RCIE = 1;
			RC2IE = 1;
			



			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
			if(wMaxGet > 0) {
				DATA_TX_PACKET_T * ptx = find_next_empty_tx_buffer();
				if(ptx != NULL) {
					wMaxGet = (wMaxGet > PACK_MAX_RX_SIZE)?PACK_MAX_RX_SIZE:wMaxGet;
				    TCPGetArray(MySocket,(BYTE *)&buffer[0],wMaxGet);
				    ptx = prase_in_buffer(buffer,wMaxGet);
				    if(ptx != NULL) {
					    if(ptx->index > 0) {
						    //启动发送
						    //PIE1bits.TXIE = 1;
							TX2IE = 1;
					    }
				    }
				}
			} else {
				TCPDiscard(MySocket);
			}
			break;
	}
}


#if defined(__18CXX)
/*********************************************************************
 * Function:        void UART2TCPBridgeISR(void)
 *
 * PreCondition:    UART interrupt has occured
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This function is supposed to be called in the ISR 
 *					context.
 ********************************************************************/
void UART2TCPBridgeISR2(void)
{
	// NOTE: All local variables used here should be declared static
	static BYTE i;

	// Store a received byte, if pending, if possible
	if(RC2IF && RC2IE) //PIR1bits.RCIF && PIE1bits.RCIE)
	{
		// Get the byte
		i = RCREG2;

		// Clear the interrupt flag so we don't keep entering this ISR
		//PIR1bits.RCIF = 0;
		RC2IF = 0;

		pack_prase_in(i);
	
	}

	// Transmit a byte, if pending, if possible
	if(TX2IF && TX2IE)  //PIR1bits.TXIF && PIE1bits.TXIE)
	{
		if(pcurrent_tx == NULL) {
			pcurrent_tx = find_ready_tx_buffer();
			if(pcurrent_tx != NULL) {
				//启动发送
				tx_index = 0;
			}
		}
		if(pcurrent_tx) {
			if(tx_index < pcurrent_tx->index) {
				TXREG2 = pcurrent_tx->buffer[tx_index++];
			} else {
				//完毕
				pcurrent_tx->finished = 0;
				pcurrent_tx = NULL;
				//PIE1bits.TXIE = 0;
				TX2IE = 0;
			}
		}
	}
}

#else
/*********************************************************************
 * Function:        void _ISR _U2RXInterrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Copies bytes to and from the local UART TX and 
 *					RX FIFOs
 *
 * Note:            None
 ********************************************************************/
#if __C30_VERSION__ >= 300
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
#elif defined(__C30__)
void _ISR _U2RXInterrupt(void)
#else
void _U2RXInterrupt(void)
#endif
{
	BYTE i;

	// Store a received byte, if pending, if possible
	// Get the byte
	i = U2RXREG;
	
	// Clear the interrupt flag so we don't keep entering this ISR
	IFS1bits.U2RXIF = 0;
	
	// Copy the byte into the local FIFO, if it won't cause an overflow
	if(RXHeadPtr != RXTailPtr - 1)
	{
		if((RXHeadPtr != vUARTRXFIFO + sizeof(vUARTRXFIFO)) || (RXTailPtr != vUARTRXFIFO))
		{
			*RXHeadPtr++ = i;
			if(RXHeadPtr >= vUARTRXFIFO + sizeof(vUARTRXFIFO))
				RXHeadPtr = vUARTRXFIFO;
		}
	}
}
/*********************************************************************
 * Function:        void _ISR _U2TXInterrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Copies bytes to and from the local UART TX and 
 *					RX FIFOs
 *
 * Note:            None
 ********************************************************************/
#if __C30_VERSION__ >= 300
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
#elif defined(__C30__)
void _ISR _U2TXInterrupt(void)
#else
void _U2TXInterrupt(void)
#endif
{
	// Transmit a byte, if pending, if possible
	if(TXHeadPtr != TXTailPtr)
	{
		// Clear the TX interrupt flag before transmitting again
		IFS1bits.U2TXIF = 0;

		U2TXREG = *TXTailPtr++;
		if(TXTailPtr >= vUARTTXFIFO + sizeof(vUARTTXFIFO))
			TXTailPtr = vUARTTXFIFO;
	}
	else	// Disable the TX interrupt if we are done so that we don't keep entering this ISR
	{
		IEC1bits.U2TXIE = 0;
	}
}


#if defined(__C32__)
void __attribute((interrupt(ipl6), vector(_UART2_VECTOR), nomips16)) U2Interrupt(void)
{
	if(IFS1bits.U2RXIF)
		_U2RXInterrupt();
	if(IEC1bits.U2TXIE)
	{
		if(IFS1bits.U2TXIF)
			_U2TXInterrupt();
	}
}
#endif

#endif	// end of ISRs


#endif	//#if defined(STACK_USE_UART2TCP_BRIDGE)
