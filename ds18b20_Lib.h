/*******************************************************
 * Libray for DS18B20 and DS1821
 * Author: MicroKid
 * Comments: Revised for 12F510 on XC8 v2.10
 * Revision history: Created on on 2006; 1st revision on July 12, 2018; 
 * 2nd revision on April 10, 2020 
*******************************************************/

unsigned char DS_read_byte(void);
void DS_write_byte(unsigned char data);
unsigned char DS_present(void);
//void DS_SETUP(void);
//unsigned char DS_Read_Temp(void);