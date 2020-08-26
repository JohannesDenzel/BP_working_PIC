#ifndef CCP2_TIMER_H
#define CCP2_TIMER_H


#define CCP2_F PIR2bits.CCP2IF


#include "global_Config.h"



void CCP2_TimerSetup(void);
void CCP2_SetTimer(uint16_t);
void CCP2_Reset(void); //use only if its sure that all flags need to be reset
//if not and it is called in a loop before set timer -> the code might not work





#endif //CCP2_TIMER_H