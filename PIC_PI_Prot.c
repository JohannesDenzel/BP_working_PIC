    #include "PIC_PI_Prot.h"


/******************************************************************************/
//Shared/extern Variables
//
//
//set_ht_global     set heating temperature
//                  changed in executeCommand by CHT
//                  used in HeatingTempcontrol
//
//set_ct_global     set cooling temperature
//                  changed in executeCommand by CHT
//                  used in CoolingTempcontrol
//
/******************************************************************************/

int8_t set_ht_global = 25; //set heating Temperature heating off, used in main
int8_t set_ct_global = 25; //set cooling Temperature, cooling off, used in main

// Expects to read a value between 1-100, with a timeout of 20ms. Returns the value read or NACK if it fails
uint8_t readValue(uint16_t timeWindow_par){
    CCP2_SetTimer(timeWindow_par);
    while(!(CCP2_F)){
        uint8_t byte = SerialRec();
        if(byte >= 1 && byte <= 100){
            return byte;
        }
    }
    return NACK;
}

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
//      pump, heating, peltier on can be changed anytime
//      Not sure if this command is actually needed
//
//CHT   Change Heating Temperature
//      the value sent is desired temperature + 20
//      -19°C to +80°C possible
//
//CCT   Change Cooling Temperature
//      the value sent is desired temperature + 20
//      -19°C to +80°C possible
//  
/******************************************************************************/

void executeCommand(uint8_t command, uint8_t value){
    if(command == CPS){ //Change Pumspeed

        //for debugging purposes the LED 3 is turned on if the pumpspeed is > 50
        if(value > 50){
            LATBbits.LB4 = 0;
        } else {
            LATBbits.LB4 = 1;
        }
        
        uint8_t pump_intern = SetPumpSpeed(value); //Set Pumpspeed
        //SerialSend(pump_intern); //send response: NACK if value > 100, only for debugging
        
        
    }
    else if(command == CSS){
        //Change System State ON: values can be changed, OFF: values cant be changed 
        //by changing Tris to input
        //Heating RB0 CCP4
        //Pump    RA4 CCP5
        if(value <= 50){
            TRISAbits.RA4 = 1;  //pump pin off -> Input
            TRISBbits.RB0 = 1;  //Heating pin off -> Input
            TRISCbits.RC2 = 1;  //Cooling pin off -> Input
        }else if(value > 50 && value <= 100){
            TRISAbits.RA4 = 0;  //pump pin output
            TRISBbits.RB0 = 0;  //heating pin output
            TRISCbits.RC2 = 0;  //cooling pin output
        
        }                       //else value > 100 action needed?
        
    } 
    else if(command == CHT){
        //Change Heating Temperature
        set_ht_global = value - 20; //used in heating temperature control
        //use default value if value makes no sense? 
    }
    
    else if(command == CCT){
        //change Cooling Temperature
        set_ct_global = value - 20;
        
    }
}

//function for the IWT and IDWT part
//returns the serial respone if sth received, if not NACK
uint8_t SerStartEnd(uint8_t data_par, uint16_t timeWindow_par){
    uint8_t serRec_intern = NACK;
    
    for(int i = 0; i < 3; ++i){
        SerialSend(data_par);
        // timeWindow_par us time window for receiving an answer
        CCP2_SetTimer(timeWindow_par); 
        while(!(CCP2_F)){ //CCP2_F not set
            
            serRec_intern = SerialRec();
            if(serRec_intern != NACK){ 
                //SerialSend(ACK); //only for testing, if PIC did receive it it will send ACK 
                return serRec_intern; //leave function
            }
        }
        
      }
    
    return serRec_intern; //if nothing received -> returns NACK
    
}

/******************************************************************************
//Exsample 1 SerStartEnd:

 void main(void) {
    __init_();
      
    uint8_t serRec = NACK;
    CCP4_SetTimer(50000); //50ms 
    while(42){
        LATBbits.LB2 ^= 1; //LED 1 toggle every loop to see if PIC is running
        
      uint8_t hs = SerStartEnd(IWT, 50000);
     
      if(hs != NACK){
        LATBbits.LB3 ^= 1; //LED2 toggle everytime ACK is received 
      }  
     
    }
    return;
}
 
*******************************************************************************/

