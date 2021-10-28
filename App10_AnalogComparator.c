/*
 * File:   MicroKid_Template.c
 * Author: MicroKid
 * Compiler: 
 * Revision History:
 * Description:
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
#pragma config  IOSCFS= ON     //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
//#define I_O   GPIObits.GP0    //Pin 7
//#define I_O	GPIObits.GP1	//Pin 6
#define Relay   GPIObits.GP2    //Pin 5
//#define I_O   GPIObits.GP3    //Pin 4:can be defined as input only
//#define I_O   GPIObits.GP4    //Pin 3
//#define I_O   GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define FALSE   0
#define TRUE    1
#define OFF     0
#define ON      1
// ***************************** Variables ********************************** //
unsigned char i;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
	{
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1, for calibration bit-0 should be written as 0
    TRISGPIO = 0b00001011;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as ... for
                            //GP4 is set as ... for
                            //GP3 is set as only input for ...
                            //GP2 is set as output for RELAY
                            //GP1 is set as input for C1IN-
                            //GP0 is set as ... for C1IN+
    
    CM1CON0 = 0b11111111;   //Analog Comparator set OFF
                            //bit 7: C1OUT: Comparator Output bit
                                //:1 = VIN+ > VIN-
                                //:0 = VIN+ < VIN-
                            //bit 6: C1OUTEN: Comparator Output Enable bit (Note: Overrides T0CS bit for TRIS control of RB2.)
                                //:1 = Output of comparator is NOT placed on the C1OUT pin
                                //:0 = Output of comparator is placed in the C1OUT pin
                            //bit 5: C1POL: Comparator Output Polarity bit
                                //:1 = Output of comparator is not inverted
                                //:0 = Output of comparator is inverted
                            //bit 4: C1T0CS: Comparator TMR0 Clock Source bit
                                //:1 = TMR0 clock source selected by T0CS control bit
                                //:0 = Comparator output used as TMR0 clock source
                            //bit3: C1ON: Comparator Enable bit
                                //:1 = Comparator is ON
                                //:0 = Comparator is OFF
                            //bit 2: C1NREF: Comparator Negative Reference Select bit
                                //:1 = C1IN- pin
                                //:0 = 0.6V internal reference
                            //bit 1: C1PREF: Comparator Positive Reference Select bit
                                //:1 = C1IN+ pin
                                //:0 = C1IN- pin
                            //bit 0: C1WU: Comparator Wake-up On Change Enable bit
                                //:1 = Wake-up On Comparator Change is disabled
                                //:0 = Wake-up On Comparator Change is enabled
    
    ADCON0 = 0b00111100;    // All pins are set as digital
                            // bit 7-6: ADC Analog Pin Select
                                //:00 = No pins configured for analog input
                                //:01 = AN2 configured as an analog input
                                //:10 = AN2 and AN0 configured as analog inputs
                                //:11 = AN2, AN1 and AN0 configured as analog inputs
                            //bit 5-4: ADC conversion Clock Selection; 11 = INTOSC/4
                            //bit 3-2: ADC Channel Select bits;
                                //:00 = Channel AN0
                                //:01 = Channel AN1
                                //:10 = Channel AN2
                                //:11 = 0.6V absolute voltage reference
                            //bit 1: GO/DONE: 1 = ADC conversion in progress
                            //bit 0: ADC Enable bit; 1 = ADC module is operating
    
    OPTION = 0b11000011;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010); 1:16 (011); 1:256 (111)
    }
// ***************************** MAIN loop ********************************** //
void main(void) 
    {
    SETUP_initialize(); //initialize setup
    // ******* for(;;) loop *** //
    Relay = ON; //Turn-ON the RELAY at startup
    for (;;)
        {
        if (C1OUT)      // If an ERROR
            {
            __delay_ms(10); //wait 10ms ...
            if (C1OUT)      //than check again
                {           //still there is an ERROR
                Relay = OFF;    //Turn-OFF Relay
                __delay_ms (1000);  //wait 1 sec
                Relay = ON;         //Turn-ON Relay
                __delay_ms (10);    //wait 10 ms
                if (C1OUT)          //check again
                    {               //if ERROR
                    Relay = OFF;    //Turn-OFF Relay
                    while(1);       //wait for service
                    }
                }
            }
        }
    }
// ******************************** END ************************************* //
