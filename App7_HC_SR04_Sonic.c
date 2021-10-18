/*
 * File:   HC_SR04_Sonic.c
 * Author: Microkid
 *
 * Created on October 31, 2020, 5:39 PM
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
#pragma config  IOSCFS= ON      //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
//---------- Input and Output Pin(s) ----------
//#define	I_O_		GPIObits.GP0	//Pin 7
#define     UART_TX		GPIObits.GP1	//Pin 6
//#define	I_O 		GPIObits.GP2    //Pin 5
//#define	Input       GPIObits.GP3	//Pin 4: can be defined as input only
#define	Trig GPIObits.GP4	//Pin 3
#define	Echo GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define OneBitDelay  10 // @ 8MHz, TMR0 prescaler 1:16, Baudrate 9600
#define DataBitCount 8   //no parity, no flow control
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
// ***************************** Variables ********************************** //
unsigned char i, TempData, OutRange, Distance, Digit_100, Digit_10, Digit_1;
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
	{
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1 for calibration bit-0 should be written as 0
    TRISGPIO = 0b00101000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as input for ECHO
                            //GP4 is set as output for Trig
                            //GP3 is set as ...
                            //GP2 is set as ...
                            //GP1 is set as output for UART Tx
                            //GP0 is set as ... for
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b00111100;    // All pins are set as digital
    OPTION = 0b11000011;    //Bit7:RBWU disabled (1), Bit6:RBPU disabled (1), 
                            //Bit5:T0CS Timer0 clock source selected as Fosc/4 (0)
                            //If Bit5 is reset '0' Bit4 not important
                            //Bit3:PSA prescaler assigned to Timer0 (0)
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010); 1:16 (011); 1:256 (111)
    }
// *************************** UART Initialize ****************************** //
void InitSoftUART(void)
    {
    UART_TX = 1;
    }
// ***************************** UART Transmit ****************************** //
void UART_Transmit(unsigned char SendValue)
    {
    /* Basic Logic
     * TX pin is usually HIGH. A HIGH to LOW bit ,s the starting bit and a
     * LOW to HIGH bit is the ending bit. No parity bit. No flow control
     * BitCount is the number of bits to transmit.
     * LSB is the first during transmission.
     */
    unsigned char j;
    UART_TX = 0;
    TMR0 = 0;
    while (TMR0 < OneBitDelay);
    for (j=0;j<DataBitCount;j++)
        {
        // Set Data pin according to the DataValue
        TempData = SendValue & 1;
        if (TempData)
            UART_TX = 1;
        else 
            UART_TX = 0;
        SendValue >>= 1;
        SendValue &= 0b01111111;
        TMR0 = 0;
        while (TMR0 < OneBitDelay);
        }
    // Send STOP bit
    UART_TX = 1;
    TMR0 = 0;
    while (TMR0 < OneBitDelay);
    }
// **************************** Decimal to BCD ****************************** //
unsigned char GetDigit(unsigned char n, unsigned char k)  
    {
    /* the data send to ANDROID (via BlueTooth)  should be in ASCII code
     * Thus, the data should be converted in BCD format before sending.
     */  
    while (n--)
        k /= 10;
    return k%10;
    }
void main(void) 
    {
    SETUP_initialize();
    InitSoftUART();
    for(;;)
        {
        OutRange = 0;   //Out of Range state
        for (i=0;i<4;i++)
            __delay_ms(250);    //delay before each SONiC trigger
        Trig = 1;   //Trig activated
        __delay_us(10); //HIGH time for Trig
        Trig = 0;   //Trig deactivated
        while (!Echo);   //wait till Echo is '0'
        TMR0 = 0;   //Reset TiMER 0
        while (Echo)    //wait till Echo is '1' for response time
            {
            if (TMR0 > 250)
                {
                OutRange = 0x0F; //Out of Range
                break;
                }
            }
        if (OutRange == 0x0F)   //if Out of Range, send "OutRange"
            {
            UART_Transmit('*');
            __delay_ms(25);     //Delay between each transmission
            UART_Transmit('O');
            __delay_ms(25);
            UART_Transmit('u');
            __delay_ms(25);           
            UART_Transmit('t');
            __delay_ms(25);
            UART_Transmit('R');
            __delay_ms(25);
            UART_Transmit('a');
            __delay_ms(25);
            UART_Transmit('n');
            __delay_ms(25);
            UART_Transmit('g');
            __delay_ms(25);
            UART_Transmit('e');
            __delay_ms(25);
            UART_Transmit('*');
            }
        else 
            {
            Distance = TMR0;        //Echo response time
            Distance >>= 2;         //Little bit calibration
            Digit_100 = GetDigit(2,Distance);   //Get the...
            Digit_10 = GetDigit(1,Distance);    //digits of...
            Digit_1 = GetDigit(0,Distance);     //the Distance
            UART_Transmit ('*');
            __delay_ms(25);
            UART_Transmit (Digit_100+48);       //Transmit the ASCII value of...
            __delay_ms(25);
            UART_Transmit (Digit_10+48);        //Digits
            __delay_ms(25);
            UART_Transmit (Digit_1+48);        //...
            __delay_ms(25);
            UART_Transmit ('c');
            __delay_ms(25);
            UART_Transmit ('m');
            __delay_ms(25);
            UART_Transmit ('*');
            }
        } // *** END for(;;) **** //
    }// ***** END main() ****** //
//****************************************************************************//
