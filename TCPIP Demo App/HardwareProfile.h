/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
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
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		10/03/06	Original, copied from Compiler.h
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

// Choose which hardware profile to compile for here.  See 
// the hardware profiles below for meaning of various options.  
//#define PICDEMNET2
//#define HPC_EXPLORER
//#define PICDEMZ
//#define PIC24FJ64GA004_PIM	// Explorer 16, but with the PIC24FJ64GA004 PIM module, which has significantly differnt pin mappings
//#define EXPLORER_16		// PIC24FJ128GA010, PIC24HJ256GP610, dsPIC33FJ256GP710 PIMs
//#define DSPICDEM11
//#define YOUR_BOARD

// If no hardware profiles are defined, assume that we are using 
// a Microchip demo board and try to auto-select the correct profile
// based on processor selected in MPLAB
#if !defined(PICDEMNET2) && !defined(HPC_EXPLORER) && !defined(PICDEMZ) && !defined(EXPLORER_16) && !defined(PIC24FJ64GA004_PIM) && !defined(DSPICDEM11) && !defined(PICDEMNET2) && !defined(INTERNET_RADIO) && !defined(YOUR_BOARD)
	#if defined(__18F97J60) || defined(_18F97J60)
		#define PICDEMNET2	//自动选择为PICDEMNET2
	#elif defined(__18F67J60) || defined(_18F67J60)
		#define INTERNET_RADIO
	#elif defined(__18F8722) || defined(__18F87J10) || defined(_18F8722) || defined(_18F87J10) || defined(__18F87J50) || defined(_18F87J50)
		#define HPC_EXPLORER
	#elif defined(__18F4620) || defined(_18F4620)
		#define PICDEMZ
	#elif defined(__PIC24FJ64GA004__)
		#define PIC24FJ64GA004_PIM
	#elif defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__) || defined(__PIC32MX__)
		#define EXPLORER_16
	#elif defined(__dsPIC30F__)
		#define DSPICDEM11
	#endif
#endif

// Set configuration fuses (but only once)
#if defined(THIS_IS_STACK_APPLICATION)
	#if defined(__18CXX)
		#if defined(__EXTENDED18__)
			#pragma config XINST=ON
		#elif !defined(HI_TECH_C)
			#pragma config XINST=OFF
		#endif
	
		#if defined(__18F8722)	//__表示MCC18编译器，_表示HIPICC18,后面都是以此来区分选用哪个编译器来编译的
			// PICDEM HPC Explorer board
			#pragma config OSC=HSPLL, FCMEN=OFF, IESO=OFF, PWRT=OFF, WDT=OFF, LVP=OFF
		#elif defined(_18F8722)	// HI-TECH PICC-18 compiler
			// PICDEM HPC Explorer board
			__CONFIG(1, HSPLL);
			__CONFIG(2, WDTDIS);
			__CONFIG(3, MCLREN);
			__CONFIG(4, XINSTDIS & LVPDIS);
		#elif defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10)
			// PICDEM HPC Explorer board
			#pragma config WDTEN=OFF, FOSC2=ON, FOSC=HSPLL
		#elif defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) 
			// PICDEM.net 2 or any other PIC18F97J60 family device
			#pragma config WDT=OFF, FOSC2=ON, FOSC=HSPLL, ETHLED=ON
		#elif defined(_18F97J60) || defined(_18F96J65) || defined(_18F96J60) || defined(_18F87J60) || defined(_18F86J65) || defined(_18F86J60) || defined(_18F67J60) || defined(_18F66J65) || defined(_18F66J60) 
			// PICDEM.net 2 board with HI-TECH PICC-18 compiler
			__CONFIG(1, WDTDIS & XINSTDIS);		//在这个硬件版本中（Ethernet-V2.0）我选用HIPICC18编译器来进行编译
			__CONFIG(2, HSPLL);
			__CONFIG(3, ETHLEDEN);
		#elif defined(__18F4620)	
			// PICDEM Z board
			#pragma config OSC=HSPLL, WDT=OFF, MCLRE=ON, PBADEN=OFF, LVP=OFF
		#endif
	#elif defined(__PIC24F__)
		// Explorer 16 board
		_CONFIG2(FNOSC_PRIPLL & POSCMOD_XT)		// Primary XT OSC with 4x PLL
		_CONFIG1(JTAGEN_OFF & FWDTEN_OFF)		// JTAG off, watchdog timer off
	#elif defined(__dsPIC33F__) || defined(__PIC24H__)
		// Explorer 16 board
		_FOSCSEL(FNOSC_PRIPLL)			// PLL enabled
		_FOSC(OSCIOFNC_OFF & POSCMD_XT)	// XT Osc
		_FWDT(FWDTEN_OFF)				// Disable Watchdog timer
		// JTAG should be disabled as well
	#elif defined(__dsPIC30F__)
		// dsPICDEM 1.1 board
		_FOSC(XT_PLL16)					// XT Osc + 16X PLL
		_FWDT(WDT_OFF)					// Disable Watchdog timer
		_FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
	#elif defined(__PIC32MX__)
		#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FWDTEN = OFF, FPBDIV = DIV_1, POSCMOD = XT, FNOSC = PRIPLL, CP = OFF
	#endif
#endif // Prevent more than one set of config fuse definitions

// Clock frequency value.
// This value is used to calculate Tick Counter value
#if defined(__18CXX)
	// All PIC18 processors
	#if defined(PICDEMNET2) || defined(INTERNET_RADIO)		//因为采用了25M晶振，CUP内部频率被倍频至41666667Hz
		#define GetSystemClock()		(41666667ul)      // Hz
		#define GetInstructionClock()	(GetSystemClock()/4)
		#define GetPeripheralClock()	GetInstructionClock()
	#elif defined(PICDEMZ)
		#define GetSystemClock()		(16000000ul)      // Hz
		#define GetInstructionClock()	(GetSystemClock()/4)
		#define GetPeripheralClock()	GetInstructionClock()
	#else
		#define GetSystemClock()		(40000000ul)      // Hz
		#define GetInstructionClock()	(GetSystemClock()/4)
		#define GetPeripheralClock()	GetInstructionClock()
	#endif
#elif defined(__PIC24F__)	
	// PIC24F processor
	#define GetSystemClock()		(32000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/2)
	#define GetPeripheralClock()	GetInstructionClock()
#elif defined(__PIC24H__)	
	// PIC24H processor
	#define GetSystemClock()		(80000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/2)
	#define GetPeripheralClock()	GetInstructionClock()
#elif defined(__dsPIC33F__)	
	// dsPIC33F processor
	#define GetSystemClock()		(80000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/2)
	#define GetPeripheralClock()	GetInstructionClock()
#elif defined(__dsPIC30F__)
	// dsPIC30F processor
	#define GetSystemClock()		(117920000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/4)
	#define GetPeripheralClock()	GetInstructionClock()
#elif defined(__PIC32MX__)
	// PIC32MX processor
	#define GetSystemClock()		(80000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/1)
	#define GetPeripheralClock()	(GetInstructionClock()/1)	// Set your divider according to your Peripheral Bus Frequency configuration fuse setting
#endif

// Hardware mappings
#if defined(HPC_EXPLORER) && !defined(HI_TECH_C)
// PICDEM HPC Explorer + Ethernet PICtail
	// I/O pins
	#define LED0_TRIS			(TRISDbits.TRISD0)
	#define LED0_IO				(LATDbits.LATD0)
	#define LED1_TRIS			(TRISDbits.TRISD1)
	#define LED1_IO				(LATDbits.LATD1)
	#define LED2_TRIS			(TRISDbits.TRISD2)
	#define LED2_IO				(LATDbits.LATD2)
	#define LED3_TRIS			(TRISDbits.TRISD3)
	#define LED3_IO				(LATDbits.LATD3)
	#define LED4_TRIS			(TRISDbits.TRISD4)
	#define LED4_IO				(LATDbits.LATD4)
	#define LED5_TRIS			(TRISDbits.TRISD5)
	#define LED5_IO				(LATDbits.LATD5)
	#define LED6_TRIS			(TRISDbits.TRISD6)
	#define LED6_IO				(LATDbits.LATD6)
	#define LED7_TRIS			(TRISDbits.TRISD7)
	#define LED7_IO				(LATDbits.LATD7)
	#define LED_GET()			(LATD)
	#define LED_PUT(a)			(LATD = (a))


	#define BUTTON0_TRIS		(TRISBbits.TRISB0)
	#define	BUTTON0_IO			(PORTBbits.RB0)
	#define BUTTON1_TRIS		(TRISBbits.TRISB0)	// No Button1 on this board, remap to Button0
	#define	BUTTON1_IO			(PORTBbits.RB0)
	#define BUTTON2_TRIS		(TRISBbits.TRISB0)	// No Button2 on this board, remap to Button0
	#define	BUTTON2_IO			(PORTBbits.RB0)
	#define BUTTON3_TRIS		(TRISBbits.TRISB0)	// No Button3 on this board, remap to Button0
	#define	BUTTON3_IO			(PORTBbits.RB0)

	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		(TRISBbits.TRISB5)
	#define ENC_RST_IO			(LATBbits.LATB5)
	#define ENC_CS_TRIS			(TRISBbits.TRISB3)
	#define ENC_CS_IO			(LATBbits.LATB3)
	#define ENC_SCK_TRIS		(TRISCbits.TRISC3)
	#define ENC_SDI_TRIS		(TRISCbits.TRISC4)
	#define ENC_SDO_TRIS		(TRISCbits.TRISC5)
	#define ENC_SPI_IF			(PIR1bits.SSPIF)
	#define ENC_SSPBUF			(SSPBUF)
	#define ENC_SPISTAT			(SSP1STAT)
	#define ENC_SPISTATbits		(SSP1STATbits)
	#define ENC_SPICON1			(SSP1CON1)
	#define ENC_SPICON1bits		(SSP1CON1bits)
	#define ENC_SPICON2			(SSP1CON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISBbits.TRISB4)
	#define EEPROM_CS_IO		(LATBbits.LATB4)
	#define EEPROM_SCK_TRIS		(TRISCbits.TRISC3)
	#define EEPROM_SDI_TRIS		(TRISCbits.TRISC4)
	#define EEPROM_SDO_TRIS		(TRISCbits.TRISC5)
	#define EEPROM_SPI_IF		(PIR1bits.SSPIF)
	#define EEPROM_SSPBUF		(SSPBUF)
	#define EEPROM_SPICON1		(SSP1CON1)
	#define EEPROM_SPICON1bits	(SSP1CON1bits)
	#define EEPROM_SPICON2		(SSP1CON2)
	#define EEPROM_SPISTAT		(SSP1STAT)
	#define EEPROM_SPISTATbits	(SSP1STATbits)

	// Register name fix up for some processors (ex: PIC18F87J10)
	#define SPBRGH				SPBRGH1