/******************************************************************************
//Exsample 2 SerStartEnd:

void main(void) {
    __init_();
      
    uint8_t serRec = NACK;
    CCP4_SetTimer(50000); //10ms 
    while(42){
        LATBbits.LB2 ^= 1; //LED 1 toggle every loop to see if PIC is running
        
      uint8_t hs = SerStartEnd(IWT, 50000);
    
      //if ACK received stop main loop (while(42)) until second ACK received
      if(hs == ACK){
         LATBbits.LB3 ^= 1; //LED2 toggle everytime ACK is received
         while(1){
             uint8_t rec = SerialRec();
             
             if(rec == ACK){
                 break;
             }
         }
      }
   
        
    }
    

    return;
}
 
*******************************************************************************/



/******************************************************************************/
//Sets Pumpspeed
//Input: received value between 0 and 100:
//value < 20 -> Pump off
//return ACK if everything worked -> Pumpspeed in range 0-100
/******************************************************************************/
uint8_t SetPumpSpeed(uint8_t value_par){
    if(value_par < 20){
        DUTY_CYC_P = 0; //Turn Pump off
        return ACK;
    }else if(value_par <= 100){
        //uint8_t cyc_intern = (uint8_t) ((uint16_t) ((value_par * PR_P)/100));
        DUTY_CYC_P = (uint8_t) (value_par/100.0f * PR_P);
        return ACK;
    }else{
        return NACK;
    }
    
}

/******************************************************************************/
//sets PWM duty cycle for heating
//Input: value between 1 and 100
// value <= 5: heating turned off
//Output: ACK if setting of PWM Value worked, NACK if not
/******************************************************************************/
uint8_t SetHeatingPWM(uint8_t value_par){
    if(value_par <= 5){
        DUTY_CYC_H = 0; //Turn heating off
        LATBbits.LB3 = 1; //turn off LED2, for debugging purposes
        return ACK;
    }else if(value_par <= 100){
        //uint8_t cyc_intern = (uint8_t) ((uint16_t) ((value_par * PR_P)/100));
        DUTY_CYC_H = (uint8_t) (value_par/100.0f * PR_H); //map value to duty cycle 
        LATBbits.LB3 = 0; //turn on LED2, for debugging purposes
        return ACK;
    }else{
        return NACK;
    }
}

/******************************************************************************/
//sets PWM dutycycle for cooling
//Input: value between 1 and 100
//if value == 1: cooling turned off
//Output: ACK if setting of PWM Value worked, NACK if not
/******************************************************************************/
uint8_t SetCoolingPWM(uint8_t value_par){
    if(value_par <= 1){
        DUTY_CYC_C = 0; //Turn cooling off
        return ACK;
        LATBbits.LB4 = 1; //turn off led3, for debugging purposes
    }else if(value_par <= 100){
        //uint8_t cyc_intern = (uint8_t) ((uint16_t) ((value_par * PR_P)/100));
        DUTY_CYC_C = (uint8_t) (value_par/100.0f * DUTY_CYC_C_MAX); //map value to duty cycle 
        LATBbits.LB4 = 0; //turn on led3, for debugging purposes
        return ACK;
    }else{
        return NACK;
    }
}

