/*
		PERIPHERAL DISABLE REGISTERS

ADC1							AD1MD		PMD1<0>
CTMU							CTMUMD		PMD1<8>
Comparator Voltage Reference	CVRMD		PMD1<12>
Comparator 1					CMP1MD		PMD2<0>
Comparator 2					CMP2MD		PMD2<1>
Input Capture 1					IC1MD		PMD3<0>
Input Capture 2					IC2MD		PMD3<1>
Input Capture 3					IC3MD		PMD3<2>
Input Capture 4					IC4MD		PMD3<3>
Input Capture 5					IC5MD		PMD3<4>
Output Compare 1				OC1MD		PMD3<16>
Output Compare 2				OC2MD		PMD3<17>
Output Compare 3				OC3MD		PMD3<18>
Output Compare 4				OC4MD		PMD3<19>
Output Compare 5				OC5MD		PMD3<20>
Timer1							T1MD		PMD4<0>
Timer2							T2MD		PMD4<1>
Timer3							T3MD		PMD4<2>
Timer4							T4MD		PMD4<3>
Timer5							T5MD		PMD4<4>
UART1							U1MD		PMD5<0>
UART2							U2MD		PMD5<1>
UART3							U3MD		PMD5<2>
UART4							U4MD		PMD5<3>
UART5							U5MD		PMD5<4>
SPI1							SPI1MD		PMD5<8>
SPI2							SPI2MD		PMD5<9>
I2C1							I2C1MD		PMD5<16>
I2C2							I2C2MD		PMD5<17>
USB								USBMD		PMD5<24>
RTCC							RTCCMD		PMD6<0>
Reference Clock Output			REFOMD		PMD6<1>
PMP								PMPMD		PMD6<16>

// this one must be cleared before writing to PMDxx registers
PMD write lock bit				PMDLOCK		CFGCON<12>

// to modify PMDLOCK the unlock sequence must be executed:
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	
	CFGCON modify
	
	SYSKEY = 0x33333333;
	
	enableInterrupts();

*/
