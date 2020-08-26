#ifndef DS18B20_H
#define DS18B20_H


#include "global_Config.h"
//Quelle: https://pic-projekte.de/blog/ds18s20/
//Da der Code von Pic Projekte nicht funktioniert hat 
//(Temperaturwerte waren scheinbar zufällig)
//wurden einige Funktionen aus der Arduino Library als Referenz genommen und 
//sozusagen so umgeschrieben, dass sie mit dem PIC funktionieren
//https://www.pjrc.com/teensy/td_libs_OneWire.html






//Serial Receive needs to be turned off!

//Prototypes

/******************************************************************************/
//ow_mri
//OneWire Master Reset impuls
//returs low if at least 1 slave available
/******************************************************************************/
uint8_t ow_mri (void);          //1. Master Reset Impuls 

/******************************************************************************/
//ow_rd_bit
//OneWire Read Bit
//returns bit
/******************************************************************************/
uint8_t ow_rd_bit (void);       //read 1 Bit

/******************************************************************************/
//ow_rd_byte
//OneWire read byte
//returs one byte
/******************************************************************************/
uint8_t ow_rd_byte (void);      //read 1 Byte

/******************************************************************************/
//ow_wr_bit
//OneWire write bit
//writes one bit to the bus 
//if val != 0 it writes 1 
//if val == 0 it writes 0
/******************************************************************************/
void ow_wr_bit (uint8_t val);   //write 1 Bit

/******************************************************************************/
//ow_wr_byte
//OneWire write byte
//writes the byte val to the bus
/******************************************************************************/
void ow_wr_byte (uint8_t val);  

/******************************************************************************/
//ConvTempInt
//Input Scratchpad Data
//Output Temperature -127°C to +127°C
//Outputs 0 for all sensors if all sensors are disconnected, have no power
//Outputs -127°C if no temperature is read -> at least 1 sensor is connected
/******************************************************************************/
int8_t ConvTempInt(uint8_t [9]); //Convert scratchpad data to temperature

/******************************************************************************/
//Select sensor
//Input: 64 bit Sensor Adress
//after Selecting the Sensor, the next byte written to the bus has to be the command adressed
//to the selected Sensor
//Example:
//SelectSensor(Sensor1);
//ow_wr_byte(Convert_T); //Conver_T starts Temperature conversion
/******************************************************************************/
void SelectSensor(uint8_t [8]);  //input 64 bit adress

/******************************************************************************/
//ReadTemperature
//Sends the ReadScratch command, reads the data and converts it to a Temperature
//by calling ConvTempInt
//Example if more than 1 sensor is on the bus:
//ow_mri();                                 //1. Master Reset Impuls
//ow_wr_byte(Skip_ROM);                     //2. send skip rom 
//ow_wr_byte(Convert_T);                    //3. send conv_T to all sensors
//__Delay_ms(750);                          //4. wait for minimum of 750ms
//ow_mri();                                 //5. mri
//SelectSensor(Sensor1);                    //6. select sensor
//int8_t temperature = ReadTemperature();   //7. ReadTemperature
/******************************************************************************/
int8_t ReadTemperature(void);    



#endif //DS18B20_H
