#ifndef PIC_PI_PROT_H
#define PIC_PI_PROT_H

#include "global_Config.h"
#include "CCP2_Timer.h"
#include "DS18B20.h"
#include "PWM.h"
#include "SerCom.h"




/******************************************************************************/
//readValue:
//Input: uint16_t timeWindow_par - time window for receiving the value 0-65000us
//output: value between 1-100
//if value 0 or >100 output: NACK
//code must send ACK to RPI after or before command 
/******************************************************************************/
uint8_t readValue(uint16_t);


/******************************************************************************/
//executeCommand:
//executes the Change commands with given values: 
//
//CPS   Change Pumpspeed 
//      Value 1-100, 1-20 => pump off
//
//CSS   Change Systemstate
//      Value 1-50: 
//      pump, heating, peltier off: cant be turned on except by
//      Value 51-100: 
//      pump, heating on can be changed anytime
//      Not sure if this command is actually needed
//
//CHT   Change Heating Temperature
//      send desired temperature +20.
//      temperature between -19°C (1) and +80°C (100) can be set
//
//CCT   Change Cooling Temperature
//      same as heating temperature
//  
/******************************************************************************/
void executeCommand(uint8_t, uint8_t); 

/******************************************************************************/
//SerStartEnd
//function for the IWT and IDWT part
//Inputs:
//1. uint8_t sendData: IWT or IDWT tries to send it for 3 times and save response
//2. uint16_t timeWindow: Time Window for receiving the response
// returns the serial response or NACK if nothing received 
/******************************************************************************/
uint8_t SerStartEnd(uint8_t, uint16_t);


/******************************************************************************/
//SetPumpseed:
//Input: received value between 1 and 100:
//value < 20 => Pump off
//return ACK if everything worked -> Pumpspeed in range 20-100
/******************************************************************************/
uint8_t SetPumpSpeed(uint8_t);

/******************************************************************************/
//sets PWM dutycycle for heating
//Input: value between 1 and 100
// value <= 5: heating turned off
//Output: ACK if setting of PWM Value worked, NACK if not
/******************************************************************************/
uint8_t SetHeatingPWM(uint8_t value_par);

/******************************************************************************/
//sets PWM dutycycle for cooling
//Input: value between 1 and 100
//if value <= 1: cooling turned off
//Output: ACK if setting of PWM Value worked, NACK if not
/******************************************************************************/
uint8_t SetCoolingPWM(uint8_t value_par);



/******************************************************************************/
//LinearTempControl
//Linear Temperatur Control
//Maps the Temperature difference between set and measured Temperature to a pwm value
//returns PWM Value 0-100 for Temperature Control for Heating or cooling
// => for higher difference between measured and set Temperature -> the pwm duty cycle will get bigger
// if difference bigger than set max difference -> 100% duty cycle
//Inputs:
//Ts - set Temperature
//Tm - measured Temperature
//dTmax - set max Temperature difference
//
//Method will only work properly if cooling/heating Temperature (rise) is 
//linear to PWM duty cycle
/******************************************************************************/
uint8_t LinearTempControl(int8_t Ts, int8_t Tm, uint8_t dTmax);

/******************************************************************************/
//
//HeatingTempControl
//calculates the PWM Percentage for the Heating PWM (1-100%)
//if measured temperature is bigger than set temperature, 
//the percentage will be 0 or 1
//
//Ts set (Heating) Temperature
//Tm measured (Heating) Temperature
//dTmax max Temperature difference. if difference between set and measured is  
//bigger than max -> 100% pwm
/******************************************************************************/
uint8_t HeatingTempControl(uint8_t Ts, int8_t Tm, uint8_t dTmax);

/******************************************************************************/
//
//CoolingTempControl
//calculates the PWM Percentage for the cooling PWM (1-100%)
//if measured temperature is smaller than set temperature, 
//the value will be 1
//
//Ts set (cooling) Temperature
//Tm measured (cooling) Temperature
//dTmax max Temperature difference. if difference between set and measured is  
//bigger than max -> 100%pwm
/******************************************************************************/
uint8_t CoolingTempControl(int8_t Ts, int8_t Tm, uint8_t dTmax);




#endif //PIC_PI_PROT_H