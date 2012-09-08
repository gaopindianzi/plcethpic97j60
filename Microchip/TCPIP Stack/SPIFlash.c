/*********************************************************************
 *
 *  SPI Flash Interface Headers
 *	- Tested to be compatible with SST25VF016B
 *	- Expected compatibility with all SST25VFxxxB devices
 *
 *********************************************************************
 * FileName:        SPIFlash.c
 * Dependencies:    None
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32MX
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * E. Wood				3/20/08	Original
********************************************************************/
#define __SPIFLASH_C

#include "TCPIP Stack/TCPIP.h"

#if defined(MPFS_USE_SPI_FLASH) && defined(SPIFLASH_CS_TRIS) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))

#define READ			0x03	// SPI Flash opcode: Read up up to 25MHz
#define READ_FAST		0x0B	// SPI Flash opcode: Read up to 50MHz with 1 dummy byte
#define ERASE_4K		0x20	// SPI Flash opcode: 4KByte sector erase
#define ERASE_32K		0x52	// SPI Flash opcode: 32KByte block erase
#define ERASE_64K		0xD8	// SPI Flash opcode: 64KByte block erase
#define ERASE_ALL		0x60	// SPI Flash opcode: Entire chip erase
#define WRITE			0x02	// SPI Flash opcode: Write one byte
#define WRITE_STREAM	0xAD	// SPI Flash opcode: Write continuous stream of words (AAI mode)
#define RDSR			0x05	// SPI Flash opcode: Read Status Register
#define EWSR			0x50	// SPI Flash opcode: Enable Write Status Register
#define WRSR			0x01	// SPI Flash opcode: Write Status Register
#define WREN			0x06	// SPI Flash opcode: Write Enable
#define WRDI			0x04	// SPI Flash opcode: Write Disable / End AAI mode
#define RDID			0x90	// SPI Flash opcode: Read ID
#define JEDEC_ID		0x9F	// SPI Flash opcode: Read JEDEC ID
#define EBSY			0x70	// SPI Flash opcode: Enable write BUSY status on SO pin
#define DBSY			0x80	// SPI Flash opcode: Disable write BUSY status on SO pin

#define BUSY	0x01	// Mask for Status Register BUSY bit
#define WEL		0x02	// Mask for Status Register BUSY bit
#define BP0		0x04	// Mask for Status Register BUSY bit
#define BP1		0x08	// Mask for Status Register BUSY bit
#define BP2		0x10	// Mask for Status Register BUSY bit
#define BP3		0x20	// Mask for Status Register BUSY bit
#define AAI		0x40	// Mask for Status Register BUSY bit
#define BPL		0x80	// Mask for Status Register BUSY bit

#if defined(__PIC24F__)
    #define PROPER_SPICON1 	(0x001B | 0x0120)	// 1:1 primary prescale, 2:1 secondary prescale, CKE=1, MASTER mode
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
    #define PROPER_SPICON1	(0x000F | 0x0120)	// 1:1 primary prescale, 5:1 secondary prescale, CKE=1, MASTER mode
#elif defined(__dsPIC30F__)
    #define PROPER_SPICON1 	(0x0017 | 0x0120)	// 1:1 primary prescale, 3:1 secondary prescale, CKE=1, MASTER mode
#elif defined(__PIC32MX__)
    #define PROPER_SPICON1	(_SPI2CON_ON_MASK | _SPI2CON_FRZ_MASK | _SPI2CON_CKE_MASK | _SPI2CON_MSTEN_MASK)
#else
	#define PROPER_SPICON1	(0x20)				// SSPEN bit is set, SPI in master mode, FOSC/4, IDLE state is low level
#endif

// Maximum speed of SPI Flash part in Hz
// Should theoretically operate at 25MHz, but need to account for level-shifting delays
#define SPIFLASH_MAX_SPI_FREQ		(16000000ul)

#define SPI_FLASH_SECTOR_SIZE		(4096ul)
#define SPI_FLASH_SECTOR_MASK		(SPI_FLASH_SECTOR_SIZE - 1)

// Internal pointer to address being written
DWORD dwWriteAddr;