/******************************************************************************/
//LinearTempControl, not used!!
//Linear Temperatur Control
//Maps the Temperature difference between set and measured Temperature to a pwm value
//returns PWM Value 0-100 for Temperature Control for Heating or cooling
//Inputs:
//Ts - set Temperature
//Tm - measured Temperature
//dTmax - max Temperature difference
//
//Method will only work properly if cooling/heating Temperature (rise) is 
//linear to PWM duty cycle
/******************************************************************************/
/*
uint8_t LinearTempControl(int8_t Ts, int8_t Tm, uint8_t dTmax){
    uint8_t pwm_intern = 100;
    uint8_t Ts_intern = 0;
    uint8_t Tm_intern = 0;
    uint8_t dT_intern = 0;
    
    //calculate absolutes
    if(Ts < 0){ //negative
        Ts_intern = -1 * Ts; //set Heating Temperature. Set in execute command if CHT
    }else{ //positive
        Ts_intern = Ts;
    }
    
    if(Tm < 0){ //negative
        Tm_intern = -1 * Tm; 
    }else{ //positive
        Tm_intern = Tm;
    }
    
    //|dT|
    if(Ts_intern > Tm_intern){
        dT_intern = Ts_intern - Tm_intern;
    }else{ //|Tm| >= |Ts|
        dT_intern = Tm_intern - Ts_intern; 
    }
    
    if(dT_intern >= dTmax){
        pwm_intern = 100; //max
    }else{ //0<= dT <= dTmax
        pwm_intern = (uint8_t) (100.0f * dT_intern)/dTmax;
        
    }
    
    
    
    
    return pwm_intern;
}

*/

/******************************************************************************/
//
//HeatingTempControl
//calculates the PWM Percentage for the Heating PWM (1-100%)
//if measured temperature is bigger than set temperature, 
//the percentage will be 1 (smallest value of the protokol))
//
//Ts set (Heating) Temperature
//Tm measured (Heating) Temperature
//dTmax max Temperature difference. if difference between set and measured is  
//bigger than max -> 100%pwm
/******************************************************************************/
uint8_t HeatingTempControl(uint8_t Ts, int8_t Tm, uint8_t dTmax){
    uint8_t pwm_intern = 0;
    uint16_t Ts_intern = 0;
    uint16_t Tm_intern = 0;
    uint16_t dT_intern = 0;
    
    //check if Ts, Tm are in a range that makes sense?
    //sould be done before calling function
    
    if(Tm < Ts){ //heating on, regardles of tm negative
        dT_intern = Ts - Tm; //allways positive because Ts shouldnt be < 0
        
    }else{
       pwm_intern = 1; //heating off
       return pwm_intern;
    }
    
    if(dT_intern >= dTmax){
        pwm_intern = 100; //full power
    }else{ //dT < max
        
        pwm_intern = (uint8_t) ((uint16_t ) (100 * dT_intern)/dTmax); //could actually also become 0
        //so if it shouldnt become 0 that needs to be checked!
    }
    
    return pwm_intern;
}


/******************************************************************************/
//
//CoolingTempControl
//calculates the PWM Percentage for the cooling PWM (1-100%)
//if measured temperature is smaller than set temperature, 
//the percentage will be 1
//
//Ts set (cooling) Temperature
//Tm measured (cooling) Temperature
//dTmax max Temperature difference. if difference between set and measured is  
//bigger than max -> 100%pwm
/******************************************************************************/
uint8_t CoolingTempControl(int8_t Ts, int8_t Tm, uint8_t dTmax){
    uint8_t pwm_intern = 0;
    uint8_t Ts_intern = 0;
    uint8_t Tm_intern = 0;
    int8_t dT_intern = 0;
    
    //check if Ts, Tm are in a range that makes sense?
    //sould be done before calling function
    
    if(Ts < Tm){ //cooling on
        dT_intern = Tm - Ts; //can be negative if both numbers are negative
        if(dT_intern < 0){
            dT_intern = -1 * dT_intern; //change to positive
        }
        
    }else{
       pwm_intern = 1; //cooling off
       return pwm_intern;
    }
    
    if(dT_intern >= dTmax){
        pwm_intern = DUTY_CYC_C_MAX; //full power 01.07.20 22:30
    }else{ //dT < max
        pwm_intern = (uint8_t) ((uint16_t )(DUTY_CYC_C_MAX * dT_intern)/dTmax); // 01.07.20 22:30 DUTY_CYC_C_MAX instead of 40
    }
    
    return pwm_intern;
}



