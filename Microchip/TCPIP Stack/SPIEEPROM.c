/*********************************************************************
 *
 *               Data SPI EEPROM Access Routines
 *
 *********************************************************************
 * FileName:        SPIEEPROM.c
 * Dependencies:    None
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     5/20/02     Original (Rev. 1.0)
 * Howard Schlunder		9/01/04		Rewritten for SPI EEPROMs
 * Howard Schlunder		8/10/06		Modified to control SPI module 
 *									frequency whenever EEPROM accessed 
 *									to allow bus sharing with different 
 *									frequencies.
********************************************************************/
#define __SPIEEPROM_C

#include "TCPIP Stack/TCPIP.h"

#if defined(MPFS_USE_EEPROM) && defined(EEPROM_CS_TRIS) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))

// IMPORTANT SPI NOTE: The code in this file expects that the SPI interrupt 
//		flag (EEPROM_SPI_IF) be clear at all times.  If the SPI is shared with 
//		other hardware, the other code should clear the EEPROM_SPI_IF when it is 
//		done using the SPI.

// SPI Serial EEPROM buffer size.  To enhance performance while
// cooperatively sharing the SPI bus with other peripherals, bytes 
// read and written to the memory are locally buffered. Legal 
// sizes are 1 to the EEPROM page size.
#define EEPROM_BUFFER_SIZE    			(32)

// EEPROM SPI opcodes
#define READ	0x03	// Read data from memory array beginning at selected address
#define WRITE	0x02	// Write data to memory array beginning at selected address
#define WRDI	0x04	// Reset the write enable latch (disable write operations)
#define WREN	0x06	// Set the write enable latch (enable write operations)
#define RDSR	0x05	// Read Status register
#define WRSR	0x01	// Write Status register

#define EEPROM_MAX_SPI_FREQ		(10000000ul)	// Hz

static void DoWrite(void);

static DWORD EEPROMAddress;
static BYTE EEPROMBuffer[EEPROM_BUFFER_SIZE];
static BYTE *EEPROMBufferPtr;

/*********************************************************************
 * Function:        void XEEInit(unsigned char speed)
 *
 * PreCondition:    None
 *
 * Input:           speed - not used (included for compatibility only)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize SPI module to communicate to serial
 *                  EEPROM.
 *
 * Note:            Code sets SPI clock to Fosc/16.  
 ********************************************************************/
#if defined(HPC_EXPLORER) && !defined(__18F87J10)
	#define PROPER_SPICON1	(0x20)		/* SSPEN bit is set, SPI in master mode, FOSC/4, IDLE state is low level */
#elif defined(__PIC24F__)
    #define PROPER_SPICON1 	(0x0013 | 0x0120)	/* 1:1 primary prescale, 4:1 secondary prescale, CKE=1, MASTER mode */
#elif defined(__dsPIC30F__)
    #define PROPER_SPICON1 	(0x0017 | 0x0120)	/* 1:1 primary prescale, 3:1 secondary prescale, CKE=1, MASTER mode */
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
    #define PROPER_SPICON1	(0x0003 | 0x0120)	/* 1:1 primary prescale, 8:1 secondary prescale, CKE=1, MASTER mode */
#elif defined(__PIC32MX__)
    #define PROPER_SPICON1	(_SPI2CON_ON_MASK | _SPI2CON_FRZ_MASK | _SPI2CON_CKE_MASK | _SPI2CON_MSTEN_MASK)
#else
	#define PROPER_SPICON1	(0x21)		/* SSPEN bit is set, SPI in master mode, FOSC/16, IDLE state is low level */
#endif

void XEEInit(void)
{
	EEPROM_CS_IO = 1;
	EEPROM_CS_TRIS = 0;		// Drive SPI EEPROM chip select pin

	EEPROM_SCK_TRIS = 0;	// Set SCK pin as an output
	EEPROM_SDI_TRIS = 1;	// Make sure SDI pin is an input
	EEPROM_SDO_TRIS = 0;	// Set SDO pin as an output

	#if defined(__C30__)
		EEPROM_SPICON1 = PROPER_SPICON1; // See PROPER_SPICON1 definition above
	    EEPROM_SPICON2 = 0;
	    EEPROM_SPISTAT = 0;    // clear SPI
	    EEPROM_SPISTATbits.SPIEN = 1;
	#elif defined(__C32__)
		EEPROM_SPIBRG = (GetPeripheralClock()-1ul)/2ul/EEPROM_MAX_SPI_FREQ;
	    EEPROM_SPICON1bits.CKE = 1;
	    EEPROM_SPICON1bits.MSTEN = 1;
		EEPROM_SPICON1bits.ON = 1;	
	#elif defined(__18CXX)
		EEPROM_SPICON1 = PROPER_SPICON1; // See PROPER_SPICON1 definition above
		EEPROM_SPI_IF = 0;
		EEPROM_SPISTATbits.CKE = 1; 	// Transmit data on rising edge of clock
		EEPROM_SPISTATbits.SMP = 0;		// Input sampled at middle of data output time
	#endif
}