void _SendCmd(BYTE cmd);
void _WaitWhileBusy(void);
void _EraseSector(DWORD dwAddr);
//void _GetStatus(void);


/*****************************************************************************
  Function:
	void SPIFlashInit(void)

  Description:
	Initializes SPI Flash module.

  Precondition:
	None

  Parameters:
	None

  Returns:
	None

  Remarks:
	This function is only called once during the lifetime of the application.

  Internal:
	This function sends WRDI to clear any pending write operation, and also
	clears the software write-protect on all memory locations.
  ***************************************************************************/
void SPIFlashInit(void)
{
	BYTE Dummy;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif	

	SPIFLASH_CS_IO = 1;
	SPIFLASH_CS_TRIS = 0;	// Drive SPI Flash chip select pin

	SPIFLASH_SCK_TRIS = 0;	// Set SCK pin as an output
	SPIFLASH_SDI_TRIS = 1;	// Make sure SDI pin is an input
	SPIFLASH_SDO_TRIS = 0;	// Set SDO pin as an output

	// Save SPI state (clock speed)
	SPICON1Save = SPIFLASH_SPICON1;
	SPIFLASH_SPICON1 = PROPER_SPICON1;

	#if defined(__C30__)
	    SPIFLASH_SPICON2 = 0;
	    SPIFLASH_SPISTAT = 0;    // clear SPI
	    SPIFLASH_SPISTATbits.SPIEN = 1;
	#elif defined(__C32__)
		SPIFLASH_SPIBRG = (GetPeripheralClock()-1ul)/2ul/SPIFLASH_MAX_SPI_FREQ;	
	#elif defined(__18CXX)
		SPIFLASH_SPI_IF = 0;
		SPIFLASH_SPISTATbits.CKE = 1;		// Transmit data on rising edge of clock
		SPIFLASH_SPISTATbits.SMP = 0;		// Input sampled at middle of data output time
	#endif


	// Clear any pre-existing AAI write mode
	// This may occur if the PIC is reset during a write, but the Flash is 
	// not tied to the same hardware reset.
	_SendCmd(WRDI);

	// Execute Enable-Write-Status-Register (EWSR) instruction
	_SendCmd(EWSR);

	// Clear Write-Protect on all memory locations
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SSPBUF = WRSR;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = 0x00;	// Clear all block protect bits
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_CS_IO = 1;

	// Restore SPI state
	SPIFLASH_SPICON1 = SPICON1Save;
}


/*****************************************************************************
  Function:
	void SPIFlashReadArray(DWORD dwAddress, BYTE *vData, WORD wLength)

  Description:
	Reads an array of bytes from the SPI Flash module.

  Precondition:
	SPIFlashInit has been called, and the chip is not busy (should be
	handled elsewhere automatically.)

  Parameters:
	dwAddress - Address from which to read
	vData - Where to store data that has been read
	wLength - Length of data to read

  Returns:
	None
  ***************************************************************************/
void SPIFlashReadArray(DWORD dwAddress, BYTE *vData, WORD wLength)
{
	BYTE Dummy;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif	

	// Ignore operations when the destination is NULL or nothing to read
	if(vData == NULL || wLength == 0)
		return;

	// Save SPI state (clock speed)
	SPICON1Save = SPIFLASH_SPICON1;
	SPIFLASH_SPICON1 = PROPER_SPICON1;
	
	// Activate chip select
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SPI_IF = 0;

	// Send READ opcode
	SPIFLASH_SSPBUF = READ;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Send address
	SPIFLASH_SSPBUF = ((BYTE*)&dwAddress)[2];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwAddress)[1];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwAddress)[0];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Read data
	while(wLength--)
	{
		SPIFLASH_SSPBUF = 0;
		while(!SPIFLASH_SPI_IF);
		*vData++ = SPIFLASH_SSPBUF;
		SPIFLASH_SPI_IF = 0;
	};
	
	// Deactivate chip select
	SPIFLASH_CS_IO = 1;

	// Restore SPI state
	SPIFLASH_SPICON1 = SPICON1Save;
}