#elif defined(HPC_EXPLORER) && defined(HI_TECH_C)
// PICDEM HPC Explorer + Ethernet PICtail
	typedef struct
	{
	    unsigned char BF:1;
	    unsigned char UA:1;
	    unsigned char R_W:1;
	    unsigned char S:1;
	    unsigned char P:1;
	    unsigned char D_A:1;
	    unsigned char CKE:1;
	    unsigned char SMP:1;
	} SSPSTATbits;
	typedef struct 
	{
	    unsigned char RBIF:1;
	    unsigned char INT0IF:1;
	    unsigned char TMR0IF:1;
		unsigned char RBIE:1;
	    unsigned char INT0IE:1;
	    unsigned char TMR0IE:1;
	    unsigned char GIEL:1;
	    unsigned char GIEH:1;
	} INTCONbits;
	typedef struct 
	{
	    unsigned char RBIP:1;
	    unsigned char INT3IP:1;
	    unsigned char TMR0IP:1;
	    unsigned char INTEDG3:1;
	    unsigned char INTEDG2:1;
	    unsigned char INTEDG1:1;
	    unsigned char INTEDG0:1;
	    unsigned char RBPU:1;
	} INTCON2bits;
	typedef struct 
	{
	    unsigned char ADON:1;
	    unsigned char GO:1;
	    unsigned char CHS0:1;
	    unsigned char CHS1:1;
	    unsigned char CHS2:1;
	    unsigned char CHS3:1;
		unsigned char :1;
		unsigned char ADCAL:1;
	} ADCON0bits;
	typedef struct 
	{
		unsigned char ADCS0:1;
		unsigned char ADCS1:1;
		unsigned char ADCS2:1;
		unsigned char ACQT0:1;
		unsigned char ACQT1:1;
		unsigned char ACQT2:1;
		unsigned char :1;
		unsigned char ADFM:1;
	} ADCON2bits;
	typedef struct 
	{
	    unsigned char TMR1IF:1;
	    unsigned char TMR2IF:1;
	    unsigned char CCP1IF:1;
	    unsigned char SSPIF:1;
	    unsigned char TXIF:1;
	    unsigned char RCIF:1;
	    unsigned char ADIF:1;
	    unsigned char PSPIF:1;
	} PIR1bits;
	typedef struct 
	{
	    unsigned char TMR1IE:1;
	    unsigned char TMR2IE:1;
	    unsigned char CCP1IE:1;
	    unsigned char SSPIE:1;
	    unsigned char TXIE:1;
	    unsigned char RCIE:1;
	    unsigned char ADIE:1;
	    unsigned char PSPIE:1;
	} PIE1bits;
	typedef struct
	{
		unsigned char TMR1IP:1;
		unsigned char TMR2IP:1;
		unsigned char CCP1IP:1;
		unsigned char SSP1IP:1;
		unsigned char TXIP:1;
		unsigned char RCIP:1;
		unsigned char ADIP:1;
		unsigned char PSPIP:1;
	} IPR1bits;
	typedef struct
	{
		unsigned char BOR:1;
		unsigned char POR:1;
		unsigned char PD:1;
		unsigned char TO:1;
		unsigned char RI:1;
		unsigned char CM:1;
		unsigned char :1;
		unsigned char IPEN:1;
	} RCONbits;
	typedef struct
	{
	    unsigned char T0PS0:1;
	    unsigned char T0PS1:1;
	    unsigned char T0PS2:1;
	    unsigned char PSA:1;
	    unsigned char T0SE:1;
	    unsigned char T0CS:1;
	    unsigned char T08BIT:1;
	    unsigned char TMR0ON:1;
	} T0CONbits;
	typedef struct
	{
	    unsigned char TX9D:1;
	    unsigned char TRMT:1;
	    unsigned char BRGH:1;
	    unsigned char SENDB:1;
	    unsigned char SYNC:1;
	    unsigned char TXEN:1;
	    unsigned char TX9:1;
	    unsigned char CSRC:1;
	} TXSTAbits;
	typedef struct
	{
	    unsigned char RX9D:1;
	    unsigned char OERR:1;
	    unsigned char FERR:1;
	    unsigned char ADDEN:1;
	    unsigned char CREN:1;
	    unsigned char SREN:1;
	    unsigned char RX9:1;
	    unsigned char SPEN:1;
	} RCSTAbits;
	typedef struct {
	  unsigned ABDEN:1;
	  unsigned WUE:1;
	  unsigned :1;
	  unsigned BRG16:1;
	  unsigned TXCKP:1;
	  unsigned RXDTP:1;
	  unsigned RCIDL:1;
	  unsigned ABDOVF:1;
	} BAUDCONbits;
	
	#define TXSTA				TXSTA1
	#define RCSTA				RCSTA1
	#define SPBRG				SPBRG1
	#define SPBRGH				SPBRGH1
	#define RCREG				RCREG1
	#define TXREG				TXREG1

	// I/O pins
	#define LED0_TRIS			(TRISD0)
	#define LED0_IO				(LATD0)
	#define LED1_TRIS			(TRISD1)
	#define LED1_IO				(LATD1)
	#define LED2_TRIS			(TRISD2)
	#define LED2_IO				(LATD2)
	#define LED3_TRIS			(TRISD3)
	#define LED3_IO				(LATD3)
	#define LED4_TRIS			(TRISD4)
	#define LED4_IO				(LATD4)
	#define LED5_TRIS			(TRISD5)
	#define LED5_IO				(LATD5)
	#define LED6_TRIS			(TRISD6)
	#define LED6_IO				(LATD6)
	#define LED7_TRIS			(TRISD7)
	#define LED7_IO				(LATD7)
	#define LED_GET()			(LATD)
	#define LED_PUT(a)			(LATD = (a))

	#define BUTTON0_TRIS		(TRISB0)
	#define	BUTTON0_IO			(RB0)
	#define BUTTON1_TRIS		(TRISB0)	// No Button1 on this board, remap to Button0
	#define	BUTTON1_IO			(RB0)
	#define BUTTON2_TRIS		(TRISB0)	// No Button2 on this board, remap to Button0
	#define	BUTTON2_IO			(RB0)
	#define BUTTON3_TRIS		(TRISB0)	// No Button3 on this board, remap to Button0
	#define	BUTTON3_IO			(RB0)

	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		(TRISB5)
	#define ENC_RST_IO			(LATB5)
	#define ENC_CS_TRIS			(TRISB3)
	#define ENC_CS_IO			(LATB3)
	#define ENC_SCK_TRIS		(TRISC3)
	#define ENC_SDI_TRIS		(TRISC4)
	#define ENC_SDO_TRIS		(TRISC5)
	#define ENC_SPI_IF			(SSP1IF)
	#define ENC_SSPBUF			(SSP1BUF)
	#define ENC_SPISTAT			(SSP1STAT)
	#define ENC_SPISTATbits		(*((SSPSTATbits*)&SSP1STAT))
	#define ENC_SPICON1			(SSP1CON1)
	#define ENC_SPICON1bits		(*((SSPCON1bits*)&SSP1CON))
	#define ENC_SPICON2			(SSP1CON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISB4)
	#define EEPROM_CS_IO		(LATB4)
	#define EEPROM_SCK_TRIS		(TRISC3)
	#define EEPROM_SDI_TRIS		(TRISC4)
	#define EEPROM_SDO_TRIS		(TRISC5)
	#define EEPROM_SPI_IF		(SSP1IF)
	#define EEPROM_SSPBUF		(SSP1BUF)
	#define EEPROM_SPICON1		(SSP1CON1)
	#define EEPROM_SPICON1bits	(*((SSPCON1bits*)&SSP1CON))
	#define EEPROM_SPICON2		(SSP1CON2)
	#define EEPROM_SPISTAT		(SSP1STAT)
	#define EEPROM_SPISTATbits	(*((SSPSTATbits*)&SSP1STAT))

	#define INTCONbits			(*((INTCONbits*)&INTCON))
	#define INTCON2bits			(*((INTCON2bits*)&INTCON2))
	#define ADCON0bits			(*((ADCON0bits*)&ADCON0))
	#define ADCON2bits			(*((ADCON2bits*)&ADCON2))
	#define PIR1bits			(*((PIR1bits*)&PIR1))
	#define PIE1bits			(*((PIE1bits*)&PIE1))
	#define IPR1bits			(*((IPR1bits*)&IPR1))
	#define RCONbits			(*((RCONbits*)&RCON))
	#define T0CONbits			(*((T0CONbits*)&T0CON))
	#define TXSTAbits			(*((TXSTAbits*)&TXSTA1))
	#define RCSTAbits			(*((RCSTAbits*)&RCSTA1))
	#define BAUDCONbits			(*((BAUDCONbits*)&BAUDCON1))
	
