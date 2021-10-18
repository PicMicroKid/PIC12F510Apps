/*
 * File:   BlueTooth_Android.c compiler version XC8 v2.10
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
#pragma config  IOSCFS= ON     //OFF =  4 MHz INTOSC speed; ON = 8 MHz INTOSC speed
// *************************** I/O Pin Definition *************************** //
//---------- Input and Output Pin(s) ----------
//#define	I_O		GPIObits.GP0	//Pin 7
#define	UART_TX		GPIObits.GP1	//Pin 6
//#define	LED 	GPIObits.GP2        //Pin 5
//#define	Input   GPIObits.GP3	//Pin 4: can be defined as input only
#define	LED         GPIObits.GP4	//Pin 3
//#define	I_O     GPIObits.GP5    //Pin 2
// *************************** Definitions ********************************** //
#define OneBitDelay  11 // 10 for 12F510 @ 8MHz, TMR0 prescaler 1:16, Baudrate 9600
#define DataBitCount    8   //no parity, no flow control
#define FALSE 0
#define TRUE 1
#define OFF 0
#define ON 1
// ***************************** Variables ********************************** //
unsigned char i, POT_value;
//unsigned char OneBitDelay;
unsigned char TempData, Digit_100 = 0, Digit_10 = 0, Digit_1 = 0; 
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
                            //GP1 is set as output for UART_TX
                            //GP0 is set as ... for
    CM1CON0 = 0b11110110;   //Analog Comparator set OFF
    ADCON0 = 0b01111001;    // AN2 (GP2) is selected as Analog Channel
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
                            //Bit2-0:PS<2:0> Timer0 Rate select as 1:8 (010);1:16 (011)
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
    /* the data send to ANDROID (via BlueTooth)  should be in ASCII code
     * Thus, the data should be converted in BCD format before sending.
     */    
    {
    while (n--)
        k /= 10;
    return k%10;
    }
// ************************ Detect BitDelay Value *************************** //
/*void Detect_OneBitDelay(void)
    {
    for (OneBitDelay = 5; OneBitDelay < 20; OneBitDelay++)
        {
        Digit_100 = GetDigit(2,OneBitDelay);
        Digit_10 = GetDigit(1,OneBitDelay);
        Digit_1 = GetDigit(0,OneBitDelay);
        UART_Transmit ('<');
        __delay_ms(15);
        UART_Transmit (Digit_100 + 48);
        __delay_ms(15);
        UART_Transmit (Digit_10 + 48);
        __delay_ms(15);
        UART_Transmit (Digit_10 + 48);
        UART_Transmit ('>');
        __delay_ms(15);        
        }
    }*/
// ************************************************************************** //
void main(void)
    {
    SETUP_initialize();     // Initialize SETUP
    InitSoftUART();
    ADCON0bits.GO = TRUE;   //Start ADC to read POT value
    POT_value = 0;          //Initial value of POT_value is 0
    LED = OFF;              //Initial state of LED is OFF
    __delay_ms(10);         //delay for AD conversion (couple of ms is enough)
    // ************************** //
    for(;;)
        {
        /*for (i=0;i=10;i++)
            Detect_OneBitDelay();*/
        if (!ADCON0bits.GO)     //if GO bit is clear, ADC is done and ready to read
            {
            POT_value = ADRES;    //ADRES is the register ADC value stored in
            ADCON0bits.GO = TRUE; //Restart ADC 
            }
        Digit_100 = GetDigit(2,POT_value);
        Digit_10 = GetDigit(1,POT_value);
        Digit_1 = GetDigit(0,POT_value);
        UART_Transmit ('<');
        __delay_ms(15);
        UART_Transmit (Digit_100 + 48);
        __delay_ms(15);
        UART_Transmit (Digit_10 + 48);
        __delay_ms(15);
        UART_Transmit (Digit_1 + 48);
        __delay_ms(15);
        UART_Transmit ('>');
        __delay_ms(250);
        }
    //return;
    }
// ******** END *********** //
