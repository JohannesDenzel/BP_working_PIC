#ifndef PWM_H
#define PWM_H


#include "global_Config.h"


//Configures and initializes the PWM Modules and corseponding pins
//to be called in the init function in the main file

void Setup_Pump_PWM(void);
void Setup_Heating_PWM(void);
void Setup_Cooling_PWM(void);



#endif //PWM_H