/*****************************************************************************
  Function:
	void SPIFlashBeginWrite(DWORD dwAddr)

  Summary:
	Prepares the SPI Flash module for writing.

  Description:
	Prepares the SPI Flash module for writing.  Subsequent calls to 
	SPIFlashWrite or SPIFlashWriteArray will begin at this location and 
	continue sequentially.
	
	SPI Flash

  Precondition:
	SPIFlashInit has been called.

  Parameters:
	dwAddr - Address where the writing will begin

  Returns:
	None

  Remarks:
	Flash parts have large sector sizes, and can only erase entire sectors
	at once.  The SST parts for which this library was written have sectors
	that are 4kB in size.  Your application must ensure that writes begin on 
	a sector boundary so that the SPIFlashWrite functions will erase the 
	sector before attempting to write.  Entire sectors need not be written
	at once, so applications can begin writing to the front of a sector, 
	perform other tasks, then later call SPIFlashBeginWrite and point to an
	address in this sector that has not yet been programmed.  However, care
	must taken to ensure that writes are not attempted on addresses that are
	not in the erased state.  The chip will provide no indication that the
	write has failed, and will silently ignore the command.
  ***************************************************************************/
void SPIFlashBeginWrite(DWORD dwAddr)
{
	dwWriteAddr = dwAddr;
}

/*****************************************************************************
  Function:
	void SPIFlashWrite(BYTE vData)

  Summary:
	Writes a byte to the SPI Flash part.

  Description:
	This function writes a byte to the SPI Flash part.  If the current 
	address pointer indicates the beginning of a 4kB sector, the entire
	sector will first be erased to allow writes to proceed.  If the current
	address pointer indicates elsewhere, it will be assumed that the sector
	has already been erased.  If this is not true, the chip will silently
	ignore the write command.

  Precondition:
	SPIFlashInit and SPIFlashBeginWrite have been called, and the current
	address is either the front of a 4kB sector or has already been erased.

  Parameters:
	vData - The byte to write to the next memory location.

  Returns:
	None

  Remarks:
	See Remarks in SPIFlashBeginWrite for important information about Flash
	memory parts.
  ***************************************************************************/
void SPIFlashWrite(BYTE vData)
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
	SPICON1Save = SPIFLASH_SPICON1;
	SPIFLASH_SPICON1 = PROPER_SPICON1;
	
	// If address is a 4k boundary, erase a sector first
	if((dwWriteAddr & SPI_FLASH_SECTOR_MASK) == 0)
		_EraseSector(dwWriteAddr);
	
	// Enable writing
	_SendCmd(WREN);
	
	// Activate the chip select
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SPI_IF = 0;
	
	// Issue WRITE command with address
	SPIFLASH_SSPBUF = WRITE;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[2];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[1];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[0];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Write the byte
	SPIFLASH_SSPBUF = vData;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	dwWriteAddr++;
	
	// Deactivate chip select and wait for write to complete
	SPIFLASH_CS_IO = 1;
	_WaitWhileBusy();
	
	// Restore SPI state
	SPIFLASH_SPICON1 = SPICON1Save;
}

/*****************************************************************************
  Function:
	void SPIFlashWriteArray(BYTE* vData, WORD wLen)

  Summary:
	Writes an array of bytes to the SPI Flash part.

  Description:
	This function writes an array of bytes to the SPI Flash part.  When the
	address pointer crosses a 4kB sector boundary (and has more data to write),
	the next sector will automatically be erased.  If the current address 
	pointer indicates an address that is not a sector boundary and is not 
	already erased, the chip will silently ignore the write command until the
	next sector boundary is crossed.
	
  Precondition:
	SPIFlashInit and SPIFlashBeginWrite have been called, and the current
	address is either the front of a 4kB sector or has already been erased.

  Parameters:
	vData - The array to write to the next memory location
	wLen - The length of the data to be written

  Returns:
	None

  Remarks:
	See Remarks in SPIFlashBeginWrite for important information about Flash
	memory parts.
  ***************************************************************************/
