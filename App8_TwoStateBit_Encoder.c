/*
 * File:   TwoBit_Encode.c
 * Author: MicroKid
 * Compiler: Xc8 V:2.10
 * Revision History: Created on Dec 23 2019 then revised for 12F510
 * Description:The protocol is based on sending data format with TwoStateBit
 * method and encode data to the raw data format and send 24 (bit) pulses. 16 bit
 * is address (8 bit sender address 8 bit receiver address), other 8 bit is data
 * (command, value, etc.)
 *  ____
 * |    |_ bit '1': The rate of the HIGH:LOW is 4:1
 *  _
 * | |____ bit '0': The rate of the HIGH:LOW is 1:4
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
#pragma config  IOSCFS= OFF      //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
#define RF_output   GPIObits.GP2
#define BUTON1       GPIObits.GP5
#define BUTON2       GPIObits.GP4

#define RF_LED      GPIObits.GP0    //Pin 7
//#define BUTON       GPIObits.GP1    //Pin 6
//#define I_O   GPIObits.GP2    //Pin 5
//#define I_O   GPIObits.GP3    //Pin 4:can be defined as input only
//#define RF_output   GPIObits.GP4    //Pin 3, Trig Pin on the MicroKid MicroBoard
//#define I_O   GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define FALSE   0
#define TRUE    1
#define ON      1
#define OFF     0
#define HIGH    1
#define LOW     0

// ***************************** Variables ********************************** //
unsigned const char SyncTime = 208, TimeHIGH = 60, TimeLOW = 20;
unsigned char RF_data [3] = {251, 37, 213}; //These values randomly selected. You may change.
unsigned char SendData;
unsigned char i,j,k;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
    {
    OSCCAL = 0b01111110;    //Max frequency is set (0b01111110); b-7:1 for calibration
                            //bit-0 should be written as 0; 0b01111100 is OK to program ZD-MF 04
    // ************** Laod OSC Fabriq calibration ****************
    TRISGPIO = 0b00111000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as ... for
                            //GP4 is set as output for RF_output
                            //GP3 is set as ... for
                            //GP2 is set as ... for
                            //GP1 is set as input for BUTON
                            //GP0 is set as output for RF_LED
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b00111100;    //All pins are set as digital
    OPTION = 0b11000011;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as;
                                //1:2 (000)
                                //1:4 (001)
                                //1:8 (010)
                                //1:16 (011); selected
                                //1:32 (100)
                                //1:64 (101)
                                //1:128 (110)
                                //1:256 (111)
    RF_LED = OFF;
    RF_output = LOW;
    }
// **************************** Send RF Data ******************************** //
void sendRFdata(void)
    {
    // ***********************
    RF_output = HIGH; //Start pulse
    RF_LED = ON;    
    TMR0 = 0;   //Reset Timer
    // ***********************
    for (k=0;k<3;k++)
        {
        SendData = RF_data[k];
        for (i=0;i<8;i++)
            {
            if (SendData & 0b10000000)  // Send MSB first; if bit is '1'
                {
                while (TMR0 < TimeHIGH);
                RF_output = LOW;
                RF_LED = OFF;
                TMR0 = 0;
                while (TMR0 < TimeLOW);
                RF_output = HIGH;
                RF_LED = ON;
                TMR0 = 0;
                }
            else    //if bit is '0'
                {
                while (TMR0 < TimeLOW);
                RF_output = LOW;
                RF_LED = OFF;
                TMR0 = 0;
                while (TMR0 < TimeHIGH);
                RF_output = HIGH;
                RF_LED = ON;
                TMR0 = 0;
                }
            SendData <<= 1; //Shift data to send the next bit
            }
        }
    while (TMR0 < TimeLOW);    //Sync pulse HIGH
    RF_output = LOW;
    RF_LED = OFF;
    TMR0 = 0;
    while (TMR0 < SyncTime);    //Sync ...
    TMR0 = 0;
    while (TMR0 < SyncTime);    //pulse ...
    TMR0 = 0;    
    while (TMR0 < SyncTime);    //LOW ...
    TMR0 = 0;  
    while (TMR0<TimeLOW);       //...
    }
/*void sendRFdata(void)
    {
    // ***********************
    RF_output = HIGH; //Start pulse
    RF_LED = ON;    
    TMR0 = 0;   //Reset Timer
    // ***********************
    for (k=0;k<3;k++)
        {
        SendData = RF_data[k];
        for (i=0;i<8;i++)
            {
            if (SendData & 0b10000000)  // Send MSB first; if bit is '1'
                {
                while (TMR0 < TimeHIGH);
                RF_output = LOW;
                RF_LED = OFF;
                TMR0 = 0;
                while (TMR0 < TimeLOW);
                RF_output = HIGH;
                RF_LED = ON;
                TMR0 = 0;
                }
            else    //if bit is '0'
                {
                while (TMR0 < TimeLOW);
                RF_output = LOW;
                RF_LED = OFF;
                TMR0 = 0;
                while (TMR0 < TimeHIGH);
                RF_output = HIGH;
                RF_LED = ON;
                TMR0 = 0;
                }
            SendData <<= 1; //Shift data to send the next bit
            }
        }
    while (TMR0 < TimeLOW);    //Sync pulse HIGH
    RF_output = LOW;
    RF_LED = OFF;
    TMR0 = 0;
    while (TMR0 < SyncTime);    //Sync ...
    TMR0 = 0;
    while (TMR0 < SyncTime);    //pulse ...
    TMR0 = 0;    
    while (TMR0 < SyncTime);    //LOW ...
    TMR0 = 0;  
    while (TMR0<TimeLOW);       //...
}*/
// ***************************** MAIN loop ********************************** //
void main(void) 
    {
    SETUP_initialize();     //initialize setup
    for (j=0;j<8;j++)
        __delay_ms(250);    //delay at power-up
    // ******* for(;;) loop *** //
    for (;;) 
        {
        if (!BUTON1) //if BUTON is pushed
            {
            RF_data [2] = 15;
            for(j=0;j<4;j++)    //bit streams
                sendRFdata();
            }
        if (!BUTON2) //if BUTON is pushed
            {
            RF_data [2] = 20;
            for(j=0;j<4;j++)    //bit streams
                sendRFdata();
            }
        __delay_ms(135);  //after my tests
        }// END for(;;)
    }// END main()
// ******************************** END ************************************* //