#elif defined(PICDEMZ) && !defined(HI_TECH_C)
// PICDEM Z + Ethernet PICtail
	// I/O pins
	#define LED0_TRIS			(TRISAbits.TRISA1)
	#define LED0_IO				(LATAbits.LATA1)
	#define LED1_TRIS			(TRISAbits.TRISA0)
	#define LED1_IO				(LATAbits.LATA0)
	#define LED2_TRIS			(TRISAbits.TRISA1)
	#define LED2_IO				(LATAbits.LATA1)
	#define LED3_TRIS			(TRISAbits.TRISA0)
	#define LED3_IO				(LATAbits.LATA0)
	#define LED4_TRIS			(TRISAbits.TRISA1)
	#define LED4_IO				(LATAbits.LATA1)
	#define LED5_TRIS			(TRISAbits.TRISA0)
	#define LED5_IO				(LATAbits.LATA0)
	#define LED6_TRIS			(TRISAbits.TRISA1)
	#define LED6_IO				(LATAbits.LATA1)
	#define LED7_TRIS			(TRISAbits.TRISA0)
	#define LED7_IO				(LATAbits.LATA0)
	#define LED_GET()			((LED1_IO<<1) | LED0_IO)
	#define LED_PUT(a)			do{BYTE vTemp = (a); LED0_IO = vTemp&0x1; LED1_IO = vTemp&0x2;} while(0)

	#define BUTTON0_TRIS		(TRISBbits.TRISB5)  // Button0 is connected to EEPROM_CS on this board, remap to Button1
	#define	BUTTON0_IO			(PORTBbits.RB5)
	#define BUTTON1_TRIS		(TRISBbits.TRISB5)
	#define	BUTTON1_IO			(PORTBbits.RB5)
	#define BUTTON2_TRIS		(TRISBbits.TRISB5)	// No Button2 on this board, remap to Button1
	#define	BUTTON2_IO			(PORTBbits.RB5)
	#define BUTTON3_TRIS		(TRISBbits.TRISB5)	// No Button3 on this board, remap to Button1
	#define	BUTTON3_IO			(PORTBbits.RB5)

	// ENC28J60 I/O pins
	#define ENC_CS_TRIS			(TRISBbits.TRISB3)
	#define ENC_CS_IO			(LATBbits.LATB3)
	#define ENC_SCK_TRIS		(TRISCbits.TRISC3)
	#define ENC_SDI_TRIS		(TRISCbits.TRISC4)
	#define ENC_SDO_TRIS		(TRISCbits.TRISC5)
	#define ENC_SPI_IF			(PIR1bits.SSPIF)
	#define ENC_SSPBUF			(SSPBUF)
	#define ENC_SPISTAT			(SSPSTAT)
	#define ENC_SPISTATbits		(SSPSTATbits)
	#define ENC_SPICON1			(SSPCON1)
	#define ENC_SPICON1bits		(SSPCON1bits)
	#define ENC_SPICON2			(SSPCON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISBbits.TRISB4)
	#define EEPROM_CS_IO		(LATBbits.LATB4)
	#define EEPROM_SCK_TRIS		(TRISCbits.TRISC3)
	#define EEPROM_SDI_TRIS		(TRISCbits.TRISC4)
	#define EEPROM_SDO_TRIS		(TRISCbits.TRISC5)
	#define EEPROM_SPI_IF		(PIR1bits.SSPIF)
	#define EEPROM_SSPBUF		(SSPBUF)
	#define EEPROM_SPICON1		(SSPCON1)
	#define EEPROM_SPICON1bits	(SSPCON1bits)
	#define EEPROM_SPICON2		(SSPCON2)
	#define EEPROM_SPISTAT		(SSPSTAT)
	#define EEPROM_SPISTATbits	(SSPSTATbits)

#elif defined(PIC24FJ64GA004_PIM)
// Explorer 16 + PIC24FJ64GA004 PIM + Ethernet PICtail Plus

	// Push Button I/O pins
	#define BUTTON3_TRIS		TRISAbits.TRISA10		// Mutliplexed with LED0
	#define BUTTON3_IO			PORTAbits.RA10
	#define BUTTON2_TRIS		TRISAbits.TRISA9		// Multiplexed with LED4
	#define BUTTON2_IO			PORTAbits.RA9
	#define BUTTON1_TRIS		TRISCbits.TRISC6		// Multiplexed with LED7
	#define BUTTON1_IO			PORTCbits.RC6
	#define BUTTON0_TRIS		TRISAbits.TRISA7		// Multiplexed with LED1
	#define BUTTON0_IO			PORTAbits.RA7
		
	// LED I/O pins
	#define LED0_TRIS			TRISAbits.TRISA10		// Multiplexed with BUTTON3
	#define LED0_IO				LATAbits.LATA10
	#define LED1_TRIS			TRISAbits.TRISA7		// Multiplexed with BUTTON0
	#define LED1_IO				LATAbits.LATA7				
	#define LED2_TRIS			TRISBbits.TRISB8		// Multiplexed with LCD_DATA4
	#define LED2_IO				LATBbits.LATB8
	#define LED3_TRIS			TRISBbits.TRISB9		// Multiplexed with LCD_DATA3
	#define LED3_IO				LATBbits.LATB9
	#define LED4_TRIS			TRISAbits.TRISA9		// Multiplexed with BUTTON2
	#define LED4_IO				LATAbits.LATA9
	#define LED5_TRIS			TRISAbits.TRISA8		// Multiplexed with EEPROM_CS
	#define LED5_IO				LATAbits.LATA8
	#define LED6_TRIS			TRISBbits.TRISB12		// Multiplexed with LCD_DATA0
	#define LED6_IO				LATBbits.LATB12
	#define LED7_TRIS			TRISCbits.TRISC6		// Multiplexed with BUTTON1
	#define LED7_IO				LATCbits.LATC6
	#define LED_GET()			(0)
	#define LED_PUT(a)			
	
	// UART I/O Mapping
	#define UARTTX_TRIS			(TRISCbits.TRISC9)
	#define UARTTX_IO			(PORTCbits.RC9)
	#define UARTRX_TRIS			(TRISCbits.TRISC3)
	#define UARTRX_IO			(PORTCbits.RC3)

	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		(TRISCbits.TRISC8)	// Not connected by default
	#define ENC_RST_IO			(PORTCbits.RC8)
	#define ENC_CS_TRIS			(TRISBbits.TRISB3)
	#define ENC_CS_IO			(PORTBbits.RB3)
	// SPI SCK, SDI, SDO pins are automatically controlled by the 
	// PIC24/dsPIC/PIC32 SPI module 
	#define ENC_SPI_IF			(IFS0bits.SPI1IF)
	#define ENC_SSPBUF			(SPI1BUF)
	#define ENC_SPISTAT			(SPI1STAT)
	#define ENC_SPISTATbits		(SPI1STATbits)
	#define ENC_SPICON1			(SPI1CON1)
	#define ENC_SPICON1bits		(SPI1CON1bits)
	#define ENC_SPICON2			(SPI1CON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISAbits.TRISA8)
	#define EEPROM_CS_IO		(PORTAbits.RA8)
	#define EEPROM_SCK_TRIS		(TRISCbits.TRISC8)
	#define EEPROM_SDI_TRIS		(TRISCbits.TRISC4)
	#define EEPROM_SDO_TRIS		(TRISCbits.TRISC5)
	#define EEPROM_SPI_IF		(IFS2bits.SPI2IF)
	#define EEPROM_SSPBUF		(SPI2BUF)
	#define EEPROM_SPICON1		(SPI2CON1)
	#define EEPROM_SPICON1bits	(SPI2CON1bits)
	#define EEPROM_SPICON2		(SPI2CON2)
	#define EEPROM_SPISTAT		(SPI2STAT)
	#define EEPROM_SPISTATbits	(SPI2STATbits)

	// LCD Module I/O pins
	#define LCD_DATA0_TRIS		(TRISBbits.TRISB12)		// Multiplexed with LED6
	#define LCD_DATA0_IO		(LATBbits.LATB12)
	#define LCD_DATA1_TRIS		(TRISBbits.TRISB11)
	#define LCD_DATA1_IO		(LATBbits.LATB11)
	#define LCD_DATA2_TRIS		(TRISBbits.TRISB10)
	#define LCD_DATA2_IO		(LATBbits.LATB10)
	#define LCD_DATA3_TRIS		(TRISBbits.TRISB9)		// Multiplexed with LED3
	#define LCD_DATA3_IO		(LATBbits.LATB9)
	#define LCD_DATA4_TRIS		(TRISBbits.TRISB8)		// Multiplexed with LED2
	#define LCD_DATA4_IO		(LATBbits.LATB8)
	#define LCD_DATA5_TRIS		(TRISBbits.TRISB7)
	#define LCD_DATA5_IO		(LATBbits.LATB7)
	#define LCD_DATA6_TRIS		(TRISBbits.TRISB6)
	#define LCD_DATA6_IO		(LATBbits.LATB6)
	#define LCD_DATA7_TRIS		(TRISBbits.TRISB5)
	#define LCD_DATA7_IO		(LATBbits.LATB5)
	#define LCD_RD_WR_TRIS		(TRISBbits.TRISB13)
	#define LCD_RD_WR_IO		(LATBbits.LATB13)
	#define LCD_RS_TRIS			(TRISCbits.TRISC7)
	#define LCD_RS_IO			(LATCbits.LATC7)
	#define LCD_E_TRIS			(TRISBbits.TRISB14)
	#define LCD_E_IO			(LATBbits.LATB14)

	// Peripheral Pin Select Outputs
	#define NULL_IO		0
	#define C1OUT_IO	1
	#define C2OUT_IO	2
	#define U1TX_IO		3
	#define U1RTS_IO	4
	#define U2TX_IO		5
	#define U2RTS_IO	6
	#define SDO1_IO		7
	#define SCK1OUT_IO	8
	#define SS1OUT_IO	9
	#define SDO2_IO		10
	#define SCK2OUT_IO	11
	#define SS2OUT_IO	12
	#define OC1_IO		18
	#define OC2_IO		19
	#define OC3_IO		20
	#define OC4_IO		21
	#define OC5_IO		22

