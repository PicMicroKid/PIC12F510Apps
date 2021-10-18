/*
 * File:   IR_detect.c compiler version XC8 v2.10
 * Author: MicroKid
 * IR sensor with IR LED and LED type IR receiver
 */ 
#include <xc.h>
// *** Microcontroller MIPs (FCY) *** //
#define FOSC_ 8000000       // * target device system clock frequency: Max Frequency is MHz for 12F510 * //
#define _XTAL_FREQ FOSC_    // * required for __delay_ms, __delay_us macros * //
// CONFIG bits that are part-specific for the PIC 12F510
#pragma config  OSC   = IntRC   //Oscillator Selection bits (internal RC oscillator)
#pragma config  WDT   = OFF     //Watchog Timer Enable bit (WDT disabled)
#pragma config  CP    = OFF     //Code protection bit (CP is ON)
#pragma config  MCLRE = OFF     //GP3/MCLRE Pin Function Select bit (GP3/MCLRE pin function is GP3)
#pragma config  IOSCFS= OFF     //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
//---------- Input and Output Pin(s) ----------
//#define	I_O_1		GPIObits.GP0	//Pin 7
#define     LED		GPIObits.GP1	//Pin 6
//#define	I_O 		GPIObits.GP2    //Pin 5
//#define	Input       GPIObits.GP3	//Pin 4: can be defined as input only
#define	IR_LED		GPIObits.GP4	//Pin 3
#define	IR_Receiver GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
// ***************************** Variables ********************************** //
unsigned char i;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
	{
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1 for calibration bit-0 should be written as 0
    TRISGPIO = 0b00101000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as input for IR_Receiver
                            //GP4 is set as output for IR_LED
                            //GP3 is set as ... for
                            //GP2 is set as ...
                            //GP1 is set as output for LED
                            //GP0 is set as ... for
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b00111100;    // All pins are set as digital
    OPTION = 0b11000111;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010);1:256 (111)
    LED = OFF;
    IR_LED = OFF;
    }
// ************************************************************************** //
void main(void)
    {
    SETUP_initialize();
    for(;;)
        {
        IR_LED = ON;
        __delay_us(25);
        if (!IR_Receiver)
            {
            IR_LED = OFF;
            __delay_ms(15);
            if (IR_Receiver)
                LED = ON;
            else LED = OFF;
            }
        else
            {
            IR_LED = OFF;
            LED = OFF;
            }
        }
    return;
    }
// ******** END *********** //
