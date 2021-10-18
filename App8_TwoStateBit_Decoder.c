/*
 * File:   TwoBit_Decoder.c
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
#pragma config  IOSCFS= OFF     //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
#define	LED      GPIObits.GP0       // Pin No:7
//#define          I_O GPIObits.GP1   // Pin No:6
//#define          I_O GPIObits.GP2	  // Pin No:5
//#define          I_O GPIObits.GP3   // Pin No:4
//#define          I_O GPIObits.GP4	  // Pin No:3; Pin No:3; Trig Pin
#define	RF_Input GPIObits.GP5       // Pin No:2; Echo Pin of the MicroKid micro board
// *************************** Definitions ********************************** //
#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0
//These values for 8MHz and Prescale rate 1:32
#define TimeHIGH_UP     65
#define TimeHIGH_DOWN   55

#define TimeLOW_UP      25
#define TimeLOW_DOWN    15

#define AddressByte   251
#define DataByte      37
#define Command       213
// ****************************************************************************
typedef struct {
 unsigned B0:1; 
 unsigned B1:1;
 unsigned B2:1;
 unsigned B3:1;
 unsigned B4:1; 
 unsigned B5:1;
 unsigned B6:1;
 unsigned B7:1;
}DURUM;
//*****************************************************************************
DURUM BIT_DURUM;
#define LEDstate         BIT_DURUM.B0    //for LED state
#define RF_DATA_RECIEVED BIT_DURUM.B2
// ***************************** Variables ********************************* //
unsigned char RF_data[3]; //RF_data[0]-->AddressByte; RF_data[1] --> DataByte_1; RF_data[2] -->DataByte_2
unsigned char i, i_RFdata, ReceiveData, GetTime, Counter;

// ************************** Initializing Setup **************************** //
void SETUP_Initialize(void)                                                
	{
    OSCCAL = 0b01111100;    //Max frequency is set (0b01111110); b-7:1 for calibration
                            //bit-0 should be written as 0; 0b01111100 is OK to program ZD-MF 04
    TRISGPIO = 0b00110000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as input for RF_input
                            //GP4 is set as input for ...
                            //GP3 is set as ...
                            //GP2 is set as ...
                            //GP1 is set as ...
                            //GP0 is set as output for LED
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b00111100;    // All pins are set as digital; ADC:OFF
    OPTION = 0b11000011;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4
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
    RF_DATA_RECIEVED = FALSE;
    RF_data [0] = 0;
    RF_data [1] = 0;
    RF_data [2] = 0;
    Counter = 0;
    LED = OFF;
    LEDstate = FALSE;
    }
// ************************************************************************** //
// ************************** Capturing RF DATA bit stream ****************** //
void CAPTURE_RF_BitStream(void)
    {
    for(;;)
        {
        // ******************* if bit is '1' **********************
        if (GetTime > TimeHIGH_DOWN && GetTime < TimeHIGH_UP)   //check the pulse time is between the upper and lower limit
            {
            while (!RF_Input);
            GetTime = TMR0;
            TMR0 = 0;      //Reset the Timer0
            if (GetTime > TimeLOW_DOWN && GetTime < TimeLOW_UP)
                {
                ReceiveData = ReceiveData << 1;
                ReceiveData = ReceiveData | 0b00000001;
                i_RFdata ++;
                }
            else
                return;
            }
        // ****************** if bit is '0' **********************
        else if (GetTime > TimeLOW_DOWN && GetTime < TimeLOW_UP)            
            {
            while (!RF_Input);
            GetTime = TMR0;
            TMR0 = 0;      //Reset the Timer0
            if (GetTime > TimeHIGH_DOWN && GetTime < TimeHIGH_UP)
                {
                ReceiveData = ReceiveData << 1;
                ReceiveData = ReceiveData & 0b11111110;
                i_RFdata ++;
                }
            else
                return;
            }
        // ************* if bit is undefined *************************
        else
            return;
        // ************* Check RF bit number *****************
        if (i_RFdata == 8)
            {
            RF_data[0] = ReceiveData;  //store first 8 bit (0-7)   ;AddressByte
            }
        else if (i_RFdata == 16)
            {
            RF_data[1] = ReceiveData;  //store second 8 bit (8-15) ;DataByte_1
            }
        else if (i_RFdata == 24)
            {
            RF_data[2] = ReceiveData;  //store third 8 bit (16-23) ;DataByte_2
            RF_DATA_RECIEVED = TRUE;
            return;
            }
        // ********************************************
        while (RF_Input); //wait till RF_Input is LOW
        GetTime = TMR0;
        TMR0 = 0;      //Reset the Timer0
        }
    }
// ************************************************************************** //
// ***************************** Check Sycn Pulse *************************** //
void CheckSyncPulse(void)
    {
    while (RF_Input); //wait till RF_Input is LOW
    TMR0 = 0;      //Reset the Timer0
    while (!RF_Input) //wait till RF_Input is HIGH
        {
        if (TMR0 > 210) //upper limit of the sync time
            {
            TMR0 = 0;
            Counter++; //count for Sync pulse length
            }
        }
    TMR0 = 0;      //Reset the Timer0
    while (RF_Input);
    GetTime = TMR0;
    TMR0 = 0;
    if (Counter > 2) //check Sync pulse length
        {
        ReceiveData = 0;
        i_RFdata = 0;
        CAPTURE_RF_BitStream(); //Sync pulse captured, get the BitStream
        Counter = 0;
        }
    else Counter = 0;    
    }
// ************************************************************************** //
// ********************************* MAIN Loop ****************************** //
void main(void)
    {
    SETUP_Initialize();     //initialize setup
    for(i=0;i<6;i++)
        __delay_ms(250);    //delay at power-up
	for(;;)
        {
        // *********************** Check Sync Pulse ***********************
        if (RF_Input)   //if RF input is HIGH
            CheckSyncPulse();   //then check the sync pulse
        // *********************** Verify RF Data ***********************
        if (RF_DATA_RECIEVED)
            {
            if ( (RF_data [0] == AddressByte) && (RF_data [1] == DataByte) && (RF_data [2] == Command))
                {
                if (LEDstate)   //if LED already ON...
                    {
                    LED = OFF;  //Turn OFF the LED...
                    LEDstate = FALSE;
                    }
                else
                    {
                    LED = ON;   //else turn ON the LED
                    LEDstate = TRUE;
                    }
                __delay_ms(1000);
                RF_DATA_RECIEVED = FALSE;
                }
            }
        } // *** END for(;;) **** //
    }// ***** END main() ****** //
// ************************************************************************** //