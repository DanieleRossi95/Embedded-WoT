//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoLowPowerPic32.h								//
//																					//
//						Energy savings features for Fishino boards					//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//  Version 7.3.0 - December 2017	Initial version									//
//  Version 7.3.2 - 2018/01/05		Added support for Piranha boards				//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#define OPT_BOARD_INTERNAL
#include "FishinoLowPower.h"

#ifdef _FISHINO_PIC32_

#include <FishinoRTC.h>
#include <FishinoFlash.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#if defined(FISHINO32)
	#define ON_PIN	29
#elif defined(FISHINO_PIRANHA)
	#define ON_PIN 30
#elif defined(FISHINO_PIRANHA_PROTO)
	#define ON_PIN 30
#else
	#error "Unsupported board"
#endif


// enabled peripherals in off mode
#define		PERIPH_USB			0x0001
#define		PERIPH_SERIAL0		0x0002
#define		PERIPH_SPI			0x0004
#define		PERIPH_I2C			0x0008

// peripherals (and other) state to save before sleep
// and restore after
#ifdef _FISHINO_PIC32_

// dummy dword used to read ports
static uint32_t dummy;

enum class LP_PERIPHERALS
{
	ADC = 0, CTMU, CVREF, COMP1, COMP2,
	IC1, IC2, IC3, IC4, IC5,
	OC1, OC2, OC3, OC4, OC5,
	TIMER1, TIMER2, TIMER3, TIMER4, TIMER5,
	UART1, UART2, UART3, UART4, UART5,
	SPI1, SPI2, I2C1, I2C2, 
	USB, RTCC, REFCLK, PMP
};

struct LPSTATE
{
	// ansel and tris registers
	uint32_t ansel[6], tris[6];
	
	// peripheral on states, packed
	uint32_t pon;
	
	// peripheral power states
	uint32_t pmd[6];
	
	// change detection enables, pullups and pulldowns for all I/O
	uint32_t cncon[6], cnen[6], cnpu[6], cnpd[6];
	
	// global interrupts and priorities for all ports
	uint8_t cnenan;		// packed, 1 bit x port
	uint8_t cnprio;		// 3 bit, all ports same prio
	uint8_t cnsub;		// 2 bits, all ports same prio
	
	// the pointer to the interrupt routine
	void (*cnisr)(void);
	
	// the pointer for peripheral interrupt routines
	void (*uartisr)(void);
	void (*spiisr)(void);
	void (*i2cisr)(void);
	void (*usbisr)(void);
};
#endif

extern "C" { void _GEN_EXCPT_ADDR(); }

// dummy ISR for pin change wakeup
static void _dummy_cn_isr(void) __attribute__((interrupt(), nomips16));
static void _dummy_cn_isr(void)
{
	IFS1CLR = _IFS1_CNBIF_MASK | _IFS1_CNCIF_MASK | _IFS1_CNDIF_MASK | _IFS1_CNEIF_MASK | _IFS1_CNFIF_MASK | _IFS1_CNGIF_MASK;
	dummy = PORTB;
	dummy = PORTC;
	dummy = PORTD;
	dummy = PORTE;
	dummy = PORTF;
	dummy = PORTG;
}

// dummy ISR for other peripheral events (USB, I2C, SPI and UART(
// used just to wake us up if requested
static void _dummy_isr(void) __attribute__((interrupt(), nomips16));
static void _dummy_isr(void)
{
	// just remove ALL isr flags
	IFS0CLR = 0xffffffff;
	IFS1CLR = 0xffffffff;
	IFS2CLR = 0xffffffff;
}