#elif defined(EXPLORER_16)
// Explorer 16 + PIC24FJ128GA010/PIC24HJ256GP610/dsPIC33FJ256GP710 PIM + Ethernet PICtail Plus
// Explorer 16 + PIC32MX460F512L/PIC32MX360F512L PIM + Ethernet PICtail Plus

	#define LED0_TRIS			(TRISAbits.TRISA0)	// Ref D3
	#define LED0_IO				(LATAbits.LATA0)	
	#define LED1_TRIS			(TRISAbits.TRISA1)	// Ref D4
	#define LED1_IO				(LATAbits.LATA1)
	#define LED2_TRIS			(TRISAbits.TRISA2)	// Ref D5
	#define LED2_IO				(LATAbits.LATA2)
	#define LED3_TRIS			(TRISAbits.TRISA3)	// Ref D6
	#define LED3_IO				(LATAbits.LATA3)
	#define LED4_TRIS			(TRISAbits.TRISA4)	// Ref D7
	#define LED4_IO				(LATAbits.LATA4)
	#define LED5_TRIS			(TRISAbits.TRISA5)	// Ref D8
	#define LED5_IO				(LATAbits.LATA5)
	#define LED6_TRIS			(TRISAbits.TRISA6)	// Ref D9
	#define LED6_IO				(LATAbits.LATA6)
	#define LED7_TRIS			(TRISAbits.TRISA7)	// Ref D10	// Note: This is multiplexed with BUTTON1
	#define LED7_IO				(LATAbits.LATA7)
	#define LED_GET()			(*((volatile unsigned char*)(&LATA)))
	#define LED_PUT(a)			(*((volatile unsigned char*)(&LATA)) = (a))


	#define BUTTON0_TRIS		(TRISDbits.TRISD13)	// Ref S4
	#define	BUTTON0_IO			(PORTDbits.RD13)
	#define BUTTON1_TRIS		(TRISAbits.TRISA7)	// Ref S5	// Note: This is multiplexed with LED7
	#define	BUTTON1_IO			(PORTAbits.RA7)
	#define BUTTON2_TRIS		(TRISDbits.TRISD7)	// Ref S6
	#define	BUTTON2_IO			(PORTDbits.RD7)
	#define BUTTON3_TRIS		(TRISDbits.TRISD6)	// Ref S3
	#define	BUTTON3_IO			(PORTDbits.RD6)

	#define UARTTX_TRIS			(TRISFbits.TRISF5)
	#define UARTTX_IO			(PORTFbits.RF5)
	#define UARTRX_TRIS			(TRISFbits.TRISF4)
	#define UARTRX_IO			(PORTFbits.RF4)

	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		(TRISDbits.TRISD15)	// Not connected by default
	#define ENC_RST_IO			(PORTDbits.RD15)
	#define ENC_CS_TRIS			(TRISDbits.TRISD14)
	#define ENC_CS_IO			(PORTDbits.RD14)
	// SPI SCK, SDI, SDO pins are automatically controlled by the 
	// PIC24/dsPIC/PIC32 SPI module 
	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
		#define ENC_SPI_IF			(IFS0bits.SPI1IF)
		#define ENC_SSPBUF			(SPI1BUF)
		#define ENC_SPISTAT			(SPI1STAT)
		#define ENC_SPISTATbits		(SPI1STATbits)
		#define ENC_SPICON1			(SPI1CON1)
		#define ENC_SPICON1bits		(SPI1CON1bits)
		#define ENC_SPICON2			(SPI1CON2)
	#else					// PIC32
		#define ENC_SPI_IF			(IFS0bits.SPI1RXIF)
		#define ENC_SSPBUF			(SPI1BUF)
		#define ENC_SPICON1			(SPI1CON)
		#define ENC_SPICON1bits		(SPI1CONbits)
		#define ENC_SPIBRG			(SPI1BRG)
	#endif

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISDbits.TRISD12)
	#define EEPROM_CS_IO		(PORTDbits.RD12)
	#define EEPROM_SCK_TRIS		(TRISGbits.TRISG6)
	#define EEPROM_SDI_TRIS		(TRISGbits.TRISG7)
	#define EEPROM_SDO_TRIS		(TRISGbits.TRISG8)
	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
		#define EEPROM_SPI_IF		(IFS2bits.SPI2IF)
		#define EEPROM_SSPBUF		(SPI2BUF)
		#define EEPROM_SPICON1		(SPI2CON1)
		#define EEPROM_SPICON1bits	(SPI2CON1bits)
		#define EEPROM_SPICON2		(SPI2CON2)
		#define EEPROM_SPISTAT		(SPI2STAT)
		#define EEPROM_SPISTATbits	(SPI2STATbits)
	#else					// PIC32
		#define EEPROM_SPI_IF		(IFS1bits.SPI2RXIF)
		#define EEPROM_SSPBUF		(SPI2BUF)
		#define EEPROM_SPICON1		(SPI2CON)
		#define EEPROM_SPICON1bits	(SPI2CONbits)
		#define EEPROM_SPIBRG		(SPI2BRG)
	#endif

	// LCD Module I/O pins
	#define LCD_DATA_TRIS		(*((volatile BYTE*)&TRISE))
	#define LCD_DATA_IO			(*((volatile BYTE*)&LATE))
	#define LCD_RD_WR_TRIS		(TRISDbits.TRISD5)
	#define LCD_RD_WR_IO		(LATDbits.LATD5)
	#define LCD_RS_TRIS			(TRISBbits.TRISB15)
	#define LCD_RS_IO			(LATBbits.LATB15)
	#define LCD_E_TRIS			(TRISDbits.TRISD4)
	#define LCD_E_IO			(LATDbits.LATD4)

//	// Serial Flash/SRAM/UART PICtail Plus attached to SPI2 (middle pin group)
//	#define SPIRAM_CS_TRIS			(TRISGbits.TRISG9)
//	#define SPIRAM_CS_IO			(LATGbits.LATG9)
//	#define SPIRAM_SCK_TRIS			(TRISGbits.TRISG6)
//	#define SPIRAM_SDI_TRIS			(TRISGbits.TRISG7)
//	#define SPIRAM_SDO_TRIS			(TRISGbits.TRISG8)
//	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
//		#define SPIRAM_SPI_IF			(IFS2bits.SPI2IF)
//		#define SPIRAM_SSPBUF			(SPI2BUF)
//		#define SPIRAM_SPICON1			(SPI2CON1)
//		#define SPIRAM_SPICON1bits		(SPI2CON1bits)
//		#define SPIRAM_SPICON2			(SPI2CON2)
//		#define SPIRAM_SPISTAT			(SPI2STAT)
//		#define SPIRAM_SPISTATbits		(SPI2STATbits)
//	#else					// PIC32
//		#define SPIRAM_SPI_IF			(IFS1bits.SPI2RXIF)
//		#define SPIRAM_SSPBUF			(SPI2BUF)
//		#define SPIRAM_SPICON1			(SPI2CON)
//		#define SPIRAM_SPICON1bits		(SPI2CONbits)
//		#define SPIRAM_SPIBRG			(SPI2BRG)
//	#endif
//	#define SPIFLASH_CS_TRIS		(TRISBbits.TRISB8)
//	#define SPIFLASH_CS_IO			(LATBbits.LATB8)
//	#define SPIFLASH_SCK_TRIS		(TRISGbits.TRISG6)
//	#define SPIFLASH_SDI_TRIS		(TRISGbits.TRISG7)
//	#define SPIFLASH_SDI_IO			(PORTGbits.RG7)
//	#define SPIFLASH_SDO_TRIS		(TRISGbits.TRISG8)
//	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
//		#define SPIFLASH_SPI_IF			(IFS2bits.SPI2IF)
//		#define SPIFLASH_SSPBUF			(SPI2BUF)
//		#define SPIFLASH_SPICON1		(SPI2CON1)
//		#define SPIFLASH_SPICON1bits	(SPI2CON1bits)
//		#define SPIFLASH_SPICON2		(SPI2CON2)
//		#define SPIFLASH_SPISTAT		(SPI2STAT)
//		#define SPIFLASH_SPISTATbits	(SPI2STATbits)
//	#else					// PIC32
//		#define SPIFLASH_SPI_IF			(IFS1bits.SPI2RXIF)
//		#define SPIFLASH_SSPBUF			(SPI2BUF)
//		#define SPIFLASH_SPICON1		(SPI2CON)
//		#define SPIFLASH_SPICON1bits	(SPI2CONbits)
//		#define SPIFLASH_SPIBRG			(SPI2BRG)
//	#endif

