/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *	 -Reference: AN833
 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
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
 * Author              Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti		4/19/01		Original (Rev. 1.0)
 * Nilesh Rajbharti		2/09/02		Cleanup
 * Nilesh Rajbharti		5/22/02		Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti		7/9/02		Rev 2.1 (See version.log for detail)
 * Nilesh Rajbharti		4/7/03		Rev 2.11.01 (See version log for detail)
 * Howard Schlunder		10/1/04		Beta Rev 0.9 (See version log for detail)
 * Howard Schlunder		10/8/04		Beta Rev 0.9.1 Announce support added
 * Howard Schlunder		11/29/04	Beta Rev 0.9.2 (See version log for detail)
 * Howard Schlunder		2/10/05		Rev 2.5.0
 * Howard Schlunder		1/5/06		Rev 3.00
 * Howard Schlunder		1/18/06		Rev 3.01 ENC28J60 fixes to TCP, 
 *									UDP and ENC28J60 files
 * Howard Schlunder		3/01/06		Rev. 3.16 including 16-bit micro support
 * Howard Schlunder		4/12/06		Rev. 3.50 added LCD for Explorer 16
 * Howard Schlunder		6/19/06		Rev. 3.60 finished dsPIC30F support, added PICDEM.net 2 support
 * Howard Schlunder		8/02/06		Rev. 3.75 added beta DNS, NBNS, and HTTP client (GenericTCPClient.c) services
 * Howard Schlunder		12/28/06	Rev. 4.00RC added SMTP, Telnet, substantially modified TCP layer
 * Howard Schlunder		04/09/07	Rev. 4.02 added TCPPerformanceTest, UDPPerformanceTest, Reboot and fixed some bugs
 * Howard Schlunder		xx/xx/07	Rev. 4.03
 * HSchlunder & EWood	08/27/07	Rev. 4.11
 * HSchlunder & EWood	10/08/07	Rev. 4.13
 * HSchlunder & EWood	11/06/07	Rev. 4.16
 * HSchlunder & EWood	11/08/07	Rev. 4.17
 * HSchlunder & EWood	11/12/07	Rev. 4.18
 * HSchlunder & EWood	02/11/08	Rev. 4.19
 * HSchlunder & EWood   04/26/08    Rev. 4.50 Moved most code to other files for clarity
 * HSchlunder & EWood   07/24/08    Rev. 4.51
 ********************************************************************/
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"


// Include functions specific to this stack application
#include "MainDemo.h"

#include "tcp_cmd_task.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "hal_io_interface.h"
#include "plc_prase.h"

#include "debug.h"

#define  THISINFO        1
#define  THISERROR       1
#define  THISASSERT      1

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
BYTE AN0String[8];
BYTE myDHCPBindCount = 0xFF;
#if !defined(STACK_USE_DHCP_CLIENT)
	#define DHCPBindCount	(1)
#endif


// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);
static void ProcessIO(void);

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)
	#if defined(HI_TECH_C)
	void interrupt low_priority LowISR(void)
	#else
	#pragma interruptlow LowISR
	void LowISR(void)
	#endif
	{
	    TickUpdate();
	}
	
	#if defined(HI_TECH_C)
	void interrupt HighISR(void)
	#else
	#pragma interruptlow HighISR
	void HighISR(void)
	#endif
	{
	    #if defined(STACK_USE_UART2TCP_BRIDGE)
		UART2TCPBridgeISR();
		#endif
	}
	
	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif

// C30 and C32 Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
#elif defined(__C30__)
	void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
	{
	    Nop();
		Nop();
	}
	void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
	{
	    Nop();
		Nop();
	}
	
#elif defined(__C32__)
	void _general_exception_handler(unsigned cause, unsigned status)
	{
		Nop();
		Nop();
	}
#endif

/***************/
/*  ��ֲ�ı��� */
/***************/
unsigned char led_reg;

void DebugStringHex(const char * str,unsigned long num,const char * tail);

//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
    static TICK t = 0;

    // Initialize application specific hardware
    InitializeBoard();

#if defined(STACK_USE_DS18B20)
	DS18B20_Init();
#endif

#ifdef STACK_USE_DS1302
	DS1302_Init();