// save state before going low power
static void saveState(LPSTATE &state)
{
	// save ports tris and ansel registers
	state.ansel[0] = ANSELB;
	state.ansel[1] = ANSELC;
	state.ansel[2] = ANSELD;
	state.ansel[3] = ANSELE;
	state.ansel[4] = ANSELF;
	state.ansel[5] = ANSELG;
	
	state.tris[0] = TRISB;
	state.tris[1] = TRISC;
	state.tris[2] = TRISD;
	state.tris[3] = TRISE;
	state.tris[4] = TRISF;
	state.tris[5] = TRISG;

	// save peripheral ON states
	state.pon = 0;
	if(AD1CON1bits.ON)		state.pon |= 0x00000001;
	if(CTMUCONbits.ON)		state.pon |= 0x00000002;
	if(CVRCONbits.ON)		state.pon |= 0x00000004;
	if(CM1CONbits.ON)		state.pon |= 0x00000008;
	if(CM2CONbits.ON)		state.pon |= 0x00000010;
	if(IC1CONbits.ON)		state.pon |= 0x00000020;
	if(IC2CONbits.ON)		state.pon |= 0x00000040;
	if(IC3CONbits.ON)		state.pon |= 0x00000080;
	if(IC4CONbits.ON)		state.pon |= 0x00000100;
	if(IC5CONbits.ON)		state.pon |= 0x00000200;
	if(OC1CONbits.ON)		state.pon |= 0x00000400;
	if(OC2CONbits.ON)		state.pon |= 0x00000800;
	if(OC3CONbits.ON)		state.pon |= 0x00001000;
	if(OC4CONbits.ON)		state.pon |= 0x00002000;
	if(OC5CONbits.ON)		state.pon |= 0x00004000;
	if(T1CONbits.ON)		state.pon |= 0x00008000;
	if(T2CONbits.ON)		state.pon |= 0x00010000;
	if(T3CONbits.ON)		state.pon |= 0x00020000;
	if(T4CONbits.ON)		state.pon |= 0x00040000;
	if(T5CONbits.ON)		state.pon |= 0x00080000;
	if(U1MODEbits.ON)		state.pon |= 0x00100000;
	if(U2MODEbits.ON)		state.pon |= 0x00200000;
	if(U3MODEbits.ON)		state.pon |= 0x00400000;
	if(U4MODEbits.ON)		state.pon |= 0x00800000;
	if(SPI1CONbits.ON)		state.pon |= 0x01000000;
	if(SPI2CONbits.ON)		state.pon |= 0x02000000;
	if(I2C1CONbits.ON)		state.pon |= 0x04000000;
	if(I2C2CONbits.ON)		state.pon |= 0x08000000;
	if(U1PWRCbits.USBPWR)	state.pon |= 0x10000000;
	if(RTCCONbits.ON)		state.pon |= 0x20000000;
	if(REFOCONbits.ON)		state.pon |= 0x40000000;
	if(PMCONbits.ON)		state.pon |= 0x80000000;
	
	// save peripherals power states
	state.pmd[0] = PMD1;
	state.pmd[1] = PMD2;
	state.pmd[2] = PMD3;
	state.pmd[3] = PMD4;
	state.pmd[4] = PMD5;
	state.pmd[5] = PMD6;
	
	// save pin change detection control bits
	state.cncon[0] = CNCONB;
	state.cncon[1] = CNCONC;
	state.cncon[2] = CNCOND;
	state.cncon[3] = CNCONE;
	state.cncon[4] = CNCONF;
	state.cncon[5] = CNCONG;
	
	// save pin change detection enabled states
	state.cnen[0] = CNENB;
	state.cnen[1] = CNENC;
	state.cnen[2] = CNEND;
	state.cnen[3] = CNENE;
	state.cnen[4] = CNENF;
	state.cnen[5] = CNENG;
	
	// save pullup states
	state.cnpu[0] = CNPUB;
	state.cnpu[1] = CNPUC;
	state.cnpu[2] = CNPUD;
	state.cnpu[3] = CNPUE;
	state.cnpu[4] = CNPUF;
	state.cnpu[5] = CNPUG;
	
	// save pulldown states
	state.cnpd[0] = CNPDB;
	state.cnpd[1] = CNPDC;
	state.cnpd[2] = CNPDD;
	state.cnpd[3] = CNPDE;
	state.cnpd[4] = CNPDF;
	state.cnpd[5] = CNPDG;
	
	// save global change detection interrupt enables
	state.cnenan = 0;
	if(IEC1bits.CNBIE)		state.cnenan |= 0x01;
	if(IEC1bits.CNCIE)		state.cnenan |= 0x02;
	if(IEC1bits.CNDIE)		state.cnenan |= 0x04;
	if(IEC1bits.CNEIE)		state.cnenan |= 0x08;
	if(IEC1bits.CNFIE)		state.cnenan |= 0x10;
	if(IEC1bits.CNGIE)		state.cnenan |= 0x20;
	
	// save change detection interrupt priorities
	state.cnprio	= IPC8bits.CNIP;
	state.cnsub		= IPC8bits.CNIS;
	
	// save the pointer to the interrupt routine
	state.cnisr = getIntVector(_CHANGE_NOTICE_VECTOR);

#if defined(FISHINO32)
	state.uartisr = getIntVector(_UART_4_VECTOR);
	state.spiisr = getIntVector(_SPI_2_VECTOR);
#elif defined(FISHINO_PIRANHA)
	state.uartisr = getIntVector(_UART_3_VECTOR);
	state.spiisr = getIntVector(_SPI_1_VECTOR);
#elif defined(FISHINO_PIRANHA_PROTO)
	state.uartisr = getIntVector(_UART_1_VECTOR);
	state.spiisr = getIntVector(_SPI_1_VECTOR);
#else
	#error "Unsupported board"
#endif

	state.i2cisr = getIntVector(_I2C_1_VECTOR);
	state.usbisr = getIntVector(_USB_1_VECTOR);
}