#elif defined(DSPICDEM11)
// dsPICDEM 1.1 Development Board + Ethernet PICtail airwired. There 
// is no PICtail header on this development board.  The following 
// airwires must be made:
// 1. dsPICDEM GND <-> PICtail GND (PICtail pin 27)
// 2. dsPICDEM Vdd <- PICtail VPIC (PICtail pin 25)
// 3. dsPICDEM RG2 -> PICtail ENC28J60 CS (PICtail pin 22)
// 4. dsPICDEM RF6 -> PICtail SCK (PICtail pin 11)
// 5. dsPICDEM RF7 <- PICtail SDI (PICtail pin 9)
// 6. dsPICDEM RF8 -> PICtail SDO (PICtail pin 7)
// 7. dsPICDEM RG3 -> PICtail 25LC256 CS (PICtail pin 20)

	#define LED0_TRIS			(TRISDbits.TRISD3)	// Ref LED4
	#define LED0_IO				(PORTDbits.RD3)	
	#define LED1_TRIS			(TRISDbits.TRISD2)	// Ref LED3
	#define LED1_IO				(PORTDbits.RD2)
	#define LED2_TRIS			(TRISDbits.TRISD1)	// Ref LED2
	#define LED2_IO				(PORTDbits.RD1)
	#define LED3_TRIS			(TRISDbits.TRISD0)	// Ref LED1
	#define LED3_IO				(PORTDbits.RD0)
	#define LED4_TRIS			(TRISDbits.TRISD3)	// No LED, Remapped to Ref LED4
	#define LED4_IO				(PORTDbits.RD3)	
	#define LED5_TRIS			(TRISDbits.TRISD2)	// No LED, Remapped to Ref LED3
	#define LED5_IO				(PORTDbits.RD2)
	#define LED6_TRIS			(TRISDbits.TRISD1)	// No LED, Remapped to Ref LED2
	#define LED6_IO				(PORTDbits.RD1)
	#define LED7_TRIS			(TRISDbits.TRISD0)	// No LED, Remapped to Ref LED1
	#define LED7_IO				(PORTDbits.RD0)
	#define LED_GET()			((LED3_IO<<3) | (LED2_IO<<2) | (LED1_IO<<1) | LED0_IO)
	#define LED_PUT(a)			do{BYTE vTemp = (a); LED0_IO = vTemp&0x1; LED1_IO = vTemp&0x2; LED2_IO = vTemp&0x4; LED3_IO = vTemp&0x8;} while(0)

	#define BUTTON0_TRIS		(TRISAbits.TRISA15)	// Ref SW4
	#define	BUTTON0_IO			(PORTAbits.RA15)
	#define BUTTON1_TRIS		(TRISAbits.TRISA14)	// Ref SW3
	#define	BUTTON1_IO			(PORTAbits.RA14)
	#define BUTTON2_TRIS		(TRISAbits.TRISA13)	// Ref SW2
	#define	BUTTON2_IO			(PORTAbits.RA13)
	#define BUTTON3_TRIS		(TRISAbits.TRISA12)	// Ref SW1
	#define	BUTTON3_IO			(PORTAbits.RA12)

	#define UARTTX_TRIS			(TRISFbits.TRISF3)
	#define UARTTX_IO			(PORTFbits.RF3)
	#define UARTRX_TRIS			(TRISFbits.TRISF2)
	#define UARTRX_IO			(PORTFbits.RF2)

	// ENC28J60 I/O pins
	#define ENC_CS_TRIS			(TRISGbits.TRISG2)		// User must airwire this
	#define ENC_CS_IO			(PORTGbits.RG2)
	// SPI SCK, SDI, SDO pins are automatically controlled by the 
	// PIC24/dsPIC/PIC32 SPI module 
	#define ENC_SPI_IF			(IFS0bits.SPI1IF)
	#define ENC_SSPBUF			(SPI1BUF)
	#define ENC_SPICON1			(SPI1CON)
	#define ENC_SPICON1bits		(SPI1CONbits)
	#define ENC_SPICON2			(SPI1BUF)				// SPI1CON2 doesn't exist, remap to unimportant register
	#define ENC_SPISTAT			(SPI1STAT)
	#define ENC_SPISTATbits		(SPI1STATbits)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISGbits.TRISG3)	// User must airwire this
	#define EEPROM_CS_IO		(PORTGbits.RG3)
	#define EEPROM_SCK_TRIS		(TRISGbits.TRISG6)
	#define EEPROM_SDI_TRIS		(TRISGbits.TRISG7)
	#define EEPROM_SDO_TRIS		(TRISGbits.TRISG8)
	#define EEPROM_SPI_IF		(IFS0bits.SPI1IF)
	#define EEPROM_SSPBUF		(SPI1BUF)
	#define EEPROM_SPICON1		(SPI1CON)
	#define EEPROM_SPICON1bits	(SPI1CONbits)
	#define EEPROM_SPICON2		(SPI1BUF)			// SPI1CON2 doesn't exist, remap to unimportant register
	#define EEPROM_SPISTAT		(SPI1STAT)
	#define EEPROM_SPISTATbits	(SPI1STATbits)

	// SI3000 codec pins
	#define CODEC_RST_TRIS		(TRISFbits.TRISF6)
	#define CODEC_RST_IO		(PORTFbits.RF6)
	
	// PIC18F252 LCD Controller
	#define LCDCTRL_CS_TRIS		(TRISGbits.TRISG9)
	#define LCDCTRL_CS_IO		(PORTGbits.RG9)

#elif defined(PICDEMNET2) && !defined(HI_TECH_C)
// PICDEM.net 2 (PIC18F97J60 + ENC28J60)
//  * lzf * MCC18 编译器部分


    extern unsigned char led_reg;

	// I/O pins
	#define LED0_TRIS			led_reg   //(TRISJbits.TRISJ0)
	#define LED0_IO				led_reg   //(LATJbits.LATJ0)
	#define LED1_TRIS			led_reg   //(TRISJbits.TRISJ1)
	#define LED1_IO				led_reg   //(LATJbits.LATJ1)
	#define LED2_TRIS			led_reg   //(TRISJbits.TRISJ2)
	#define LED2_IO				led_reg   //(LATJbits.LATJ2)
	#define LED3_TRIS			led_reg   //(TRISJbits.TRISJ3)
	#define LED3_IO				led_reg   //(LATJbits.LATJ3)
	#define LED4_TRIS			led_reg   //(TRISJbits.TRISJ4)
	#define LED4_IO				led_reg   //(LATJbits.LATJ4)
	#define LED5_TRIS			led_reg   //(TRISJbits.TRISJ5)
	#define LED5_IO				led_reg   //(LATJbits.LATJ5)
	#define LED6_TRIS			led_reg   //(TRISJbits.TRISJ6)
	#define LED6_IO				led_reg   //(LATJbits.LATJ6)
	#define LED7_TRIS			led_reg   //(TRISJbits.TRISJ7)
	#define LED7_IO				led_reg   //(LATJbits.LATJ7)
	#define LED_GET()			led_reg   //(LATJ)
	#define LED_PUT(a)			(led_reg = (a))  //(LATJ = (a))

	#define BUTTON0_TRIS		led_reg   //(TRISFbits.TRISF5)//	跟Microchip原版有所改变
	#define	BUTTON0_IO			led_reg   //(PORTFbits.RF5)
	#define BUTTON1_TRIS		led_reg   //(TRISFbits.TRISF4)
	#define	BUTTON1_IO			led_reg   //(PORTFbits.RF4)
	#define BUTTON2_TRIS		led_reg   //(TRISFbits.TRISF3)
	#define	BUTTON2_IO			led_reg   //(PORTFbits.RF3)
	#define BUTTON3_TRIS		led_reg   //(TRISFbits.TRISF2)
	#define	BUTTON3_IO			led_reg   //(PORTFbits.RF2)


	//然后添加这些自己的变量
	/* lzf * add */
    #define RUN_LED_TRIS        (TRISGbits.TRISG3)
    #define RUN_LED_IO          (LATGbits.LATG3)
    #define IP_CONFIG_TRIS      (TRISDbits.TRISD6)
    #define IP_CONFIG_IO        (PORTDbits.RD6)

	//继电器部分
    #define  RELAY_OUT_TRIS_0       (TRISHbits.TRISH2)
    #define  RELAY_OUT_0            (LATHbits.LATH2)
    #define  RELAY_OUT_TRIS_1       (TRISHbits.TRISH3)
    #define  RELAY_OUT_1            (LATHbits.LATH3)
    #define  RELAY_OUT_TRIS_2       (TRISEbits.TRISE1)
    #define  RELAY_OUT_2            (LATEbits.LATE1)
    #define  RELAY_OUT_TRIS_3       (TRISEbits.TRISE0)
    #define  RELAY_OUT_3            (LATEbits.LATE0)
    #define  RELAY_OUT_TRIS_4       (TRISBbits.TRISB0)
    #define  RELAY_OUT_4            (LATBbits.LATB0)
    #define  RELAY_OUT_TRIS_5       (TRISBbits.TRISB1)
    #define  RELAY_OUT_5            (LATBbits.LATB1)
    #define  RELAY_OUT_TRIS_6       (TRISBbits.TRISB2)
    #define  RELAY_OUT_6            (LATBbits.LATB2)
	//输入部分
    #define  DIG_INPUT_TRIS_0       (TRISDbits.TRISD6)
    #define  DIG_INPUT_IO_0         (PORTDbits.RD6)



	#define LCD_BL_TRIS			led_reg   //(TRISHbits.TRISH3)	//LCD背光灯控制		跟Microchip原版有所改变
	#define LCD_BL_IO			led_reg   //(PORTHbits.RH3)
	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		(TRISDbits.TRISD3)	// Not connected by default
	#define ENC_RST_IO			(LATDbits.LATD3)
