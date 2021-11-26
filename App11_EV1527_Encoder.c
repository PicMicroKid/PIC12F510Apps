/*
 * File:   EV1527_Encoder.c
 * Author: MicroKid
 * Compiler: Xc8 V:2.31
 * Revision History: Created on Dec 23 2019 then Revised on August 2021 for EV1527 protocol
 * Description:The protocol is based on sending data format with TwoStateBit (based on EV1527)
 * method and encode data to the raw data format and send 24 (bit) pulses. 16 bit
 * is address (8 bit sender address 8 bit receiver address), other 8 bit is data
 * (command, value, etc.)
 * 
 * RX480E al?c? icin ENKODER (verici) yazilimi. Protokol EV1527 ile uyumlu. 
 * HIGH:LOW süreleri teknik dokumandaki süreler dikkate al?narak belirlendi.
 * Preamble haric 24 bit veri gönderilmekte. 24 bit veriden 20 bit adres, kalan 
 * 4 bit ise kanal bilgisidir. 24 bit veri icin 3 adet 8 bitlik veri paketi uzerinden
 * adres ve kanal bilgisi belirlendi.
 * 
 *  _
 * | |_____________  Preamble icin HIGH:LOW orani = 1:31 ; Preamble: The rate of the HIGH:LOW is 1:31 
 *  ____
 * |    |_  logic '1' icin HIGH:LOW orani = 3:1 ; bit '1': The rate of the HIGH:LOW is 3:1 
 *  _
 * | |____  logic '0' icin HIGH:LOW orani = 1:3 ; bit '0': The rate of the HIGH:LOW is 1:3 
 */
#include <xc.h>
// *** Microcontroller MIPs (FCY) *** //
#define _XTAL_FREQ 4000000       // * target device system clock frequency: Max Frequency is 8 MHz for 12F510 * //
                                // * required for __delay_ms, __delay_us macros * //
//CONFIG bit AYARI // CONFIG bits that are part-specific for the PIC 12F510
#pragma config  OSC   = IntRC   //Dahili OSC secildi. //Oscillator Selection bits (internal RC oscillator)
#pragma config  WDT   = OFF     //Watchog Timer Enable KAPALI // Watchog Timer Enable bit (WDT disabled)
#pragma config  CP    = OFF     //Kod koruma KAPALI //Code protection bit (CP is ON)
#pragma config  MCLRE = OFF     //MCLRE pin GP3 giris pini olarak ayarland? //GP3/MCLRE Pin Function Select bit (GP3/MCLRE pin function is GP3)
#pragma config  IOSCFS= OFF     //Dahili OSC hz?z 4 MHz olarak secildi // OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
#define BUTON1      GPIObits.GP2 //Pin 5
#define BUTON2      GPIObits.GP3 //Pin 4
#define BUTON3      GPIObits.GP4 //Pin 3
#define BUTON4      GPIObits.GP5 //Pin 2
#define RF_output   GPIObits.GP1 //Pin 6
// *************************** Definitions ********************************** //
#define FALSE   0
#define TRUE    1
#define ON      1
#define OFF     0
#define HIGH    1
#define LOW     0

#define Adress0 255
#define Adress1 32
/* Ch_x seklinde tanimlanan veriside ilk 4 bit (bit 3-0) adres bilgisi,
 * son 4 bit (bit 7-4) ise KANAL bilgisidir.
 * 
 *  7  6  5  4  3  2  1  0
 *  __ __ __ __ __ __ __ __
 *  A  B  C  D  x  x  x  x 
 * 
 * A..D bitlerinden '1' olanlar RX480 cikisinda ilgili KANALI aktif edecektir. 
 * 'x' degleri de yine ADRES bilgisine karsilik gelmektedir.
 */