// restore state after sleep
static void restoreState(LPSTATE const &state)
{
	// disable interrupts and allow access to special regs
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	CFGCONbits.PMDLOCK = 0;
	SYSKEY = 0x33333333;


	// restore peripherals interrupts
#if defined(FISHINO32)
	setIntVector(_UART_4_VECTOR, state.uartisr);
	setIntVector(_SPI_2_VECTOR, state.spiisr);
#elif defined(FISHINO_PIRANHA)
	setIntVector(_UART_3_VECTOR, state.uartisr);
	setIntVector(_SPI_1_VECTOR, state.spiisr);
#elif defined(FISHINO_PIRANHA_PROTO)
	setIntVector(_UART_1_VECTOR, state.uartisr);
	setIntVector(_SPI_1_VECTOR, state.spiisr);
#else
	#error "Unsupported board"
#endif

	setIntVector(_I2C_1_VECTOR, state.i2cisr);
	setIntVector(_USB_1_VECTOR, state.usbisr);

	// restore the pointer to the interrupt routine
	setIntVector(_CHANGE_NOTICE_VECTOR, state.cnisr);
	
	// restore change detection interrupt priorities
	IPC8bits.CNIP	= state.cnprio;
	IPC8bits.CNIS	= state.cnsub;

	// restore global change detection interrupt enables
	IEC1bits.CNBIE = (state.cnenan & 0x01 ? 1 : 0);
	IEC1bits.CNCIE = (state.cnenan & 0x02 ? 1 : 0);
	IEC1bits.CNDIE = (state.cnenan & 0x04 ? 1 : 0);
	IEC1bits.CNEIE = (state.cnenan & 0x08 ? 1 : 0);
	IEC1bits.CNFIE = (state.cnenan & 0x10 ? 1 : 0);
	IEC1bits.CNGIE = (state.cnenan & 0x20 ? 1 : 0);

	// restore pin change detection control bits
	CNCONB	= state.cncon[0];
	CNCONC	= state.cncon[1];
	CNCOND	= state.cncon[2];
	CNCONE	= state.cncon[3];
	CNCONF	= state.cncon[4];
	CNCONG	= state.cncon[5];
	
	// restore pin change detection enabled states
	CNENB	= state.cnen[0];
	CNENC	= state.cnen[1];
	CNEND	= state.cnen[2];
	CNENE	= state.cnen[3];
	CNENF	= state.cnen[4];
	CNENG	= state.cnen[5];
	
	// restore pullup states
	CNPUB	= state.cnpu[0];
	CNPUC	= state.cnpu[1];
	CNPUD	= state.cnpu[2];
	CNPUE	= state.cnpu[3];
	CNPUF	= state.cnpu[4];
	CNPUG	= state.cnpu[5];
	
	// restore pulldown states
	CNPDB	= state.cnpd[0];
	CNPDC	= state.cnpd[1];
	CNPDD	= state.cnpd[2];
	CNPDE	= state.cnpd[3];
	CNPDF	= state.cnpd[4];
	CNPDG	= state.cnpd[5];

	// restore peripherals power states
	PMD1 = state.pmd[0];
	PMD2 = state.pmd[1];
	PMD3 = state.pmd[2];
	PMD4 = state.pmd[3];
	PMD5 = state.pmd[4];
	PMD6 = state.pmd[5];

	// restore peripheral ON states
	AD1CON1bits.ON		= (state.pon & 0x00000001 ? 1 : 0);
	CTMUCONbits.ON		= (state.pon & 0x00000002 ? 1 : 0);
	CVRCONbits.ON		= (state.pon & 0x00000004 ? 1 : 0);
	CM1CONbits.ON		= (state.pon & 0x00000008 ? 1 : 0);
	CM2CONbits.ON		= (state.pon & 0x00000010 ? 1 : 0);
	IC1CONbits.ON		= (state.pon & 0x00000020 ? 1 : 0);
	IC2CONbits.ON		= (state.pon & 0x00000040 ? 1 : 0);
	IC3CONbits.ON		= (state.pon & 0x00000080 ? 1 : 0);
	IC4CONbits.ON		= (state.pon & 0x00000100 ? 1 : 0);
	IC5CONbits.ON		= (state.pon & 0x00000200 ? 1 : 0);
	OC1CONbits.ON		= (state.pon & 0x00000400 ? 1 : 0);
	OC2CONbits.ON		= (state.pon & 0x00000800 ? 1 : 0);
	OC3CONbits.ON		= (state.pon & 0x00001000 ? 1 : 0);
	OC4CONbits.ON		= (state.pon & 0x00002000 ? 1 : 0);
	OC5CONbits.ON		= (state.pon & 0x00004000 ? 1 : 0);
	T1CONbits.ON		= (state.pon & 0x00008000 ? 1 : 0);
	T2CONbits.ON		= (state.pon & 0x00010000 ? 1 : 0);
	T3CONbits.ON		= (state.pon & 0x00020000 ? 1 : 0);
	T4CONbits.ON		= (state.pon & 0x00040000 ? 1 : 0);
	T5CONbits.ON		= (state.pon & 0x00080000 ? 1 : 0);
	U1MODEbits.ON		= (state.pon & 0x00100000 ? 1 : 0);
	U2MODEbits.ON		= (state.pon & 0x00200000 ? 1 : 0);
	U3MODEbits.ON		= (state.pon & 0x00400000 ? 1 : 0);
	U4MODEbits.ON		= (state.pon & 0x00800000 ? 1 : 0);
	SPI1CONbits.ON		= (state.pon & 0x01000000 ? 1 : 0);
	SPI2CONbits.ON		= (state.pon & 0x02000000 ? 1 : 0);
	I2C1CONbits.ON		= (state.pon & 0x04000000 ? 1 : 0);
	I2C2CONbits.ON		= (state.pon & 0x08000000 ? 1 : 0);
	U1PWRCbits.USBPWR	= (state.pon & 0x10000000 ? 1 : 0);
	RTCCONbits.ON		= (state.pon & 0x20000000 ? 1 : 0);
	REFOCONbits.ON		= (state.pon & 0x40000000 ? 1 : 0);
	PMCONbits.ON		= (state.pon & 0x80000000 ? 1 : 0);
	
	// restore ports tris and ansel registers
	TRISB = state.tris[0];
	TRISC = state.tris[1];
	TRISD = state.tris[2];
	TRISE = state.tris[3];
	TRISF = state.tris[4];
	TRISG = state.tris[5];

	ANSELB = state.ansel[0];
	ANSELC = state.ansel[1];
	ANSELD = state.ansel[2];
	ANSELE = state.ansel[3];
	ANSELF = state.ansel[4];
	ANSELG = state.ansel[5];

	// re-lock regs access
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	CFGCONbits.PMDLOCK = 1;
	SYSKEY = 0x33333333;
	
	// clear interrupt flags, just in case
	IFS1bits.CNBIF = 0;
	IFS1bits.CNCIF = 0;
	IFS1bits.CNDIF = 0;
	IFS1bits.CNEIF = 0;
	IFS1bits.CNFIF = 0;
	IFS1bits.CNGIF = 0;

	CNSTATB	= 0;
	CNSTATC	= 0;
	CNSTATD	= 0;
	CNSTATE	= 0;
	CNSTATF	= 0;
	CNSTATG	= 0;
	
	// re-enable interrupts
	enableInterrupts();
}

