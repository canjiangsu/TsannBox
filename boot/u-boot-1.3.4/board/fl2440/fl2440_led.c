#include <config.h>
#include "fl2440_led.h"

// UART
#define rULCON0     (*(volatile unsigned *)0x50000000)	//UART 0 Line control
#define rUCON0      (*(volatile unsigned *)0x50000004)	//UART 0 Control
#define rUFCON0     (*(volatile unsigned *)0x50000008)	//UART 0 FIFO control
#define rUMCON0     (*(volatile unsigned *)0x5000000c)	//UART 0 Modem control
#define rUTRSTAT0   (*(volatile unsigned *)0x50000010)	//UART 0 Tx/Rx status
#define rUERSTAT0   (*(volatile unsigned *)0x50000014)	//UART 0 Rx error status
#define rUFSTAT0    (*(volatile unsigned *)0x50000018)	//UART 0 FIFO status
#define rUMSTAT0    (*(volatile unsigned *)0x5000001c)	//UART 0 Modem status
#define rUBRDIV0    (*(volatile unsigned *)0x50000028)	//UART 0 Baud rate divisor
#define WrUTXH0(ch) (*(volatile unsigned char *)0x50000020)=(unsigned char)(ch)

void fl2440_led_ctrl(unsigned int uiLed, int iSet)
{
	if (iSet == 0) {
		rGPBDAT |= uiLed;
	} else {
		rGPBDAT &= (~uiLed);
	}
	return;
}
void fl2440_led_init(void)
{
	//**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7     GPB6    GPB5    GPB4      GPB3      GPB2      GPB1      GPB0
    //Signal : LED3    Reserved LED2    Reserved LED1    LED0    Reserved  Reserved  Reserved  Reserved  Buzzer
    //Setting: OUTPUT  Reserved OUTPUT  Reserved OUTPUT  OUTPUT  Reserved  Reserved  Reserved  Reserved  OUTPUT 
    //Binary :   01  ,  11       01  ,   11      01   ,  01      11  ,     11        11   ,    11        01  
    
	rGPBCON = 0x1DD7FD;
	rGPBUP = 0x00;
	rGPBDAT &= (~FL2440_BUZZER);
	rGPBDAT |= (FL2440_LED0 | FL2440_LED1 | FL2440_LED2 | FL2440_LED3);
	return;
}

void Uart_SendByte(int data)
{
	while(!(rUTRSTAT0 & 0x2));
		WrUTXH0(data);
}
void Uart_SendStr(char *pStr)
{
	while(*pStr) {
		while(!(rUTRSTAT0 & 0x2));
			WrUTXH0(*pStr);
		pStr ++;	
	}	
}
void init_uart(void)
{
	int i;
	
	rGPHCON	= 0x0016FAAA;
	rGPHUP = 0x000007FF;

	rUFCON0 = 0x00;   //UART channel 0 FIFO control register, FIFO disable
	rUMCON0 = 0x0;   //UART chaneel 0 MODEM control register, AFC disable
	rULCON0 = 0x3;
	rUCON0 = 0x245;
#if SUCJ_START_DBG
	rUBRDIV0 = 12;   //Baud rate divisior register 0
#else
	rUBRDIV0 = 53;   //Baud rate divisior register 0
#endif
	for(i=0;i<500;i++);
#if SUCJ_START_DBG
	for (i=0; i<20; i++) {
		while(!(rUTRSTAT0 & 0x2));
			WrUTXH0('S');
	}
#endif
	Uart_SendByte('I');
}