/*********************************************************************
 * Function:        XEE_RESULT XEEBeginRead(DWORD address)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           address - Address at which read is to be performed.
 *
 * Output:          XEE_SUCCESS if successful
 *                  other value if failed.
 *
 * Side Effects:    None
 *
 * Overview:        Sets internal address counter to given address.
 *                  Puts EEPROM in sequential read mode.
 *
 * Note:            This function does not release I2C bus.
 *                  User must call XEEEndRead() when read is not longer
 *                  needed; I2C bus will released after XEEEndRead()
 *                  is called.
 ********************************************************************/
XEE_RESULT XEEBeginRead(DWORD address)
{
	// Save the address and emptry the contents of our local buffer
	EEPROMAddress = address;
	EEPROMBufferPtr = EEPROMBuffer + EEPROM_BUFFER_SIZE;
	return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        BYTE XEERead(void)
 *
 * PreCondition:    XEEInit() && XEEBeginRead() are already called.
 *
 * Input:           None
 *
 * Output:          BYTE that was read
 *
 * Side Effects:    None
 *
 * Overview:        Reads next byte from EEPROM; internal address
 *                  is incremented by one.
 *
 * Note:            None
 ********************************************************************/
BYTE XEERead(void)
{
	// Check if no more bytes are left in our local buffer
	if( EEPROMBufferPtr == EEPROMBuffer + EEPROM_BUFFER_SIZE )
	{ 
		// Get a new set of bytes
		XEEReadArray(EEPROMAddress, EEPROMBuffer, EEPROM_BUFFER_SIZE);
		EEPROMAddress += EEPROM_BUFFER_SIZE;
		EEPROMBufferPtr = EEPROMBuffer;
	}

	// Return a byte from our local buffer
	return *EEPROMBufferPtr++;
}

/*********************************************************************
 * Function:        XEE_RESULT XEEEndRead(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        This function does nothing.
 *
 * Note:            Function is used for backwards compatability with 
 *					I2C EEPROM module.
 ********************************************************************/
XEE_RESULT XEEEndRead(void)
{
    return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        XEE_RESULT XEEReadArray(DWORD address,
 *                                          BYTE *buffer,
 *                                          BYTE length)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           address     - Address from where array is to be read
 *                  buffer      - Caller supplied buffer to hold the data
 *                  length      - Number of bytes to read.
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Reads desired number of bytes in sequential mode.
 *                  This function performs all necessary steps
 *                  and releases the bus when finished.
 *
 * Note:            None
 ********************************************************************/
XEE_RESULT XEEReadArray(DWORD address,
                        BYTE *buffer,
                        BYTE length)
{
	BYTE Dummy;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif	

	// Save SPI state (clock speed)
	SPICON1Save = EEPROM_SPICON1;
	EEPROM_SPICON1 = PROPER_SPICON1;

	EEPROM_CS_IO = 0;

	// Send READ opcode
	EEPROM_SSPBUF = READ;
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	
	// Send address
	#if defined(USE_EEPROM_25LC1024)
	EEPROM_SSPBUF = ((DWORD_VAL*)&address)->v[2];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	#endif
	EEPROM_SSPBUF = ((DWORD_VAL*)&address)->v[1];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	EEPROM_SSPBUF = ((DWORD_VAL*)&address)->v[0];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	
	while(length--)
	{
		EEPROM_SSPBUF = 0;
		while(!EEPROM_SPI_IF);
		if(buffer != NULL)
			*buffer++ = EEPROM_SSPBUF;
		EEPROM_SPI_IF = 0;
	};
	
	EEPROM_CS_IO = 1;

	// Restore SPI state
	EEPROM_SPICON1 = SPICON1Save;

	return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        XEE_RESULT XEESetAddr(DWORD address)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           address     - address to be set for writing
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Modifies internal address counter of EEPROM.
 *
 * Note:            Unlike XEESetAddr() in xeeprom.c for I2C EEPROM 
 *					memories, this function is used only for writing
 *					to the EEPROM.  Reads must use XEEBeginRead(), 
 *					XEERead(), and XEEEndRead().
 *					This function does not release the SPI bus.
 ********************************************************************/
XEE_RESULT XEEBeginWrite(DWORD address)
{
	EEPROMAddress = address;
	EEPROMBufferPtr = EEPROMBuffer;
	return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        XEE_RESULT XEEWrite(BYTE val)
 *
 * PreCondition:    XEEInit() && XEEBeginWrite() are already called.
 *
 * Input:           val - Byte to be written
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Adds a byte to the current page to be writen when
 *					XEEEndWrite() is called.
 *
 * Note:            Page boundary cannot be exceeded or the byte 
 *					to be written will be looped back to the 
 *					beginning of the page.
 ********************************************************************/
XEE_RESULT XEEWrite(BYTE val)
{
	*EEPROMBufferPtr++ = val;
	if(EEPROMBufferPtr == EEPROMBuffer + EEPROM_BUFFER_SIZE)
    {
		DoWrite();
    }

    return XEE_SUCCESS;
}


/*********************************************************************
 * Function:        XEE_RESULT XEEEndWrite(void)
 *
 * PreCondition:    XEEInit() && XEEBeginWrite() are already called.
 *
 * Input:           None
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Instructs EEPROM to begin write cycle.
 *
 * Note:            Call this function after either page full of bytes
 *                  written or no more bytes are left to load.
 *                  This function initiates the write cycle.
 *                  User must call for XEEIsBusy() to ensure that write
 *                  cycle is finished before calling any other
 *                  routine.
 ********************************************************************/
XEE_RESULT XEEEndWrite(void)
{
	if( EEPROMBufferPtr != EEPROMBuffer )
    {
		DoWrite();
    }

    return XEE_SUCCESS;
}

static void DoWrite(void)
{
	BYTE Dummy;
	BYTE BytesToWrite;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif	

	// Save SPI state (clock speed)
	SPICON1Save = EEPROM_SPICON1;
	EEPROM_SPICON1 = PROPER_SPICON1;
	
	// Set the Write Enable latch
	EEPROM_CS_IO = 0;
	EEPROM_SSPBUF = WREN;
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	EEPROM_CS_IO = 1;
	
	// Send WRITE opcode
	EEPROM_CS_IO = 0;
	EEPROM_SSPBUF = WRITE;
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	
	// Send address
	#if defined(USE_EEPROM_25LC1024)
	EEPROM_SSPBUF = ((DWORD_VAL*)&EEPROMAddress)->v[2];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	#endif
	EEPROM_SSPBUF = ((DWORD_VAL*)&EEPROMAddress)->v[1];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	EEPROM_SSPBUF = ((DWORD_VAL*)&EEPROMAddress)->v[0];
	while(!EEPROM_SPI_IF);
	Dummy = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	
	BytesToWrite = (BYTE)(EEPROMBufferPtr - EEPROMBuffer);
	
	EEPROMAddress += BytesToWrite;
	EEPROMBufferPtr = EEPROMBuffer;

	while(BytesToWrite--)
	{
		// Send the byte to write
		EEPROM_SSPBUF = *EEPROMBufferPtr++;
		while(!EEPROM_SPI_IF);
		Dummy = EEPROM_SSPBUF;
		EEPROM_SPI_IF = 0;
	}

	// Begin the write
	EEPROM_CS_IO = 1;

	EEPROMBufferPtr = EEPROMBuffer;

	// Restore SPI State
	EEPROM_SPICON1 = SPICON1Save;

	// Wait for write to complete
	while( XEEIsBusy() );
}


/*********************************************************************
 * Function:        BOOL XEEIsBusy(void)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           None
 *
 * Output:          FALSE if EEPROM is not busy
 *                  TRUE if EEPROM is busy
 *
 * Side Effects:    None
 *
 * Overview:        Reads the status register
 *
 * Note:            None
 ********************************************************************/
BOOL XEEIsBusy(void)
{
	BYTE_VAL result;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif	

	// Save SPI state (clock speed)
	SPICON1Save = EEPROM_SPICON1;
	EEPROM_SPICON1 = PROPER_SPICON1;

	EEPROM_CS_IO = 0;
	// Send RDSR - Read Status Register opcode
	EEPROM_SSPBUF = RDSR;
	while(!EEPROM_SPI_IF);
	result.Val = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	
	// Get register contents
	EEPROM_SSPBUF = 0;
	while(!EEPROM_SPI_IF);
	result.Val = EEPROM_SSPBUF;
	EEPROM_SPI_IF = 0;
	EEPROM_CS_IO = 1;

	// Restore SPI State
	EEPROM_SPICON1 = SPICON1Save;

	return result.bits.b0;
}


#endif //#if defined(MPFS_USE_EEPROM) && defined(EEPROM_CS_TRIS) && defined(STACK_USE_MPFS)