//	#define ENC_CS_TRIS			(TRISDbits.TRISD1)	// Uncomment this line if you wish to use the ENC28J60 on the PICDEM.net 2 board instead of the internal PIC18F97J60 Ethernet module
	#define ENC_CS_IO			(LATDbits.LATD1)
	#define ENC_SCK_TRIS		(TRISDbits.TRISD6)
	#define ENC_SDI_TRIS		(TRISDbits.TRISD5)
	#define ENC_SDO_TRIS		(TRISDbits.TRISD4)
	#define ENC_SPI_IF			(PIR3bits.SSP2IF)
	#define ENC_SSPBUF			(SSP2BUF)
	#define ENC_SPISTAT			(SSP2STAT)
	#define ENC_SPISTATbits		(SSP2STATbits)
	#define ENC_SPICON1			(SSP2CON1)
	#define ENC_SPICON1bits		(SSP2CON1bits)
	#define ENC_SPICON2			(SSP2CON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISCbits.TRISC0)	//跟Microchip原版有所改变
	#define EEPROM_CS_IO		(LATCbits.LATC0)
	#define EEPROM_SCK_TRIS		(TRISCbits.TRISC3)
	#define EEPROM_SDI_TRIS		(TRISCbits.TRISC4)
	#define EEPROM_SDO_TRIS		(TRISCbits.TRISC5)
	#define EEPROM_SPI_IF		(PIR1bits.SSPIF)
	#define EEPROM_SSPBUF		(SSPBUF)
	#define EEPROM_SPICON1		(SSP1CON1)
	#define EEPROM_SPICON1bits	(SSP1CON1bits)
	#define EEPROM_SPICON2		(SSP1CON2)
	#define EEPROM_SPISTAT		(SSP1STAT)
	#define EEPROM_SPISTATbits	(SSP1STATbits)

	// LCD I/O pins
	#define LCD_DATA_TRIS		led_reg   //(TRISE)
	#define LCD_DATA_IO			led_reg   //(LATE)
	#define LCD_RD_WR_TRIS		led_reg   //(TRISHbits.TRISH1)
	#define LCD_RD_WR_IO		led_reg   //(LATHbits.LATH1)
	#define LCD_RS_TRIS			led_reg   //(TRISHbits.TRISH2)
	#define LCD_RS_IO			led_reg   //led_reg   //(LATHbits.LATH2)
	#define LCD_E_TRIS			led_reg   //(TRISHbits.TRISH0)
	#define LCD_E_IO			led_reg   //(LATHbits.LATH0)

	// Serial Flash/SRAM/UART PICtail
//	#define SPIRAM_CS_TRIS			(TRISBbits.TRISB5)
//	#define SPIRAM_CS_IO			(LATBbits.LATB5)
//	#define SPIRAM_SCK_TRIS			(TRISCbits.TRISC3)
//	#define SPIRAM_SDI_TRIS			(TRISCbits.TRISC4)
//	#define SPIRAM_SDO_TRIS			(TRISCbits.TRISC5)
//	#define SPIRAM_SPI_IF			(PIR1bits.SSPIF)
//	#define SPIRAM_SSPBUF			(SSP1BUF)
//	#define SPIRAM_SPICON1			(SSP1CON1)
//	#define SPIRAM_SPICON1bits		(SSP1CON1bits)
//	#define SPIRAM_SPICON2			(SSP1CON2)
//	#define SPIRAM_SPISTAT			(SSP1STAT)
//	#define SPIRAM_SPISTATbits		(SSP1STATbits)
//	#define SPIFLASH_CS_TRIS		(TRISBbits.TRISB4)
//	#define SPIFLASH_CS_IO			(LATBbits.LATB4)
//	#define SPIFLASH_SCK_TRIS		(TRISCbits.TRISC3)
//	#define SPIFLASH_SDI_TRIS		(TRISCbits.TRISC4)
//	#define SPIFLASH_SDI_IO			(PORTCbits.RC4)
//	#define SPIFLASH_SDO_TRIS		(TRISCbits.TRISC5)
//	#define SPIFLASH_SPI_IF			(PIR1bits.SSPIF)
//	#define SPIFLASH_SSPBUF			(SSP1BUF)
//	#define SPIFLASH_SPICON1		(SSP1CON1)
//	#define SPIFLASH_SPICON1bits	(SSP1CON1bits)
//	#define SPIFLASH_SPICON2		(SSP1CON2)
//	#define SPIFLASH_SPISTAT		(SSP1STAT)
//	#define SPIFLASH_SPISTATbits	(SSP1STATbits)

#elif defined(PICDEMNET2) && defined(HI_TECH_C)	//下面是该硬件版本（Ethernet-V2.0）的硬件底层定义
// PICDEM.net 2 (PIC18F97J60 + ENC28J60) + HI-TECH PICC-18 compiler
// * lzf * mask
	typedef struct
	{
		unsigned char BOR:1;
		unsigned char POR:1;
		unsigned char PD:1;
		unsigned char TO:1;
		unsigned char RI:1;
		unsigned char CM:1;
		unsigned char :1;
		unsigned char IPEN:1;
	} RCONbits;
	typedef struct
	{
	    unsigned char BF:1;
	    unsigned char UA:1;
	    unsigned char R_W:1;
	    unsigned char S:1;
	    unsigned char P:1;
	    unsigned char D_A:1;
	    unsigned char CKE:1;
	    unsigned char SMP:1;
	} SSPSTATbits;
	typedef struct
	{
	    unsigned char SSPM0:1;
	    unsigned char SSPM1:1;
	    unsigned char SSPM2:1;
	    unsigned char SSPM3:1;
	    unsigned char CKP:1;
	    unsigned char SSPEN:1;
	    unsigned char SSPOV:1;
	    unsigned char WCOL:1;
	} SSPCON1bits;
	typedef struct
	{
	    unsigned char SEN:1;
	    unsigned char RSEN:1;
	    unsigned char PEN:1;
	    unsigned char RCEN:1;
	    unsigned char ACKEN:1;
	    unsigned char ACKDT:1;
	    unsigned char ACKSTAT:1;
	    unsigned char GCEN:1;
	} SSPCON2bits;
	typedef struct 
	{
	    unsigned char RBIF:1;
	    unsigned char INT0IF:1;
	    unsigned char TMR0IF:1;
		unsigned char RBIE:1;
	    unsigned char INT0IE:1;
	    unsigned char TMR0IE:1;
	    unsigned char GIEL:1;
	    unsigned char GIEH:1;
	} INTCONbits;
	typedef struct 
	{
	    unsigned char RBIP:1;
	    unsigned char INT3IP:1;
	    unsigned char TMR0IP:1;
	    unsigned char INTEDG3:1;
	    unsigned char INTEDG2:1;
	    unsigned char INTEDG1:1;
	    unsigned char INTEDG0:1;
	    unsigned char RBPU:1;
	} INTCON2bits;
	typedef struct 
	{
	    unsigned char ADON:1;
	    unsigned char GO:1;
	    unsigned char CHS0:1;
	    unsigned char CHS1:1;
	    unsigned char CHS2:1;
	    unsigned char CHS3:1;
		unsigned char :1;
		unsigned char ADCAL:1;
	} ADCON0bits;
	typedef struct 
	{
		unsigned char ADCS0:1;
		unsigned char ADCS1:1;
		unsigned char ADCS2:1;
		unsigned char ACQT0:1;
		unsigned char ACQT1:1;
		unsigned char ACQT2:1;
		unsigned char :1;
		unsigned char ADFM:1;
	} ADCON2bits;
	typedef struct 
	{
	    unsigned char TMR1IF:1;
	    unsigned char TMR2IF:1;
	    unsigned char CCP1IF:1;
	    unsigned char SSPIF:1;
	    unsigned char TXIF:1;
	    unsigned char RCIF:1;
	    unsigned char ADIF:1;
	    unsigned char PSPIF:1;
	} PIR1bits;
	typedef struct 
	{
	    unsigned char CCP2IF:1;
	    unsigned char TMR3IF:1;
	    unsigned char HLVDIF:1;
	    unsigned char BCLIF:1;
	    unsigned char EEIF:1;
	    unsigned char :1;
	    unsigned char CMIF:1;
	    unsigned char OSCFIF:1;
	} PIR2bits;
	typedef struct 
	{
	    unsigned char TMR1IE:1;
	    unsigned char TMR2IE:1;
	    unsigned char CCP1IE:1;
	    unsigned char SSPIE:1;
	    unsigned char TXIE:1;
	    unsigned char RCIE:1;
	    unsigned char ADIE:1;
	    unsigned char PSPIE:1;
	} PIE1bits;
	typedef struct
	{
		unsigned char TMR1IP:1;
		unsigned char TMR2IP:1;
		unsigned char CCP1IP:1;
		unsigned char SSP1IP:1;
		unsigned char TXIP:1;
		unsigned char RCIP:1;
		unsigned char ADIP:1;
		unsigned char PSPIP:1;
	} IPR1bits;
	typedef struct
	{
	    unsigned char T0PS0:1;
	    unsigned char T0PS1:1;
	    unsigned char T0PS2:1;
	    unsigned char PSA:1;
	    unsigned char T0SE:1;
	    unsigned char T0CS:1;
	    unsigned char T08BIT:1;
	    unsigned char TMR0ON:1;
	} T0CONbits;
	typedef struct
	{
	    unsigned char TX9D:1;
	    unsigned char TRMT:1;
	    unsigned char BRGH:1;
	    unsigned char SENDB:1;
	    unsigned char SYNC:1;
	    unsigned char TXEN:1;
	    unsigned char TX9:1;
	    unsigned char CSRC:1;
	} TXSTAbits;
	typedef struct
	{
	    unsigned char RX9D:1;
	    unsigned char OERR:1;
	    unsigned char FERR:1;
	    unsigned char ADDEN:1;
	    unsigned char CREN:1;
	    unsigned char SREN:1;
	    unsigned char RX9:1;
	    unsigned char SPEN:1;
	} RCSTAbits;
	typedef struct
	{
		unsigned char LATA0:1;
		unsigned char LATA1:1;
		unsigned char LATA2:1;
		unsigned char LATA3:1;
		unsigned char LATA4:1;
		unsigned char LATA5:1;
		unsigned char REPU:1;
		unsigned char RDPU:1;
	} LATAbits;
	typedef struct
	{
		unsigned char TRISA0:1;
		unsigned char TRISA1:1;
		unsigned char TRISA2:1;
		unsigned char TRISA3:1;
		unsigned char TRISA4:1;
		unsigned char TRISA5:1;
		unsigned char :2;
	} TRISAbits;
	typedef struct
	{
	  unsigned :5;
	  unsigned ETHEN:1;
	  unsigned PKTDEC:1;
	  unsigned AUTOINC:1;
	} ECON2bits;
	typedef struct
	{
	  unsigned PHYRDY:1;
	  unsigned TXABRT:1;
	  unsigned RXBUSY:1;
	  unsigned :3;
	  unsigned BUFER:1;
	} ESTATbits;
	typedef struct
	{
	  unsigned :2;
	  unsigned RXEN:1;
	  unsigned TXRTS:1;
	  unsigned CSUMEN:1;
	  unsigned DMAST:1;
	  unsigned RXRST:1;
	  unsigned TXRST:1;
	} ECON1bits;
	typedef struct
	{
	  unsigned RXERIF:1;
	  unsigned TXERIF:1;
	  unsigned :1;
	  unsigned TXIF:1;
	  unsigned LINKIF:1;
	  unsigned DMAIF:1;
	  unsigned PKTIF:1;
	} EIRbits;
	typedef struct
	{
	  unsigned BUSY:1;
	  unsigned SCAN:1;
	  unsigned NVALID:1;
	} MISTATbits;
	typedef struct {
	  unsigned ABDEN:1;
	  unsigned WUE:1;
	  unsigned :1;
	  unsigned BRG16:1;
	  unsigned TXCKP:1;
	  unsigned RXDTP:1;
	  unsigned RCIDL:1;
	  unsigned ABDOVF:1;
	} BAUDCONbits;

	#define SPBRGH				SPBRGH1
	
	#define RCONbits			(*((RCONbits*)&RCON))
	#define SSP1STATbits		(*((SSPSTATbits*)&SSP1STAT))
	#define INTCONbits			(*((INTCONbits*)&INTCON))
	#define INTCON2bits			(*((INTCON2bits*)&INTCON2))
	#define ADCON0bits			(*((ADCON0bits*)&ADCON0))
	#define ADCON2bits			(*((ADCON2bits*)&ADCON2))
	#define PIR1bits			(*((PIR1bits*)&PIR1))
	#define PIR2bits			(*((PIR2bits*)&PIR2))
	#define PIE1bits			(*((PIE1bits*)&PIE1))
	#define IPR1bits			(*((IPR1bits*)&IPR1))
	#define T0CONbits			(*((T0CONbits*)&T0CON))
	#define TXSTAbits			(*((TXSTAbits*)&TXSTA))
	#define RCSTAbits			(*((RCSTAbits*)&RCSTA))
	#define SSPCON1bits			(*((SSPCON1bits*)&SSPCON1))
	#define SSPCON2bits			(*((SSPCON2bits*)&SSPCON2))
	#define LATAbits			(*((LATAbits*)&LATA))
	#define TRISAbits			(*((TRISAbits*)&TRISA))
	#define ECON1bits			(*((ECON1bits*)&ECON1))
	#define ESTATbits			(*((ESTATbits*)&ESTAT))
	#define ECON2bits			(*((ECON2bits*)&ECON2))
	#define EIRbits				(*((EIRbits*)&EIR))
	#define MISTATbits			(*((MISTATbits*)&MISTAT))
	#define BAUDCONbits			(*((BAUDCONbits*)&BAUDCON1))