#endif

	

	#if defined(USE_LCD)
	// Initialize and display the stack version on the LCD
	LCDInit();
	DelayMs(100);
	strcpypgm2ram((char*)LCDText, "TCPStack " VERSION "  "
		"                ");
	LCDUpdate();
	#endif

	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
	MPFSInit();
	#endif

	Io_Out_PowerInit();  //��Ҫ�õ�STACK_USE_DS1302����

	// Initialize Stack and application related NV variables into AppConfig.
	putrsUART((ROM char*)"\r\n InitAppConfig().");
	InitAppConfig();

    // Initiates board setup process if button is depressed 
	// on startup
    if(0) //BUTTON0_IO == 1u)
    {
		#if (defined(MPFS_USE_EEPROM) || defined(MPFS_USE_SPI_FLASH)) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
		// Invalidate the EEPROM contents if BUTTON0 is held down for more than 4 seconds
		TICK StartTime = TickGet();
		LED_PUT(0x00);

		putrsUART((ROM char*)"\r\n init board default config.");
				
		while(BUTTON0_IO == 0u)
		{
			if(TickGet() - StartTime > 4*TICK_SECOND)
			{
				#if defined(MPFS_USE_EEPROM)
			    XEEBeginWrite(0x0000);
			    XEEWrite(0xFF);
			    XEEEndWrite();
			    #elif defined(MPFS_USE_SPI_FLASH)
			    SPIFlashBeginWrite(0x00);
			    SPIFlashWrite(0xFF);
			    #endif
			    
				#if defined(STACK_USE_UART)
				putrsUART("\r\n\r\nBUTTON0 held for more than 4 seconds.  Default settings restored.\r\n\r\n");
				#endif

				LED_PUT(0x0F);
				while((LONG)(TickGet() - StartTime) <= (LONG)(9*TICK_SECOND/2));
				LED_PUT(0x00);
				while(BUTTON0_IO == 0u);
				Reset();
				break;
			}
		}
		#endif

		#if defined(STACK_USE_UART)
        DoUARTConfig();
		#endif
    }

	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
	putrsUART((ROM char*)"\r\n init statck.");
    StackInit();

	// Initialize any application-specific modules or functions/
	// For this demo application, this only includes the
	// UART 2 TCP Bridge
	#if defined(STACK_USE_UART2TCP_BRIDGE)
	putrsUART((ROM char*)"\r\n UART2TCPBridgeInit.");
	UART2TCPBridgeInit();
	#endif

	// Now that all items are initialized, begin the co-operative
	// multitasking loop.  This infinite loop will continuously 
	// execute all stack-related tasks, as well as your own
	// application's functions.  Custom functions should be added
	// at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
	putrsUART((ROM char*)"\r\n run in while loop.");

	set_led_flash(20,1500,0);

    while(1)
    {
        // Blink LED0 (right most one) every second.
        if(TickGet() - t >= TICK_SECOND/2ul)
        {
            t = TickGet();
            LED0_IO ^= 1;
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        
        // This tasks invokes each of the core stack application tasks
        StackApplications();

		// Process application specific tasks here.
		// For this demo app, this will include the Generic TCP 
		// client and servers, and the SNMP, Ping, and SNMP Trap
		// demos.  Following that, we will process any IO from
		// the inputs on the board itself.
		// Any custom modules or processing you need to do should
		// go here.
		#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)
		GenericTCPClient();
		#endif
		
		#if defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE)
		GenericTCPServer();
		#endif
		
		#if defined(STACK_USE_SMTP_CLIENT)
		SMTPDemo();
		#endif
		
		#if defined(STACK_USE_ICMP_CLIENT)
		PingDemo();
		#endif
		
		#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
		SNMPTrapDemo();
		#endif
		
		#if defined(STACK_USE_BERKELEY_API)
		BerkeleyTCPClientDemo();
		BerkeleyTCPServerDemo();
		BerkeleyUDPClientDemo();
		#endif

		ProcessIO();

        #if DEBUG_ON
	    //DebugTask();
		//DebugTcpTask();
		DiscoverTask();
		ResetTask();
		TaskLedFlash();
        #endif

		Tcp0CmdTask();
		Tcp1CmdTask();
		Tcp2CmdTask();

		//PLCָ���ϵͳ

        // If the DHCP lease has changed recently, write the new
        // IP address to the LCD display, UART, and Announce service
		if(DHCPBindCount != myDHCPBindCount)
		{
			myDHCPBindCount = DHCPBindCount;

			#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"\r\nNew IP Address: ");
			#endif

			DisplayIPValue(AppConfig.MyIPAddr);

			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif
		}

	}
}

