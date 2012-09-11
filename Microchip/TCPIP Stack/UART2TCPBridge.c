/*********************************************************************
 *
 *	UART <-> TCP Bridge Example
 *  Module for Microchip TCP/IP Stack
 *	 -Transmits all incoming TCP bytes on a socket out the UART 
 *    module, all incoming UART bytes out of the TCP socket.
 *	 -Reference: None (hopefully AN833 in the future)
 *
 *********************************************************************
 * FileName:        UART2TCPBridge.c
 * Dependencies:    TCP, Hardware UART module, ARP (optional), 
 *					DNS (optional)
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.00 or higher
 *					Microchip C30 v3.01 or higher
 *					Microchip C18 v3.13 or higher
 *					HI-TECH PICC-18 STD 9.50PL3 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2008 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and 
 * distribute: 
 * (i)  the Software when embedded on a Microchip microcontroller or 
 *      digital signal controller product ("Device") which is 
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c and 
 *      ENC28J60.h ported to a non-Microchip device used in 
 *      conjunction with a Microchip ethernet controller for the 
 *      sole purpose of interfacing with the ethernet controller. 
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT 
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL 
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS 
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE 
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER 
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		06/12/07	Original
 ********************************************************************/
#define __UART2TCPBRIDGE_C

#include "TCPIP Stack/TCPIP.h"

#define UART2TCPBRIDGE_PORT	1000
#define BAUD_RATE			115200


#if defined(STACK_USE_UART2TCP_BRIDGE)

// Comment this define out if we are the server.  
// Insert the appropriate address if we are the client.
#define USE_REMOTE_TCP_SERVER	"192.168.1.115"

// Ring buffers for transfering data to and from the UART ISR:
//  - (Head pointer == Tail pointer) is defined as an empty FIFO
//  - (Head pointer == Tail pointer - 1), accounting for wraparound,
//    is defined as a completely full FIFO.  As a result, the max data 
//    in a FIFO is the buffer size - 1.
static BYTE vUARTRXFIFO[256]; //串口是慢速设备，一般都够大了
static BYTE vUARTTXFIFO[256];//TCP传进来的数据，一次性传的比较大，尽量大些，最大是多大1500字节呢
static BYTE *RXHeadPtr = vUARTRXFIFO, *RXTailPtr = vUARTRXFIFO;
static BYTE *TXHeadPtr = vUARTTXFIFO, *TXTailPtr = vUARTTXFIFO;


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
void UART2TCPBridgeInit(void)
{
	// Initilize UART

    TXSTA = 0x20;
    RCSTA = 0x90;

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
		BAUDCONbits.BRG16 = 1;
		TXSTAbits.BRGH = 1;
		SPBRGH = ((GetPeripheralClock()+BAUD_RATE/2)/BAUD_RATE-1)>>8 & 0xFF;
		SPBRG = ((GetPeripheralClock()+BAUD_RATE/2)/BAUD_RATE-1) & 0xFF;
	#else
		// See if we can use the high baud (Fosc/16) 8-bit rate setting
		#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
			SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
			TXSTAbits.BRGH = 1;
		#else	// Use the low baud rate 8-bit setting
			SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
		#endif
	#endif
	
	// Use high priority interrupt
	IPR1bits.TXIP = 1;

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
void UART2TCPBridgeTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;
	WORD wMaxPut, wMaxGet, w;
	BYTE *RXHeadPtrShadow, *RXTailPtrShadow;
	BYTE *TXHeadPtrShadow, *TXTailPtrShadow;


	switch(BridgeState)
	{
		case SM_HOME:
			#if defined(USE_REMOTE_TCP_SERVER)
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER, TCP_OPEN_ROM_HOST, UART2TCPBRIDGE_PORT, TCP_PURPOSE_UART_2_TCP_BRIDGE);
			#else
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, UART2TCPBRIDGE_PORT, TCP_PURPOSE_UART_2_TCP_BRIDGE);
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

			if(RCSTAbits.OERR)
			{
				RCSTAbits.CREN = 0;
				RCSTAbits.CREN = 1;
				LED1_IO ^= 1;
			}
			if(RCSTAbits.FERR)
			{
				BYTE dummy = RCREG;
				LED2_IO ^= 1;
			}

			
#if 1  //测试不收发数据的情况

			
			//
			// Transfer received TCP data into the UART TX FIFO for future transmission (in the ISR)
			//

			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count

			
			//从TCP发送到UART
			TXHeadPtrShadow = TXHeadPtr;
			PIE1bits.TXIE = 0;  //关闭UART发送
			TXTailPtrShadow = TXTailPtr;
			PIE1bits.TXIE = 1; //打开UART中断，允许发送产生中断

			//计算没法出去的数据大小
			//Head装入,Tail发出
			if(TXHeadPtrShadow >= TXTailPtrShadow) {
				wMaxPut = TXHeadPtrShadow - TXTailPtrShadow;// Get UART TX FIFO free space
			} else {
				wMaxPut = TXTailPtrShadow - TXHeadPtrShadow;// Get UART TX FIFO free space
			}
			//计算空的空间
			wMaxPut = sizeof(vUARTTXFIFO) - wMaxPut;
			//
#if 0
			wMaxPut = TXTailPtrShadow - TXHeadPtrShadow;// Get UART TX FIFO free space
			if(TXHeadPtrShadow >= TXTailPtrShadow)
				wMaxPut += sizeof(vUARTTXFIFO);
