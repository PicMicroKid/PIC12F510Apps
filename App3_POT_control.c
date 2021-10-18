/*
 * File:   POT_control.c compiler version XC8 v2.10
 * Author: MicroKid
 * LED blink with POT
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
//#define	I_O		GPIObits.GP0	//Pin 7
//#define	I_O		GPIObits.GP1	//Pin 6
//#define	LED 	GPIObits.GP2        //Pin 5
//#define	Input   GPIObits.GP3	//Pin 4: can be defined as input only
#define	LED         GPIObits.GP4	//Pin 3
//#define	I_O     GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
// ***************************** Variables ********************************** //
unsigned char i, POT_value;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
	{
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1 for calibration bit-0 should be written as 0
    TRISGPIO = 0b00001100;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as ... for
                            //GP4 is set as ... for
                            //GP3 is set as output for LED
                            //GP2 is set as input for POT (ADC)
                            //GP1 is set as ... for
                            //GP0 is set as ... for
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b01111000;    // AN2 (GP2) is selected as Analog Channel
                            // bit 7-6: ADC Analog Pin Select
                                //:00 = No pins configured for analog input
                                //:01 = AN2 configured as an analog input
                                //:10 = AN2 and AN0 configured as analog inputs
                                //:11 = AN2, AN1 and AN0 configured as analog inputs
                            //bit 5-4: ADC conversion Clock Selection; 11 = INTOSC/4
                            //bit 3-2: ADC Channel Select bits;
                                //00 = Channel AN0
                                //:01 = Channel AN1
                                //:10 = Channel AN2
                                //:11 = 0.6V absolute voltage reference
                            //bit 1: GO/DONE: 1 = ADC conversion in progress
                            //bit 0: ADC Enable bit; 1 = ADC module is operating
    OPTION = 0b11000111;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010);1:256 (111)
    LED = OFF;
    }
// ************************************************************************** //
void main(void)
    {
    SETUP_initialize();     // Initialize SETUP
    ADCON0bits.GO = TRUE;   //Start ADC to read POT value
    POT_value = 0;          //Initial value of POT_value is 0
    LED = OFF;              //Initial state of LED is OFF
    __delay_ms(10);         //delay for AD conversion (couple of ms is enough)
    // ************************** //
    for(;;)
        {
        if (!ADCON0bits.GO)     //if GO bit is clear, ADC is done and ready to read
            {
            POT_value = ADRES;    //ADRES is the register ADC value stored in
            ADCON0bits.GO = TRUE; //Restart ADC 
            }
        for (i=0;i<POT_value;i++)   // Wait POT_value x 2ms
            __delay_ms(2);          // it can be changed as you want.
        LED = ON;
        for (i=0;i<POT_value;i++)
            __delay_ms(2);
        LED = OFF;
        }
    return;
    }
// ******** END *********** //