// Writes an IP address to the LCD display and the UART as available
void DisplayIPValue(IP_ADDR IPVal)
{
//	printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
    BYTE IPDigit[4];
	BYTE i;
#ifdef USE_LCD
	BYTE j;
	BYTE LCDPos=16;
#endif

	for(i = 0; i < sizeof(IP_ADDR); i++)
	{
	    uitoa((WORD)IPVal.v[i], IPDigit);

		#if defined(STACK_USE_UART)
			putsUART(IPDigit);
		#endif

		#ifdef USE_LCD
			for(j = 0; j < strlen((char*)IPDigit); j++)
			{
				LCDText[LCDPos++] = IPDigit[j];
			}
			if(i == sizeof(IP_ADDR)-1)
				break;
			LCDText[LCDPos++] = '.';
		#else
			if(i == sizeof(IP_ADDR)-1)
				break;
		#endif

		#if defined(STACK_USE_UART)
			while(BusyUART());
			WriteUART('.');
		#endif
	}

	#ifdef USE_LCD
		if(LCDPos < 32u)
			LCDText[LCDPos] = 0;
		LCDUpdate();
	#endif
}

// Processes A/D data from the potentiometer
static void ProcessIO(void)
{
#if defined(__C30__) || defined(__C32__)
    // Convert potentiometer result into ASCII string
    uitoa((WORD)ADC1BUF0, AN0String);
#else
    // AN0 should already be set up as an analog input
    ADCON0bits.GO = 1;

    // Wait until A/D conversion is done
    while(ADCON0bits.GO);

	// AD converter errata work around (ex: PIC18F87J10 A2)
	#if !defined(__18F87J50) && !defined(_18F87J50)
		PRODL = ADCON2;
		ADCON2bits.ADCS0 = 1;
		ADCON2bits.ADCS1 = 1;
		ADCON2 = PRODL;
	#endif

    // Convert 10-bit value into ASCII string
    uitoa(*((WORD*)(&ADRESL)), AN0String);
#endif
}

/*********************************************************************
 * Function:        void InitializeBoard(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize board specific hardware.
 *
 * Note:            None
 ********************************************************************/