#if 1
	// * lzf * changed ...
	extern unsigned char led_reg;
	// I/O pins
	#define LED0_TRIS			led_reg   //(TRISJ0)
	#define LED0_IO				led_reg   //(LATJ0)
	#define LED1_TRIS			led_reg   //(TRISJ1)
	#define LED1_IO				led_reg   //(LATJ1)
	#define LED2_TRIS			led_reg   //(TRISJ2)
	#define LED2_IO				led_reg   //(LATJ2)
	#define LED3_TRIS			led_reg  //(TRISJ3)
	#define LED3_IO				led_reg  //(LATJ3)
	#define LED4_TRIS			led_reg  //(TRISJ4)
	#define LED4_IO				led_reg  //(LATJ4)
	#define LED5_TRIS			led_reg  //(TRISJ5)
	#define LED5_IO				led_reg  //(LATJ5)
	#define LED6_TRIS			led_reg  //(TRISJ6)
	#define LED6_IO				led_reg  //(LATJ6)
	#define LED7_TRIS			led_reg  //(TRISJ7)
	#define LED7_IO				led_reg  //(LATJ7)
	#define LED_GET()			led_reg  //(LATJ)
	#define LED_PUT(a)			(led_reg = (a))   //(LATJ = (a))

	#define BUTTON0_TRIS		led_reg   //(TRISF5)	//跟Microchip原版有所改变
	#define	BUTTON0_IO			led_reg   //(RF5)
	#define BUTTON1_TRIS		led_reg   //(TRISF4)
	#define	BUTTON1_IO			led_reg   //(RF4)
	#define BUTTON2_TRIS		led_reg   //(TRISF3)
	#define	BUTTON2_IO			led_reg   //(RF3)
	#define BUTTON3_TRIS		led_reg   //(TRISF2)
	#define	BUTTON3_IO			led_reg   //(RF2)
	//然后添加这些自己的变量
	/* lzf * add */
    #define RUN_LED_TRIS        (TRISG3)
    #define RUN_LED_IO          (LATG3)
    #define IP_CONFIG_TRIS      (TRISD6)
    #define IP_CONFIG_IO        (RD6)

	//继电器部分
    #define  RELAY_OUT_TRIS_0       (TRISH2)
    #define  RELAY_OUT_0            (LATH2)
    #define  RELAY_OUT_TRIS_1       (TRISH3)
    #define  RELAY_OUT_1            (LATH3)
    #define  RELAY_OUT_TRIS_2       (TRISE1)
    #define  RELAY_OUT_2            (LATE1)
    #define  RELAY_OUT_TRIS_3       (TRISE0)
    #define  RELAY_OUT_3            (LATE0)
    #define  RELAY_OUT_TRIS_4       (TRISB0)
    #define  RELAY_OUT_4            (LATB0)
    #define  RELAY_OUT_TRIS_5       (TRISB1)
    #define  RELAY_OUT_5            (LATB1)
    #define  RELAY_OUT_TRIS_6       (TRISB2)
    #define  RELAY_OUT_6            (LATB2)
	//输入部分
    #define  DIG_INPUT_TRIS_0       (TRISE5)
    #define  DIG_INPUT_IO_0         (RE5)
	#define  DIG_INPUT_TRIS_1       (TRISE6)
	#define  DIG_INPUT_IO_1         (RE6)
	#define  DIG_INPUT_TRIS_2       (TRISE7)
	#define  DIG_INPUT_IO_2         (RE7)
	#define  DIG_INPUT_TRIS_3       (TRISD0)
	#define  DIG_INPUT_IO_3         (RD0)
	#define  DIG_INPUT_TRIS_4       (TRISD1)
	#define  DIG_INPUT_IO_4         (RD1)
	#define  DIG_INPUT_TRIS_5       (TRISD2)
	#define  DIG_INPUT_IO_5         (RD2)
	#define  DIG_INPUT_TRIS_6       (TRISD3)
	#define  DIG_INPUT_IO_6         (RD3)
	#define  DIG_INPUT_TRIS_7       (TRISD4)
	#define  DIG_INPUT_IO_7         (RD4)
	#define  DIG_INPUT_TRIS_8       (TRISD5)
	#define  DIG_INPUT_IO_8         (RD5)

	// ENC28J60 I/O pins
	#define ENC_RST_TRIS		led_reg //(TRISD3)	// Not connected by default
	#define ENC_RST_IO			led_reg //(LATD3)
//	#define ENC_CS_TRIS			(TRISD1)	// Uncomment this line if you wish to use the ENC28J60 on the PICDEM.net 2 board instead of the internal PIC18F97J60 Ethernet module
	#define ENC_CS_IO			led_reg  //(LATD1)
	#define ENC_SCK_TRIS		led_reg  //(TRISD6)
	#define ENC_SDI_TRIS		led_reg  //(TRISD5)
	#define ENC_SDO_TRIS		led_reg  //(TRISC4)
	#define ENC_SPI_IF			led_reg  //(SSP2IF)
	#define ENC_SSPBUF			led_reg  //(SSP2BUF)
	#define ENC_SPISTAT			led_reg  //(SSP2STAT)
	#define ENC_SPISTATbits		led_reg  //(SSP2STATbits)
	#define ENC_SPICON1			led_reg  //(SSP2CON1)
	#define ENC_SPICON1bits		led_reg  //(SSP2CON1bits)
	#define ENC_SPICON2			led_reg  //(SSP2CON2)
#endif

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISC0)	//跟Microchip原版有所改变
	#define EEPROM_CS_IO		(LATC0)
	#define EEPROM_SCK_TRIS		(TRISC3)
	#define EEPROM_SDI_TRIS		(TRISC4)
	#define EEPROM_SDO_TRIS		(TRISC5)
	#define EEPROM_SPI_IF		(SSP1IF)
	#define EEPROM_SSPBUF		(SSP1BUF)
	#define EEPROM_SPICON1		(SSP1CON1)
	#define EEPROM_SPICON1bits	(SSP1CON1bits)
	#define EEPROM_SPICON2		(SSP1CON2)
	#define EEPROM_SPISTAT		(SSP1STAT)
	#define EEPROM_SPISTATbits	(SSP1STATbits)
#if 1 //lzf changed
	// LCD I/O pins
	#define LCD_BL_TRIS			led_reg  //(TRISH3)	//LCD背光灯控制		跟Microchip原版有所改变
	#define LCD_BL_IO			led_reg  //(LATH3)
	#define LCD_DATA_TRIS		led_reg  //(TRISE)
	#define LCD_DATA_IO			led_reg  //(LATE)
	#define LCD_RD_WR_TRIS		led_reg  //(TRISH1)
	#define LCD_RD_WR_IO		led_reg  //(LATH1)
	#define LCD_RS_TRIS			led_reg  //(TRISH2)
	#define LCD_RS_IO			led_reg  //(LATH2)
	#define LCD_E_TRIS			led_reg  //(TRISH0)
	#define LCD_E_IO			led_reg  //(LATH0)