#endif

			if(wMaxPut >= wMaxGet) {				// Calculate the lesser of the two
				wMaxPut = wMaxGet;
			} else {
				LED6_IO = 1;
			}
			if(wMaxPut)							// See if we can transfer anything
			{
				// Transfer the data over.  Note that a two part put 
				// may be needed if the data spans the vUARTTXFIFO 
				// end to start address.
				w = vUARTTXFIFO + sizeof(vUARTTXFIFO) - TXHeadPtrShadow;
				if(wMaxPut >= w)
				{
					TCPGetArray(MySocket, TXHeadPtrShadow, w);

					TXHeadPtrShadow = vUARTTXFIFO;
					wMaxPut -= w;
				}
				TCPGetArray(MySocket, TXHeadPtrShadow, wMaxPut);
				TCPFlush(MySocket);

				TXHeadPtrShadow += wMaxPut;


			    PIE1bits.TXIE = 0;
			    TXHeadPtr = TXHeadPtrShadow;
			    PIE1bits.TXIE = 1;
			}




			//从UART发送到TCP

			// Read FIFO pointers into a local shadow copy.  Some pointers are volatile 
			// (modified in the ISR), so we must do this safely by disabling interrupts

			RXTailPtrShadow = RXTailPtr;
			PIE1bits.RCIE = 0;
			RXHeadPtrShadow = RXHeadPtr;
			PIE1bits.RCIE = 1;

			//
			// Transmit pending data that has been placed into the UART RX FIFO (in the ISR)
			//
			wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
#if 0
			wMaxGet = RXHeadPtrShadow - RXTailPtrShadow;	// Get UART RX FIFO byte count
			if(RXHeadPtrShadow < RXTailPtrShadow)
				wMaxGet += sizeof(vUARTRXFIFO);
#else
			//计算有多少数据
			wMaxGet = 0;
			if(RXHeadPtrShadow != RXTailPtrShadow) {
			    if(RXHeadPtrShadow >= RXTailPtrShadow) {
				    wMaxGet = RXHeadPtrShadow - RXTailPtrShadow;
			    } else {
				    wMaxGet = RXHeadPtrShadow + sizeof(vUARTRXFIFO) - RXTailPtrShadow;
			    }
			}
#endif
			if(wMaxPut >= wMaxGet) {				// Calculate the lesser of the two
				wMaxPut = wMaxGet;
			} else {
				LED5_IO = 1;
			}
			if(wMaxPut)							// See if we can transfer anything
			{
				// Transfer the data over.  Note that a two part put 
				// may be needed if the data spans the vUARTRXFIFO 
				// end to start address.
				w = vUARTRXFIFO + sizeof(vUARTRXFIFO) - RXTailPtrShadow;

				if(wMaxPut >= w)
				{
					TCPPutArray(MySocket, RXTailPtrShadow, w);

					RXTailPtrShadow = vUARTRXFIFO;
					wMaxPut -= w;
				}
				TCPPutArray(MySocket, RXTailPtrShadow, wMaxPut);
				TCPFlush(MySocket);

				RXTailPtrShadow += wMaxPut;

				PIE1bits.RCIE = 0;
				RXTailPtr = RXTailPtrShadow;
				PIE1bits.RCIE = 1;

				// No flush.  The stack will automatically flush and do 
				// transmit coallescing to minimize the number of TCP 
				// packets that get sent.  If you explicitly call TCPFlush()
				// here, latency will go down, but so will max throughput 
				// and bandwidth efficiency.
			}


#endif



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
void UART2TCPBridgeISR(void)
{
	// NOTE: All local variables used here should be declared static
	static BYTE i,s;

	// Store a received byte, if pending, if possible
	if(PIR1bits.RCIF && PIE1bits.RCIE)
	{
		// Get the byte
		i = RCREG;

		// Clear the interrupt flag so we don't keep entering this ISR
		PIR1bits.RCIF = 0;

		// Copy the byte into the local FIFO, if it won't cause an overflow
#if 0   //不等于不等于有空间，也许已经满了
		if(RXHeadPtr != RXTailPtr)
		{
			if((RXHeadPtr != vUARTRXFIFO + sizeof(vUARTRXFIFO)) || (RXTailPtr != vUARTRXFIFO))
			{
				*RXHeadPtr++ = i;
				if(RXHeadPtr >= vUARTRXFIFO + sizeof(vUARTRXFIFO))
					RXHeadPtr = vUARTRXFIFO;
			}
		}
#else
		//计算使用了多少空间
		if(RXHeadPtr >= RXTailPtr) {
			s = RXHeadPtr - RXTailPtr;
		} else {
			s = RXHeadPtr + sizeof(vUARTRXFIFO) - RXTailPtr;
		}
		//计算剩余多少空间
		s = sizeof(vUARTRXFIFO) - s;
		
		if(s > 0)
		{
			*RXHeadPtr++ = i;
			if(RXHeadPtr >= vUARTRXFIFO + sizeof(vUARTRXFIFO)) {
				RXHeadPtr = vUARTRXFIFO;
			}
#endif
		}
	}

	// Transmit a byte, if pending, if possible
	if(PIR1bits.TXIF && PIE1bits.TXIE)
	{
#if 0
		if(TXHeadPtr != TXTailPtr)
		{
		    TXREG = *TXTailPtr++;
			if(TXTailPtr >= vUARTTXFIFO + sizeof(vUARTTXFIFO))
			TXTailPtr = vUARTTXFIFO;
		}
		else	// Disable the TX interrupt if we are done so that we don't keep entering this ISR
		{
	        PIE1bits.TXIE = 0;
		}
#else
		if(TXTailPtr != TXHeadPtr) {
			TXREG = *TXTailPtr++;
			if(TXTailPtr >= vUARTTXFIFO + sizeof(vUARTTXFIFO)) {
			    TXTailPtr = vUARTTXFIFO;
			}
		} else {
			PIE1bits.TXIE = 0;
		}
#endif
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