static void InitializeBoard(void)
{	
	// LEDs
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED2_TRIS = 0;
	LED3_TRIS = 0;
	LED4_TRIS = 0;
	LED5_TRIS = 0;
	LED6_TRIS = 0;
#if !defined(EXPLORER_16)	// Pin multiplexed with a button on EXPLORER_16 
	LED7_TRIS = 0;
#endif


	RUN_LED_TRIS = 0;
	RUN_LED_IO = 1;
	
	IP_CONFIG_TRIS = 1;  //�����

	//RELAY��ʼ��
	RELAY_OUT_0 = 1;
	RELAY_OUT_1 = 1;
	RELAY_OUT_2 = 1;
	RELAY_OUT_3 = 1;
	RELAY_OUT_4 = 1;
	RELAY_OUT_5 = 1;
	RELAY_OUT_6 = 1;
	RELAY_OUT_TRIS_0 = 0;
	RELAY_OUT_TRIS_1 = 0;
	RELAY_OUT_TRIS_2 = 0;
	RELAY_OUT_TRIS_3 = 0;
	RELAY_OUT_TRIS_4 = 0;
	RELAY_OUT_TRIS_5 = 0;
	RELAY_OUT_TRIS_6 = 0;
	//����
	DIG_INPUT_TRIS_0 = 1;
	DIG_INPUT_TRIS_1 = 1;
	DIG_INPUT_TRIS_2 = 1;
	DIG_INPUT_TRIS_3 = 1;
	DIG_INPUT_TRIS_4 = 1;
	DIG_INPUT_TRIS_5 = 1;
	DIG_INPUT_TRIS_6 = 1;
	DIG_INPUT_TRIS_7 = 1;
	DIG_INPUT_TRIS_8 = 1;

	//LED_PUT(0x00);

#if defined(__18CXX)
	// Enable 4x/5x PLL on PIC18F87J10, PIC18F97J60, etc.
    OSCTUNE = 0x40;

	// Set up analog features of PORTA

	// PICDEM.net 2 board has POT on AN2, Temp Sensor on AN3
	#if defined(PICDEMNET2)
		ADCON0 = 0x09;		// ADON, Channel 2
		ADCON1 = 0x0B;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are analog
	#elif defined(PICDEMZ)
		ADCON0 = 0x81;		// ADON, Channel 0, Fosc/32
		ADCON1 = 0x0F;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are all digital
	#else
		ADCON0 = 0x01;		// ADON, Channel 0
		ADCON1 = 0x0E;		// Vdd/Vss is +/-REF, AN0 is analog
	#endif
	#if defined(__18F87J50) || defined(_18F87J50)
		#define ADCON2		ADCON1
	#endif
	ADCON2 = 0xBE;			// Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)


    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

	// Configure USART
    TXSTA = 0x20;
    RCSTA = 0x90;

	// See if we can use the high baud rate setting
	#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif


	// Enable Interrupts
	RCONbits.IPEN = 1;		// Enable interrupt priorities
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Do a calibration A/D conversion
	#if defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10) || defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) || \
	defined(_18F87J10)  || defined(_18F86J15)  || defined(_18F86J10)  || defined(_18F85J15)  || defined(_18F85J10)  || defined(_18F67J10)  || defined(_18F66J15)  || defined(_18F66J10)  || defined(_18F65J15)  || defined(_18F65J10)  || defined(_18F97J60)  || defined(_18F96J65)  || defined(_18F96J60)  || defined(_18F87J60)  || defined(_18F86J65)  || defined(_18F86J60)  || defined(_18F67J60)  || defined(_18F66J65)  || defined(_18F66J60)
		ADCON0bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON0bits.ADCAL = 0;
	#endif

#else	// 16-bit C30 and and 32-bit C32
	#if defined(__PIC32MX__)
	{
		// Enable multi-vectored interrupts
		INTEnableSystemMultiVectoredInt();
		
		// Enable optimal performance
		SYSTEMConfigPerformance(GetSystemClock());
		mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
		
		// Disable JTAG port so we get our I/O pins back, but first
		// wait 50ms so if you want to reprogram the part with 
		// JTAG, you'll still have a tiny window before JTAG goes away
		DelayMs(50);
		DDPCONbits.JTAGEN = 0;
		LED_PUT(0x00);				// Turn the LEDs off
	}
	#endif

	#if defined(__dsPIC33F__) || defined(__PIC24H__)
		// Crank up the core frequency
		PLLFBD = 38;				// Multiply by 40 for 160MHz VCO output (8MHz XT oscillator)
		CLKDIV = 0x0000;			// FRC: divide by 2, PLLPOST: divide by 2, PLLPRE: divide by 2
	
		// Port I/O
		AD1PCFGHbits.PCFG23 = 1;	// Make RA7 (BUTTON1) a digital input

		// ADC
	    AD1CHS0 = 0;				// Input to AN0 (potentiometer)
		AD1PCFGLbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		AD1PCFGLbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
	#else	//defined(__PIC24F__) || defined(__PIC32MX__)
		// ADC
	    AD1CHS = 0;					// Input to AN0 (potentiometer)
		AD1PCFGbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
		#if defined(__32MX460F512L__)	// PIC32MX460F512L PIM has different pinout to accomodate USB module
			AD1PCFGbits.PCFG2 = 0;		// Disable digital input on AN2 (potentiometer)
		#else
			AD1PCFGbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		#endif
	#endif

	// ADC
	AD1CON1 = 0x84E4;			// Turn on, auto sample start, auto-convert, 12 bit mode (on parts with a 12bit A/D)
	AD1CON2 = 0x0404;			// AVdd, AVss, int every 2 conversions, MUXA only, scan
	AD1CON3 = 0x1003;			// 16 Tad auto-sample, Tad = 3*Tcy
	#if defined(__32MX460F512L__)	// PIC32MX460F512L PIM has different pinout to accomodate USB module
		AD1CSSL = 1<<2;				// Scan pot
	#else
		AD1CSSL = 1<<5;				// Scan pot
	#endif

	// UART
	#if defined(STACK_USE_UART)
		UARTTX_TRIS = 0;
		UARTRX_TRIS = 1;
		UMODE = 0x8000;			// Set UARTEN.  Note: this must be done before setting UTXEN

		#if defined(__C30__)
			USTA = 0x0400;		// UTXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#else	//defined(__C32__)
			USTA = 0x00001400;		// RXEN set, TXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#endif
	
		#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
		#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)
		#if (BAUD_ERROR_PRECENT > 3)
			#warning UART frequency error is worse than 3%
		#elif (BAUD_ERROR_PRECENT > 2)
			#warning UART frequency error is worse than 2%
		#endif
	
		UBRG = CLOSEST_UBRG_VALUE;
	#endif

#endif

#if defined(PIC24FJ64GA004_PIM)
	// Remove some LED outputs to regain other functions
	LED1_TRIS = 1;		// Multiplexed with BUTTON0
	LED5_TRIS = 1;		// Multiplexed with EEPROM CS
	LED7_TRIS = 1;		// Multiplexed with BUTTON1
	
	// Inputs
	RPINR19bits.U2RXR = 19;			//U2RX = RP19
	RPINR22bits.SDI2R = 20;			//SDI2 = RP20
	RPINR20bits.SDI1R = 17;			//SDI1 = RP17
	
	// Outputs
	RPOR12bits.RP25R = U2TX_IO;		//RP25 = U2TX  
	RPOR12bits.RP24R = SCK2OUT_IO; 	//RP24 = SCK2
	RPOR10bits.RP21R = SDO2_IO;		//RP21 = SDO2
	RPOR7bits.RP15R = SCK1OUT_IO; 	//RP15 = SCK1
	RPOR8bits.RP16R = SDO1_IO;		//RP16 = SDO1
	
	AD1PCFG = 0xFFFF;				//All digital inputs - POT and Temp are on same pin as SDO1/SDI1, which is needed for ENC28J60 commnications

	// Lock the PPS
	asm volatile (	"mov #OSCCON,w1 \n"
					"mov #0x46, w2 \n"
					"mov #0x57, w3 \n"
					"mov.b w2,[w1] \n"
					"mov.b w3,[w1] \n"
					"bset OSCCON, #6");
#endif

#if defined(DSPICDEM11)
	// Deselect the LCD controller (PIC18F252 onboard) to ensure there is no SPI2 contention
	LCDCTRL_CS_TRIS = 0;
	LCDCTRL_CS_IO = 1;

	// Hold the codec in reset to ensure there is no SPI2 contention
	CODEC_RST_TRIS = 0;
	CODEC_RST_IO = 0;
#endif

#if defined(SPIRAM_CS_TRIS)
	SPIRAMInit();
#endif
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// Uncomment these two pragmas for production MAC address 
// serialization if using C18. The MACROM=0x1FFF0 statement causes 
// the MAC address to be located at aboslute program memory address 
// 0x1FFF0 for easy auto-increment without recompiling the stack for 
// each device made.  Note, other compilers/linkers use a different 
// means of allocating variables at an absolute address.  Check your 
// compiler documentation for the right method.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
#if (defined(MPFS_USE_EEPROM) || defined(MPFS_USE_SPI_FLASH)) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    BYTE c;
    BYTE *p;
#endif

	//AppConfig.Flags.bIsDHCPEnabled = TRUE;
	//AppConfig.Flags.bInConfigMode = TRUE;
	AppConfig.Flags.bIsDHCPEnabled = FALSE;
	AppConfig.Flags.bInConfigMode = FALSE;

	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//	{
//		_prog_addressT MACAddressAddress;
//		MACAddressAddress.next = 0x157F8;
//		_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//	}
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;

	// Load the default NetBIOS Host Name
	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(AppConfig.NetBIOSName);

#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    p = (BYTE*)&AppConfig;

    XEEBeginRead(0x0000);
    c = XEERead();
    XEEEndRead();

    // When a record is saved, first byte is written as 0x60 to indicate
    // that a valid record was saved.  Note that older stack versions 
	// used 0x57.  This change has been made to so old EEPROM contents 
	// will get overwritten.  The AppConfig() structure has been changed,
	// resulting in parameter misalignment if still using old EEPROM 
	// contents.
    if(c == 0x60u)
    {
        XEEBeginRead(0x0001);
        for ( c = 0; c < sizeof(AppConfig); c++ )
            *p++ = XEERead();
        XEEEndRead();
    }
    else
        SaveAppConfig();

#elif defined(MPFS_USE_SPI_FLASH) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
	SPIFlashReadArray(0x00, &c, 1);
	if(c == 0x60u)
		SPIFlashReadArray(0x01, (BYTE*)&AppConfig, sizeof(AppConfig));
	else
		SaveAppConfig();
#endif
}

void ResetToDefaultConfig(void)
{
#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
	XEEBeginWrite(0x0000);
	XEEWrite(0x00);
	XEEEndWrite();
#elif defined(MPFS_USE_SPI_FLASH) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    SPIFlashBeginWrite(0x0000);
    SPIFlashWrite(0x00);
#endif
}

#if (defined(MPFS_USE_EEPROM) || defined(MPFS_USE_SPI_FLASH)) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
void SaveAppConfig(void)
{
	#if defined(MPFS_USE_EEPROM)

    BYTE c;
    BYTE *p;

    p = (BYTE*)&AppConfig;
    XEEBeginWrite(0x0000);
    XEEWrite(0x60);
    for ( c = 0; c < sizeof(AppConfig); c++ )
    {
        XEEWrite(*p++);
    }

    XEEEndWrite();
    
    #else

    SPIFlashBeginWrite(0x0000);
    SPIFlashWrite(0x60);
    SPIFlashWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    
    #endif
}
#endif