// power off requested peripherals
// parameters:
// peripheralsEnabled		packed flags for peripherals that must be kept on
// interruptPorts			6 uint32_t array of flags for ports that must be kept active because used to wake us up
// usedPorts				6 uint32_t array of flags for ports that must be kept active because we use them
static void offPeripherals(
	uint16_t const &peripheralsEnabled,
	uint32_t const *interruptPorts,
	uint32_t const *usedPorts)
{
	// all our interesting peripherals are on PMD5
	// power control register
	uint32_t pd5 = 0xffffffff;

	// if we want USB disabled, wait till it's free before
	// disabling it
	if(peripheralsEnabled & PERIPH_USB)
	{
		while (U1PWRCbits.UACTPND)
			;
		U1PWRCbits.USUSPEND = 1;
	}

	// disable interrupts and enable write to power registers
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	CFGCONbits.PMDLOCK = 0;
	SYSKEY = 0x33333333;
	
	// turn off not needed peripherals
	if(peripheralsEnabled & PERIPH_USB)
	{
		pd5 &= ~_PMD5_USBMD_MASK;
		setIntVector(_USB_1_VECTOR, _dummy_isr);
	}
	else
		U1PWRCbits.USBPWR = 0;

	U2MODEbits.ON = 0;

#if defined(FISHINO32)

	U1MODEbits.ON = 0;
	U3MODEbits.ON = 0;
	if(peripheralsEnabled & PERIPH_SERIAL0)
	{
		pd5 &= ~_PMD5_U4MD_MASK;
		U4MODEbits.WAKE = 1;
		setIntVector(_UART_4_VECTOR, _dummy_isr);
		IEC2bits.U4RXIE = 1;
		IPC9bits.U4IP = 7;
	}
	else
		U4MODEbits.ON = 0;

	if(peripheralsEnabled & PERIPH_SPI)
	{
		setIntVector(_SPI_2_VECTOR, _dummy_isr);
		pd5 &= ~_PMD5_SPI2MD_MASK;
	}
	else
		SPI2CONbits.ON = 0;
	SPI1CONbits.ON = 0;

#elif defined(FISHINO_PIRANHA)

	U1MODEbits.ON = 0;
	U4MODEbits.ON = 0;
	if(peripheralsEnabled & PERIPH_SERIAL0)
	{
		pd5 &= ~_PMD5_U3MD_MASK;
		U3MODEbits.WAKE = 1;
		setIntVector(_UART_3_VECTOR, _dummy_isr);
		IEC2bits.U3RXIE = 1;
		IPC9bits.U3IP = 7;
	}
	else
		U3MODEbits.ON = 0;

	if(peripheralsEnabled & PERIPH_SPI)
	{
		pd5 &= ~_PMD5_SPI1MD_MASK;
		setIntVector(_SPI_1_VECTOR, _dummy_isr);
	}
	else
		SPI1CONbits.ON = 0;
	SPI2CONbits.ON = 0;

#elif defined(FISHINO_PIRANHA_PROTO)

	U3MODEbits.ON = 0;
	U4MODEbits.ON = 0;
	if(peripheralsEnabled & PERIPH_SERIAL0)
	{
		pd5 &= ~_PMD5_U1MD_MASK;
		U1MODEbits.WAKE = 1;
		setIntVector(_UART_1_VECTOR, _dummy_isr);
		IEC1bits.U1RXIE = 1;
		IPC7bits.U1IP = 7;
	}
	else
		U1MODEbits.ON = 0;

	if(peripheralsEnabled & PERIPH_SPI)
	{
		pd5 &= ~_PMD5_SPI1MD_MASK;
		setIntVector(_SPI_1_VECTOR, _dummy_isr);
	}
	else
		SPI1CONbits.ON = 0;
	SPI2CONbits.ON = 0;

#else

	#error "Unsupported board"

#endif

	if(peripheralsEnabled & PERIPH_I2C)
	{
		pd5 &= ~_PMD5_I2C1MD_MASK;
		setIntVector(_I2C_1_VECTOR, _dummy_isr);
	}
	else
		I2C1CONbits.ON = 0;
	I2C1CONbits.ON = 0;
	
	AD1CON1bits.ON		= 0;
	CTMUCONbits.ON		= 0;
	CVRCONbits.ON		= 0;
	CM1CONbits.ON		= 0;
	CM2CONbits.ON		= 0;
	IC1CONbits.ON		= 0;
	IC2CONbits.ON		= 0;
	IC3CONbits.ON		= 0;
	IC4CONbits.ON		= 0;
	IC5CONbits.ON		= 0;
	OC1CONbits.ON		= 0;
	OC2CONbits.ON		= 0;
	OC3CONbits.ON		= 0;
	OC4CONbits.ON		= 0;
	OC5CONbits.ON		= 0;
	T1CONbits.ON		= 0;
	T2CONbits.ON		= 0;
	T3CONbits.ON		= 0;
	T4CONbits.ON		= 0;
	T5CONbits.ON		= 0;
	REFOCONbits.ON		= 0;
	PMCONbits.ON		= 0;

	// un-power not needed peripherals
	PMD1SET = 0xffffffff;
	PMD2SET = 0xffffffff;
	PMD3SET = 0xffffffff;
	PMD4SET = 0xffffffff;
	PMD5SET = pd5;
	PMD6SET = 0xfffffffe; // leave RTC ON !!!
	
	// re-lock writing to power registers
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	CFGCONbits.PMDLOCK = 1;
	SYSKEY = 0x33333333;
	
	// put all non-active ports in analog mode
	TRISBSET = ~(interruptPorts[0] | usedPorts[0]);
	ANSELBSET = TRISB;
	TRISCSET = ~(interruptPorts[1] | usedPorts[1]);
	ANSELCSET = TRISC;
	
	uint32_t t = ~(interruptPorts[2] | usedPorts[2]);
#ifdef FISHINO32
	// PMD7 == D31 (ESP_CHPD)
	t &= ~(1 << 7);
#else
	// PMD6 == D32 (ESP_CHPD) | PMD11 == D30 (/OFF)
	t &= ~((1 << 6) | (1 << 11));
#endif
	TRISDSET = t;
	ANSELDSET = TRISD;

	TRISESET = ~(interruptPorts[3] | usedPorts[3]);
	ANSELESET = TRISE;

	t = ~(interruptPorts[4] | usedPorts[4]);
#ifdef FISHINO32
	// PMF5 == D29 (/OFF)
	t &= ~(1 << 5);
#endif
	TRISFSET = t;
	ANSELFSET = TRISF;

	TRISGSET =~(interruptPorts[5] | usedPorts[5]);
	ANSELGSET = TRISG;
	
	// re-enable interrupts
	enableInterrupts();
}