void SPIFlashWriteArray(BYTE* vData, WORD wLen)
{
	BYTE Dummy;
	#if defined(__18CXX)
	BYTE SPICON1Save;
	#elif defined(__C30__)
	WORD SPICON1Save;
	#else
	DWORD SPICON1Save;
	#endif
	BOOL isStarted;

	// Save SPI state (clock speed)
	SPICON1Save = SPIFLASH_SPICON1;
	SPIFLASH_SPICON1 = PROPER_SPICON1;
	
	// If starting at an odd address, write a single byte
	if((dwWriteAddr & 0x01) && wLen)
	{
		SPIFlashWrite(*vData);
		vData++;
		wLen--;
	}
	
	isStarted = FALSE;
	
	// Loop over all remaining WORDs
	while(wLen > 1)
	{
		// Don't do anything until chip is ready
		_WaitWhileBusy();
				
		// If address is a sector boundary
		if((dwWriteAddr & SPI_FLASH_SECTOR_MASK) == 0)
			_EraseSector(dwWriteAddr);

		// If not yet started, initiate AAI mode
		if(!isStarted)
		{
			// Enable writing
			_SendCmd(WREN);
			
			// Activate the chip select
			SPIFLASH_CS_IO = 0;
			SPIFLASH_SPI_IF = 0;
			
			// Issue WRITE_STREAM command with address
			SPIFLASH_SSPBUF = WRITE_STREAM;
			while(!SPIFLASH_SPI_IF);
			Dummy = SPIFLASH_SSPBUF;
			SPIFLASH_SPI_IF = 0;
			SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[2];
			while(!SPIFLASH_SPI_IF);
			Dummy = SPIFLASH_SSPBUF;
			SPIFLASH_SPI_IF = 0;
			SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[1];
			while(!SPIFLASH_SPI_IF);
			Dummy = SPIFLASH_SSPBUF;
			SPIFLASH_SPI_IF = 0;
			SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[0];
			while(!SPIFLASH_SPI_IF);
			Dummy = SPIFLASH_SSPBUF;
			SPIFLASH_SPI_IF = 0;
			
			isStarted = TRUE;
		}
		// Otherwise, just write the AAI command again
		else
		{
			// Assert the chip select pin
			SPIFLASH_CS_IO = 0;
			SPIFLASH_SPI_IF = 0;
			
			// Issue the WRITE_STREAM command for continuation
			SPIFLASH_SSPBUF = WRITE_STREAM;
			while(!SPIFLASH_SPI_IF);
			Dummy = SPIFLASH_SSPBUF;
			SPIFLASH_SPI_IF = 0;
		}
	
		// Write the two bytes
		SPIFLASH_SSPBUF = *vData++;
		while(!SPIFLASH_SPI_IF);
		Dummy = SPIFLASH_SSPBUF;
		SPIFLASH_SPI_IF = 0;
		SPIFLASH_SSPBUF = *vData++;
		while(!SPIFLASH_SPI_IF);
		Dummy = SPIFLASH_SSPBUF;
		SPIFLASH_SPI_IF = 0;
		
		// Release the chip select to begin the write
		SPIFLASH_CS_IO = 1;
	
		// Increment the address, decrement length
		dwWriteAddr += 2;
		wLen -= 2;
		
		// If a boundary was reached, end the write
		if((dwWriteAddr & SPI_FLASH_SECTOR_MASK) == 0)
		{
			_WaitWhileBusy();
			_SendCmd(WRDI);
			isStarted = FALSE;
		}
	}
	
	// Wait for write to complete, then exit AAI mode
	_WaitWhileBusy();
	_SendCmd(WRDI);
	
	// If a byte remains, write the odd address
	if(wLen)
		SPIFlashWrite(*vData);

	// Restore SPI state
	SPIFLASH_SPICON1 = SPICON1Save;
}


/*****************************************************************************
  Function:
	void _EraseSector(DWORD dwAddr)

  Summary:
	Erases a 4kB sector.

  Description:
	This function erases a 4kB sector in the Flash part.  It is called
	internally by the SPIFlashWrite functions whenever a write is attempted
	on the first byte in a sector.
	
  Precondition:
	SPIFlashInit has been called.

  Parameters:
	dwAddr - The address of the sector to be erased.

  Returns:
	None

  Remarks:
	See Remarks in SPIFlashBeginWrite for important information about Flash
	memory parts.
  ***************************************************************************/
