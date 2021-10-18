/*
 * File:   LED_blink.c
 * Author: MicroKid
 * LED blink with TMR0 or _delay_ms()
 * Created on March 18, 2020, 1:26 PM
 */
#include <xc.h>
// *** Microcontroller MIPs (FCY) *** //
#define FOSC_ 4000000       // * target device system clock frequency: Max Frequency is MHz for 12F510 * //
#define _XTAL_FREQ FOSC_    // * required for __delay_ms, __delay_us macros * //
// CONFIG bits that are part-specific for the PIC 12F510
#pragma config  OSC   = IntRC   //Oscillator Selection bits (internal RC oscillator)
#pragma config  WDT   = OFF     //Watchog Timer Enable bit (WDT disabled)
#pragma config  CP    = OFF     //Code protection bit (CP is ON)
#pragma config  MCLRE = OFF     //GP3/MCLRE Pin Function Select bit (GP3/MCLRE pin function is GP3)
#pragma config  IOSCFS= OFF     //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
//#define I_O_1   GPIObits.GP0    //Pin 7
//#define I_O_2   GPIObits.GP1    //Pin 6
#define LED     GPIObits.GP1    //Pin 5
//#define I_O_3     GPIObits.GP0    //Pin 5
//#define Input   GPIObits.GP3    //Pin 4:can be defined as input only
//#define Input   GPIObits.GP4    //Pin 3
//#define I_O_4   GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define FALSE   0
#define TRUE    1
#define OFF     0
#define ON      1
// ***************************** Variables ********************************** //
//unsigned char i;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
    {
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1 for calibration
                            //bit-0 should be written as 0
    TRISGPIO = 0b00001000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as ... for
                            //GP4 is set as ... for
                            //GP3 is set as ... for
                            //GP2 is set as output for LED
                            //GP1 is set as ... for
                            //GP0 is set as ... for
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b00111100;    //All pins are set as digital
    OPTION = 0b11000111;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010); 1:256 (111)
    LED = OFF;              //we should sure LED is OFF on power on reset
    }
// ***************************** MAIN loop ********************************** //
void main(void) 
    {
    SETUP_initialize();     //initialize setup
    // ******* for(;;) loop *** //
    for (;;)
        {
        LED = OFF; //Turn ON the LED
        __delay_ms(500);    //wait 500 ms
        LED = ON; //Turn OFF the LED
        __delay_ms(500);    //wait 500 ms
        }
    }
// ******************************** END ************************************* //