// setup wake on change interrupt pins
static void setupInterrupts(uint32_t const *interruptPorts, uint32_t const *pullUps, uint32_t const *pullDowns)
{
	// disable interrupts
	disableInterrupts();
	
	bool en = false;
	
	if(interruptPorts[0])
	{
		en = true;
		TRISBSET = interruptPorts[0];
		ANSELBCLR = interruptPorts[0];
		CNCONBbits.ON = 1;
		CNPUB = pullUps[0];
		CNPDB = pullDowns[0];
		CNENB = interruptPorts[0];
		CNSTATB = 0;
		dummy = PORTB;
		IFS1CLR = _IFS1_CNBIF_MASK;
		IEC1SET = _IEC1_CNBIE_MASK;
	}
	if(interruptPorts[1])
	{
		en = true;
		TRISCSET = interruptPorts[1];
		ANSELCCLR = interruptPorts[1];
		CNCONCbits.ON = 1;
		CNPUC = pullUps[1];
		CNPDC = pullDowns[1];
		CNENC = interruptPorts[1];
		CNSTATC = 0;
		dummy = PORTC;
		IFS1CLR = _IFS1_CNCIF_MASK;
		IEC1SET = _IEC1_CNCIE_MASK;
	}
	if(interruptPorts[2])
	{
		en = true;
		TRISDSET = interruptPorts[2];
		ANSELDCLR = interruptPorts[2];
		CNCONDbits.ON = 1;
		CNPUD = pullUps[2];
		CNPDD = pullDowns[2];
		CNEND = interruptPorts[2];
		CNSTATD = 0;
		dummy = PORTD;
		IFS1CLR = _IFS1_CNDIF_MASK;
		IEC1SET = _IEC1_CNDIE_MASK;
	}
	if(interruptPorts[3])
	{
		en = true;
		TRISESET = interruptPorts[3];
		ANSELECLR = interruptPorts[3];
		CNCONEbits.ON = 1;
		CNPUE = pullUps[3];
		CNPDE = pullDowns[3];
		CNENE = interruptPorts[3];
		CNSTATE = 0;
		dummy = PORTE;
		IFS1CLR = _IFS1_CNEIF_MASK;
		IEC1SET = _IEC1_CNEIE_MASK;
	}
	if(interruptPorts[4])
	{
		en = true;
		TRISFSET = interruptPorts[4];
		ANSELFCLR = interruptPorts[4];
		CNCONFbits.ON = 1;
		CNPUF = pullUps[4];
		CNPDF = pullDowns[4];
		CNENF = interruptPorts[4];
		CNSTATF = 0;
		dummy = PORTF;
		IFS1CLR = _IFS1_CNFIF_MASK;
		IEC1SET = _IEC1_CNFIE_MASK;
	}
	if(interruptPorts[5])
	{
		en = true;
		TRISGSET = interruptPorts[5];
		ANSELGCLR = interruptPorts[5];
		CNCONGbits.ON = 1;
		CNPUG = pullUps[5];
		CNPDG = pullDowns[5];
		CNENG = interruptPorts[5];
		CNSTATG = 0;
		dummy = PORTG;
		IFS1CLR = _IFS1_CNGIF_MASK;
		IEC1SET = _IEC1_CNGIE_MASK;
	}
	
	// if some int has been enabled, setup priority
	// and interrupt handler
	if(en)
	{
		IPC8bits.CNIP = 7;
		IPC8bits.CNIS = 0;
		setIntVector(_CHANGE_NOTICE_VECTOR, _dummy_cn_isr);
	}

	// re-enable interrupts
	enableInterrupts();
}

