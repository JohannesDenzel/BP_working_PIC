#ifndef SERCOM_H
#define SERCOM_H

#include "global_Config.h"


//initializes everything for serial communication, but the EUSART Module stays turned off
void SetupSerial(void);

//Sending 1 byte via serial
void SerialSend(uint8_t data_par);

//receiving 1 byte
uint8_t SerialRec(void);


#endif //SERCOM_H