void _EraseSector(DWORD dwAddr)
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
	SPICON1Save = SPIFLASH_SPICON1;
	SPIFLASH_SPICON1 = PROPER_SPICON1;
			
	// Enable writing
	_SendCmd(WREN);
	
	// Activate the chip select
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SPI_IF = 0;
	
	// Issue ERASE_4K command with address
	SPIFLASH_SSPBUF = ERASE_4K;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[2];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[1];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	SPIFLASH_SSPBUF = ((BYTE*)&dwWriteAddr)[0];
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Deactivate chip select to perform the erase
	SPIFLASH_CS_IO = 1;
	
	// Wait for erase to complete
	_WaitWhileBusy();

	// Restore SPI state
	SPIFLASH_SPICON1 = SPICON1Save;
}

/*****************************************************************************
  Function:
	void _SendCmd(BYTE cmd)

  Summary:
	Sends a single-byte command to the SPI Flash part.

  Description:
	This function sends a single-byte command to the SPI Flash part.  It is
	used for commands such as WREN, WRDI, and EWSR that must have the chip
	select activated, then deactivated immediately after the command is 
	transmitted.
	
  Precondition:
	SPIFlashInit has been called.

  Parameters:
	cmd - The single-byte command code to send

  Returns:
	None
  ***************************************************************************/
void _SendCmd(BYTE cmd)
{
	// Activate chip select
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SPI_IF = 0;
	
	// Send Read Status Register instruction
	SPIFLASH_SSPBUF = cmd;
	while(!SPIFLASH_SPI_IF);
	cmd = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Deactivate chip select
	SPIFLASH_CS_IO = 1;
}


/*****************************************************************************
  Function:
	void _WaitWhileBusy(void)

  Summary:
	Waits for the SPI Flash part to indicate it is idle.

  Description:
	This function waits for the SPI Flash part to indicate it is idle.  It is
	used in the programming functions to wait for operations to complete.
	
  Precondition:
	SPIFlashInit has been called.

  Parameters:
	None

  Returns:
	None
  ***************************************************************************/
void _WaitWhileBusy(void)
{
	BYTE Dummy;

	// Activate chip select
	SPIFLASH_CS_IO = 0;
	SPIFLASH_SPI_IF = 0;
	
	// Send Read Status Register instruction
	SPIFLASH_SSPBUF = RDSR;
	while(!SPIFLASH_SPI_IF);
	Dummy = SPIFLASH_SSPBUF;
	SPIFLASH_SPI_IF = 0;
	
	// Poll the BUSY bit
	do
	{
		SPIFLASH_SSPBUF = 0x00;
		while(!SPIFLASH_SPI_IF);
		Dummy = SPIFLASH_SSPBUF;
		SPIFLASH_SPI_IF = 0;
	} while(Dummy & BUSY);
	
	// Deactivate chip select
	SPIFLASH_CS_IO = 1;
}

/*****************************************************************************
  Function:
	void _GetStatus()

  Summary:
	Reads the status register of the part.

  Description:
	This function reads the status register of the part.  It was written 
	for debugging purposes, and is not needed for normal operation.  Place
	a breakpoint at the last instruction and check the "status" variable to
	see the result.
	
  Precondition:
	SPIFlashInit has been called.

  Parameters:
	None

  Returns:
	None
  ***************************************************************************/
//void _GetStatus()
//{
//	static BYTE statuses[16], Dummy;
//	static BYTE *status = statuses;
//	
//	// Activate chip select
//	SPIFLASH_CS_IO = 0;
//	SPIFLASH_SPI_IF = 0;
//	
//	// Send Read Status Register instruction
//	SPIFLASH_SSPBUF = RDSR;
//	while(!SPIFLASH_SPI_IF);
//	Dummy = SPIFLASH_SSPBUF;
//	SPIFLASH_SPI_IF = 0;
//	
//	SPIFLASH_SSPBUF = 0x00;
//	while(!SPIFLASH_SPI_IF);
//	*status = SPIFLASH_SSPBUF;
//	status++;
//	SPIFLASH_SPI_IF = 0;
//	
//	// Deactivate chip select
//	SPIFLASH_CS_IO = 1;
//	
//	if(status == &statuses[10])
//		statuses[15] = 0;
//}

#endif //#if defined(SPIFLASH_CS_TRIS)