#define CH_A 0b10000001
#define CH_B 0b01000001
#define CH_C 0b00100001
#define CH_D 0b00010001
#define CH_AC 0b10100001
#define CH_CA 0b01010001
#define CH_BD 0b10010001
#define CH_DB 0b01100001
// ***************************** Variables ********************************** //
unsigned const char Time3x = 45, Time1x = 15; //@ 4MHz, 1:16 prescale
//unsigned const char SyncTime = 200, TimeHIGH = 60, TimeLOW = 20; //@ 4MHz, 1:16 prescale
unsigned char RF_data [3] = {Adress0, Adress1, CH_A}; //These values randomly selected. You may change.
unsigned char SendData;
unsigned char i,j,k;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
    {
    OSCCAL = 0b01111100;    //Max frequency is set (0b01111110); b-7:1 for calibration
                            //bit-0 should be written as 0
    TRISGPIO = 0b00111100;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is input for BUTON4
                            //GP4 is input for BUTON3
                            //GP3 is input for BUTON2
                            //GP2 is input for BUTON1
                            //GP1 is output for RF_output
                            //GP0 is ... for
    CM1CON0 = 0b11110110;   //Analog Comparator OFF
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
    RF_output = LOW;
    }
// **************************** Send RF data EV1527 ************************* //
void SEND_RFdata(void)
    {
    //
    //  __
    // |  |_____________  Preamble: The rate of the HIGH:LOW is 1:31
    //  ______
    // |      |__  bit '1': The rate of the HIGH:LOW is 3:1
    //  __
    // |  |______  bit '0': The rate of the HIGH:LOW is 1:3
    //
    // *** Protocol for EV1527: Preamble + C0...C199 + D0+D1+D2+D3
    // ************ Preamble PULSE ***************
    RF_output = HIGH;
    TMR0 = 0;
    while (TMR0 < Time1x);    //Sync pulse HIGH
    RF_output = LOW;
    for (i=0;i<31;i++)
        {
        TMR0 = 0;
        while (TMR0 < Time1x);    //Sync ...
        }
    // *******************************************
    RF_output = HIGH; //Start pulse   
    TMR0 = 0;   //Reset Timer
    // ***********************
    for (k=0;k<3;k++)
        {
        SendData = RF_data[k];
        for (i=0;i<8;i++)
            {
            if (SendData & 0b00000001)  // Send LSB first; if bit is '1'
                {
                while (TMR0 < Time3x);
                RF_output = LOW;
                TMR0 = 0;
                while (TMR0 < Time1x);
                RF_output = HIGH;
                TMR0 = 0;
                }
            else    //if bit is '0'
                {
                while (TMR0 < Time1x);
                RF_output = LOW;
                TMR0 = 0;
                while (TMR0 < Time3x);
                RF_output = HIGH;
                TMR0 = 0;
                }
            SendData >>= 1; //Shift data to send the next bit
            }
        }
        RF_output = LOW;
    }
// ***************************** MAIN loop ********************************** //
void main(void) 
    {
    SETUP_initialize();     //initialize setup
    for (j=0;j<8;j++)
        __delay_ms(250);    //delay at power-up
    // ******* for(;;) loop *** //
    for (;;) 
        {
        if (!BUTON1) //if BUTON1 is pushed
            {
            RF_data [2] = CH_AC;
            for(j=0;j<3;j++)    //bit streams
                SEND_RFdata();
            }
        else if (!BUTON2) //if BUTON2 is pushed
            {
            RF_data [2] = CH_CA;
            for(j=0;j<3;j++)    //bit streams
                SEND_RFdata();
            }
        else if (!BUTON3) //if BUTON3 is pushed
            {
            RF_data [2] = CH_BD;
            for(j=0;j<3;j++)    //bit streams
                SEND_RFdata();
            }
        else if (!BUTON4) //if BUTON4 is pushed
            {
            RF_data [2] = CH_DB;
            for(j=0;j<3;j++)    //bit streams
                SEND_RFdata();
            }
        __delay_ms(55);  //after my tests
        }// END for(;;)
    }// END main()
// ******************************** END ************************************* //
