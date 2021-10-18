/*
 * File:   App9_softwareSPI.c
 * Author: MicroKid
 * Compiler: XC8 v2.31
 * Revision History: Feb 21, 2021
 * Description: Using software based SPI to read temperature value from MAX31588
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
#define UART_TX		GPIObits.GP1	//Pin 6
//#define I_O   GPIObits.GP2    //Pin 5
#define SPI_MISO    GPIObits.GP3    //Pin 4:can be defined as input only
#define SPI_CS      GPIObits.GP4    //Pin 3: T pin on the MicroBoard
#define SPI_clock   GPIObits.GP5    //Pin 2: E pin on the MicroBoard
// *************************** Definitions ********************************** //
#define OneBitDelay  10 // @ 8MHz, TMR0 prescaler 1:16, Baudrate 9600
#define DataBitCount 8  //no parity, no flow control
#define delay_time 5    //
#define FALSE   0
#define TRUE    1
#define OFF     0
#define ON      1
// ************************************************************************** //
typedef struct {
    unsigned B0:1;
    unsigned B1:1;
    unsigned B2:1;
    unsigned B3:1;
    unsigned B4:1;
    unsigned B5:1;
    unsigned B6:1;
    unsigned B7:1;
}_status_;
_status_ BITs_STATUS;
#define TC_error BITs_STATUS.B0 //for no-connection error
#define TC_sign BITs_STATUS.B1 //for temperature sign
// ***************************** Variables ********************************** //
unsigned char i, TempData, Digit_100, Digit_10, Digit_1, Temperature, SPI_data[4];
// ******************************** SETUP *********************************** //
void SETUP_initialize(void)
	{
    OSCCAL = 0b11111110;    //Max frequency is set; b-7:1 for calibration bit-0 should be written as 0
    TRISGPIO = 0b00001000;  //'1': for inputs; '0': for outputs
                            //GP7 is NA
                            //GP6 is NA
                            //GP5 is set as output for CLK
                            //GP4 is set as output for CS
                            //GP3 is set as input for MISO
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
    SPI_CS = 1;     //disable the MAX31855 IC
    SPI_clock = 0;  //Idle state for clock is LOW
    TC_error = 0;
    TC_sign = 0;
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
     * TX pin is usually HIGH. A HIGH to LOW bit is the starting bit and a
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
// **************************** Read Temperature **************************** //
void read_TEMPERATURE(void)
    {
    unsigned char index, data;
    #define delay_time 5
    SPI_CS = 0; //Enable IC
    __delay_us(1);  //After ~100 ns the first bit (D31) is ready @ SO (MISO) pin
    // ********************************
    for(index = 0;index < 31; ++index)
        {
        if (SPI_MISO)   //check the BiT
            data = data | 0b00000001; //store bit '1'
        else
            data = data & 0b11111110; //store bit '0'
        // *********** clock cycle ****
        SPI_clock = 1;
        __delay_us(delay_time);
        SPI_clock = 0;
        __delay_us(delay_time);
        // ****************************
        if (index == 7)
            SPI_data[0] = data; //store the bits: 31-24
        else if (index == 15)
            SPI_data[1] = data; //store the bits: 23-16
        else if (index == 23)
            SPI_data[2] = data; //store the bits: 15-8
        // ****************************
        data <<= 1; //shift data for next bit
        }
    SPI_data[3] = data; //store the bits: 7-0
    SPI_CS = 1; //Disable IC
    // *********************************
    if (SPI_data[3] & 1) //Check the open-circuit bit
        TC_error = 1;   //TC is not connected
    else TC_error = 0;  //TC is connected
    if (SPI_data[0] & 128) //Check the TC sign bit
        TC_sign = 1; //Temperature is NEGATIVE
    else TC_sign = 0; //Temperature is POSITIVE
    // **********************************
    SPI_data[1] >>= 4;
    SPI_data[1] &= 0b00001111;
    SPI_data[0] &= 0b01111111;
    TempData = SPI_data[0];
    TempData <<= 4;
    TempData += SPI_data[1];
    Temperature = TempData;
    }
// **************************** Send Temperature **************************** //
void send_TEMPERATURE(void)
    {
    UART_Transmit ('*');    //Send start character
    __delay_ms(25);         //delay before sending next byte
    // ************* if TC not connected, send ERROR message ****
    if (TC_error)   //check the error bit
        {
        UART_Transmit('E');
        __delay_ms(25);
        UART_Transmit('R');
        __delay_ms(25);
        UART_Transmit('R');
        __delay_ms(25);
        UART_Transmit('O');
        __delay_ms(25);
        UART_Transmit('R');
        __delay_ms(25);
        }
    // ********* Sending temperature value by digits **********
    else
        {
        if (Digit_100 != 0) //if Digit100 not ZERO
            {
            UART_Transmit (Digit_100 + 48); //Transmit ASCII value...
            __delay_ms(25);
            UART_Transmit (Digit_10 + 48);
            __delay_ms(25);
            }
        else if (Digit_10 != 0) //if Digit100 is ZERO and Digit10 not ZERO
            {
            UART_Transmit (Digit_10 + 48);
            __delay_ms(25);
            }
        UART_Transmit (Digit_1 + 48);
        __delay_ms(25);
        }
    UART_Transmit ('*'); //send STOP character
    }
// ***************************** MAIN loop ********************************** //
void main(void) 
    {
    SETUP_initialize(); //initialize setup
    InitSoftUART();       //initialize UART
    // ******* for(;;) loop *** //
    for (;;)
        {
        // ******************* Delay ******************
        for (i=0;i<5;i++)
            __delay_ms(250);    //Delay for each read&send proccess
        // ********************************************
        read_TEMPERATURE(); //Read 16 bit temperature value from MAX31855
        // ********** BCD conversion for temperature **
        Digit_100 = GetDigit(2,Temperature);    //Get the ...
        Digit_10 = GetDigit(1,Temperature);     //digits of ...
        Digit_1 = GetDigit(0,Temperature);      //the temperature
        // **********************************************
        send_TEMPERATURE();
        }
    }
// ******************************** END ************************************* //