// private constructor -- we don't want more objects
// besides the one created by macro
FishinoLowPowerClass::FishinoLowPowerClass()
{
	// wifi module ON by default
	_wifiDisabled = false;
	
	// no peripherals enabled by default
	_peripheralsEnabled = 0;
	
	// initialize map of used pins for external interrupt wakeup
	// ports b..g, 6 32 bit ports total)
	memset(_intPins, 0, sizeof(_intPins));
	
	// initialize used I/O to none
	memset(_usedIOs, 0, sizeof(_usedIOs));

	// ensure that WIFI module is on
	digitalWrite(ESP_CHPD_PIN, HIGH);
	pinMode(ESP_CHPD_PIN, OUTPUT);
}

// put board on sleep for requested time
void FishinoLowPowerClass::deepSleep(uint32_t ms)
{
	// round time to next second
	if(ms != 0)
	{
		if(ms < 500)
			ms = 1;
		else
			ms = (ms + 500) / 1000;
	}
	
	// store wifi module state
	bool wifiDisabled = _wifiDisabled;

	// turn off wifi module, if needed
	if(!_wifiDisabled)
		wifiOff();
	
	// save peripherals states
	LPSTATE state;
	saveState(state);
	
	// turn off all not needed peripherals and ports
	offPeripherals(_peripheralsEnabled, _intPins, _usedIOs);
	
	// setup wake interrupts
	setupInterrupts(_intPins, _intPullups, _intPulldowns);

	// if we gave a time, setup the RTC to wake up
	RTCAlarmState rtcState;
	if(ms)
	{
		// save current alarm state
		RTC.saveAlarmState(rtcState);
		
		// disable RTC alarm
		RTC.disableAlarm();
		
		// clear alarm interrupt
		RTC.clearAlarmHandler();
		
		// one-shot alarm
		RTC.setAlarmMode(AlarmModes::ALARM_ONCE);
		
		// setup requested time and start RTC alarm
		RTC.setAlarm(ms);
	}

	// turn off board power if requested
	if(_fullPowerOff)
	{
		pinMode(ON_PIN, OUTPUT);
		digitalWrite(ON_PIN, LOW);
	}
	
	// put pic in halt state if we requested no
	// peripherals to stay on, otherwise put
	// on idle state
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
/*
	if(_peripheralsEnabled)
		OSCCONCLR = 0x10;
	else
*/
		OSCCONSET = 0x10;
	SYSKEY = 0x33333333;
	enableInterrupts();

	// disable core-timer interrupts
	// we don't want ti be awaken by it
	IEC0CLR = 1;
	
	// halt cpu	
	asm volatile("wait");
	
	// re-enable core timer interrupts
	IEC0SET = 1;

	// turn on board power in case it was turned off
	digitalWrite(ON_PIN, HIGH);
	
	// usb leave suspend...
	// it doesn't work good anyways, so leave it alone
/*
	U1CONbits.RESUME = 1;
	delay(15);
	U1CONbits.RESUME = 0;
	delay(25);
*/
	// if used RTC restore its state
	if(ms)
	{
		// disable RTC alarm
		RTC.disableAlarm();
		
		RTC.restoreAlarmState(rtcState);
	}
	
	// restore peripherals and interrupts states
	restoreState(state);
	
	// re-enable wifi module if it was on
	if(!wifiDisabled)
		wifiOn();
}