#endif
	// Serial Flash/SRAM/UART PICtail
//	#define SPIRAM_CS_TRIS			(TRISB5)
//	#define SPIRAM_CS_IO			(LATB5)
//	#define SPIRAM_SCK_TRIS			(TRISC3)
//	#define SPIRAM_SDI_TRIS			(TRISC4)
//	#define SPIRAM_SDO_TRIS			(TRISC5)
//	#define SPIRAM_SPI_IF			(SSPIF)
//	#define SPIRAM_SSPBUF			(SSP1BUF)
//	#define SPIRAM_SPICON1			(SSP1CON1)
//	#define SPIRAM_SPICON1bits		(SSP1CON1bits)
//	#define SPIRAM_SPICON2			(SSP1CON2)
//	#define SPIRAM_SPISTAT			(SSP1STAT)
//	#define SPIRAM_SPISTATbits		(SSP1STATbits)
//	#define SPIFLASH_CS_TRIS		(TRISB4)
//	#define SPIFLASH_CS_IO			(LATB4)
//	#define SPIFLASH_SCK_TRIS		(TRISC3)
//	#define SPIFLASH_SDI_TRIS		(TRISC4)
//	#define SPIFLASH_SDI_IO			(RC4)
//	#define SPIFLASH_SDO_TRIS		(TRISC5)
//	#define SPIFLASH_SPI_IF			(SSPIF)
//	#define SPIFLASH_SSPBUF			(SSP1BUF)
//	#define SPIFLASH_SPICON1		(SSP1CON1)
//	#define SPIFLASH_SPICON1bits	(SSP1CON1bits)
//	#define SPIFLASH_SPICON2		(SSP1CON2)
//	#define SPIFLASH_SPISTAT		(SSP1STAT)
//	#define SPIFLASH_SPISTATbits	(SSP1STATbits)

#elif defined(INTERNET_RADIO) && !defined(HI_TECH_C)	// For C18 compiler on Internet Radio board
// Internet Radio board (05-60142)
	// I/O pins
	#define LED0_TRIS			(TRISCbits.TRISC2)
	#define LED0_IO				(LATCbits.LATC2)
	#define LED1_TRIS			(PRODL)				// No LED1 on this board
	#define LED1_IO				(PRODL)
	#define LED2_TRIS			(PRODL)				// No LED2 on this board
	#define LED2_IO				(PRODL)
	#define LED3_TRIS			(PRODL)				// No LED3 on this board
	#define LED3_IO				(PRODL)
	#define LED4_TRIS			(PRODL)				// No LED4 on this board
	#define LED4_IO				(PRODL)
	#define LED5_TRIS			(PRODL)				// No LED5 on this board
	#define LED5_IO				(PRODL)
	#define LED6_TRIS			(PRODL)				// No LED6 on this board
	#define LED6_IO				(PRODL)
	#define LED7_TRIS			(PRODL)				// No LED7 on this board
	#define LED7_IO				(PRODL)
	#define LED_GET()			(LED0_IO)
	#define LED_PUT(a)			(LED0_IO = (a))

	#define BUTTON0_TRIS		(TRISBbits.TRISB5)
	#define	BUTTON0_IO			(PORTBbits.RB5)
	#define BUTTON1_TRIS		(TRISFbits.TRISF1)
	#define	BUTTON1_IO			(PORTFbits.RF1)
	#define BUTTON2_TRIS		(TRISBbits.TRISB4)
	#define	BUTTON2_IO			(PORTBbits.RB4)
	#define BUTTON3_TRIS		(PRODL)				// No BUTTON3 on this board
	#define	BUTTON3_IO			(PRODL)

	// Serial SRAM
	#define SPIRAM_CS_TRIS			(TRISEbits.TRISE4)
	#define SPIRAM_CS_IO			(LATEbits.LATE4)
	#define SPIRAM_SCK_TRIS			(TRISCbits.TRISC3)
	#define SPIRAM_SDI_TRIS			(TRISCbits.TRISC4)
	#define SPIRAM_SDO_TRIS			(TRISCbits.TRISC5)
	#define SPIRAM_SPI_IF			(PIR1bits.SSPIF)
	#define SPIRAM_SSPBUF			(SSPBUF)
	#define SPIRAM_SPICON1			(SSP1CON1)
	#define SPIRAM_SPICON1bits		(SSP1CON1bits)
	#define SPIRAM_SPICON2			(SSP1CON2)
	#define SPIRAM_SPISTAT			(SSP1STAT)
	#define SPIRAM_SPISTATbits		(SSP1STATbits)
	#define SPIRAM2_CS_TRIS			(TRISEbits.TRISE5)
	#define SPIRAM2_CS_IO			(LATEbits.LATE5)
	#define SPIRAM2_SCK_TRIS		(TRISCbits.TRISC3)
	#define SPIRAM2_SDI_TRIS		(TRISCbits.TRISC4)
	#define SPIRAM2_SDO_TRIS		(TRISCbits.TRISC5)
	#define SPIRAM2_SPI_IF			(PIR1bits.SSPIF)
	#define SPIRAM2_SSPBUF			(SSPBUF)
	#define SPIRAM2_SPICON1			(SSP1CON1)
	#define SPIRAM2_SPICON1bits		(SSP1CON1bits)
	#define SPIRAM2_SPICON2			(SSP1CON2)
	#define SPIRAM2_SPISTAT			(SSP1STAT)
	#define SPIRAM2_SPISTATbits		(SSP1STATbits)
	
	// VLSI VS1011 MP3 decoder
	#define MP3_DREQ_TRIS			(TRISBbits.TRISB0)	// Data Request
	#define MP3_DREQ_IO 			(PORTBbits.RB0)		
	#define MP3_XRESET_TRIS			(TRISDbits.TRISD0)	// Reset, active low
	#define MP3_XRESET_IO			(LATDbits.LATD0)
	#define MP3_XDCS_TRIS			(TRISBbits.TRISB1)	// Data Chip Select
	#define MP3_XDCS_IO				(LATBbits.LATB1)
	#define MP3_XCS_TRIS			(TRISBbits.TRISB2)	// Control Chip Select
	#define MP3_XCS_IO				(LATBbits.LATB2)
	#define MP3_SCK_TRIS			(TRISCbits.TRISC3)
	#define MP3_SDI_TRIS			(TRISCbits.TRISC4)
	#define MP3_SDO_TRIS			(TRISCbits.TRISC5)
	#define MP3_SPI_IF				(PIR1bits.SSPIF)
	#define MP3_SSPBUF				(SSPBUF)
	#define MP3_SPICON1				(SSP1CON1)
	#define MP3_SPICON1bits			(SSP1CON1bits)
	#define MP3_SPICON2				(SSP1CON2)
	#define MP3_SPISTAT				(SSP1STAT)
	#define MP3_SPISTATbits			(SSP1STATbits)

#elif defined(YOUR_BOARD)
// Define your own board hardware profile here

#else
	#error "Hardware profile not defined.  See available profiles in HardwareProfile.h.  Add the appropriate macro definition to your application configuration file ('TCPIPConfig.h', etc.)"
#endif


#if defined(__18CXX)	// PIC18
	// UART mapping functions for consistent API names across 8-bit and 16 or 
	// 32 bit compilers.  For simplicity, everything will use "UART" instead 
	// of USART/EUSART/etc.
	#define BusyUART()				BusyUSART()
	#define CloseUART()				CloseUSART()
	#define ConfigIntUART(a)		ConfigIntUSART(a)
	#define DataRdyUART()			DataRdyUSART()
	#define OpenUART(a,b,c)			OpenUSART(a,b,c)
	#define ReadUART()				ReadUSART()
	#define WriteUART(a)			WriteUSART(a)
	#define getsUART(a,b,c)			getsUSART(b,a)
	#define putsUART(a)				putsUSART(a)
	#define getcUART()				ReadUSART()
	#define putcUART(a)				WriteUSART(a)
	#define putrsUART(a)			putrsUSART((far rom char*)a)

#else	 // PIC24F, PIC24H, dsPIC30, dsPIC33, PIC32
	// Some A/D converter registers on dsPIC30s are named slightly differently 
	// on other procesors, so we need to rename them.
	#if defined(__dsPIC30F__)
		#define ADC1BUF0			ADCBUF0
		#define AD1CHS				ADCHS
		#define	AD1CON1				ADCON1
		#define AD1CON2				ADCON2
		#define AD1CON3				ADCON3
		#define AD1PCFGbits			ADPCFGbits
		#define AD1CSSL				ADCSSL
		#define AD1IF				ADIF
		#define AD1IE				ADIE
		#define _ADC1Interrupt		_ADCInterrupt
	#endif

	// Select which UART the STACK_USE_UART and STACK_USE_UART2TCP_BRIDGE 
	// options will use.  You can change these to U1BRG, U1MODE, etc. if you 
	// want to use the UART1 module instead of UART2.
	#define UBRG					U2BRG
	#define UMODE					U2MODE
	#define USTA					U2STA
	#define BusyUART()				BusyUART2()
	#define CloseUART()				CloseUART2()
	#define ConfigIntUART(a)		ConfigIntUART2(a)
	#define DataRdyUART()			DataRdyUART2()
	#define OpenUART(a,b,c)			OpenUART2(a,b,c)
	#define ReadUART()				ReadUART2()
	#define WriteUART(a)			WriteUART2(a)
	#define getsUART(a,b,c)			getsUART2(a,b,c)
	#if defined(__C32__)
		#define putsUART(a)			putsUART2(a)
	#else
		#define putsUART(a)			putsUART2((unsigned int*)a)
	#endif
	#define getcUART()				getcUART2()
	#define putcUART(a)				WriteUART(a)
	#define putrsUART(a)			putsUART(a)
#endif


#endif