// enable wakeup from external pin state change
bool FishinoLowPowerClass::enableInterruptPin(uint16_t pin, bool enable)
{
	// if out of range, just return false
	if(pin >= NUM_DIGITAL_PINS)
		return false;
	
	// get port offset from first
	// and bit inside it
	uint8_t port = digital_pin_to_port_PGM[pin] - _IOPORT_PB;
	uint32_t mask = digital_pin_to_bit_mask_PGM[pin];
	
	if(enable)
		_intPins[port] |= mask;
	else
		_intPins[port] &= ~mask;
	
	return true;
}

bool FishinoLowPowerClass::enableInterruptPinPullup(uint16_t pin, bool enable)
{
	// if out of range, just return false
	if(pin >= NUM_DIGITAL_PINS)
		return false;
	
	// get port offset from first
	// and bit inside it
	uint8_t port = digital_pin_to_port_PGM[pin] - _IOPORT_PB;
	uint32_t mask = digital_pin_to_bit_mask_PGM[pin];
	
	if(enable)
		_intPullups[port] |= mask;
	else
		_intPullups[port] &= ~mask;
	
	return true;
}

bool FishinoLowPowerClass::enableInterruptPinPulldown(uint16_t pin, bool enable)
{
	// if out of range, just return false
	if(pin >= NUM_DIGITAL_PINS)
		return false;
	
	// get port offset from first
	// and bit inside it
	uint8_t port = digital_pin_to_port_PGM[pin] - _IOPORT_PB;
	uint32_t mask = digital_pin_to_bit_mask_PGM[pin];
	
	if(enable)
		_intPulldowns[port] |= mask;
	else
		_intPulldowns[port] &= ~mask;
	
	return true;
}

bool FishinoLowPowerClass::portUsed(uint16_t pin, bool used)
{
	// if out of range, just return false
	if(pin >= NUM_DIGITAL_PINS)
		return false;
	
	// get port offset from first
	// and bit inside it
	uint8_t port = digital_pin_to_port_PGM[pin] - _IOPORT_PB;
	uint32_t mask = digital_pin_to_bit_mask_PGM[pin];
	
	if(used)
		_usedIOs[port] |= mask;
	else
		_usedIOs[port] &= ~mask;
	
	return true;
}

// NOTE : USB wakeup works ONLY if board is not in full power off mode
void FishinoLowPowerClass::enableUSBWakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_USB;
	else
		_peripheralsEnabled &= ~PERIPH_USB;
}

void FishinoLowPowerClass::enableSerial0Wakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL0;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL0;
}

// enable I2C/Wire wakeup
void FishinoLowPowerClass::enableI2CWakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SPI;
	else
		_peripheralsEnabled &= ~PERIPH_SPI;
}

// enable SPI wakeup
void FishinoLowPowerClass::enableSPIWakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_I2C;
	else
		_peripheralsEnabled &= ~PERIPH_I2C;
}

#endif
