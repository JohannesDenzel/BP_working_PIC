/*
 * File:   main.c
 * Author: Megaport
 *
 * Created on 5. Juni 2020, 22:33
 */


#include "PIC_PI_Prot.h"

void EEPROM_write(uint8_t data_byte, uint8_t data_adress);
uint8_t EEPROM_read(uint8_t data_adress);

void _init_(void);
uint8_t LinearTempControl(int8_t Ts, int8_t Tm, uint8_t dTmax);

uint8_t maxTempReached = 0;

//DS18B20 Sensor adresses
uint8_t adrSensor1[8] = {0x28,0x7C,0xBF,0x79,0x97,0x7,0x3,0x3E}; //0 rings, cooling hot side, overheat control
uint8_t adrSensor2[8] = {0x28,0x7F,0x96,0x79,0x97,0x7,0x3,0x6C}; //1 ring, cooling
uint8_t adrSensor3[8] = {0x28,0x9A,0xB1,0x79,0x97,0x7,0x3,0x7};  //2 rings, heating

 struct flags_s{
                                    
        uint8_t flag0: 1;           
        
        uint8_t flag1: 1;           
        
        uint8_t flag2: 1;           
        
        uint8_t flag3: 1;
        
        uint8_t flag4: 1;
        
        uint8_t flag5: 1;
        
        uint8_t flags6_7: 2;           //free bits, bit6 - bit7
    }flags_pt_s; 
  
    //PT element calibration
    //flags only used in main
    
    //rename feld data of flags_pt_main_s for Pt elements
    //do not move to global. Its easier to understand if the defines stay here
    
#define ptcT_amb_f  flags_pt_s.flag0   // flag is set at reset or if the measured,
                                            // temperature of the cool side is bigger 
                                            // than the room temperature defined in global_config.h 

#define pt_n_av_f   flags_pt_s.flag1   // flag is set if number of temperature measurements 
                                            // for calculating a average temperature is reached.
                                            // calculate average, reset flag, reset number

#define pt_ovh_f    flags_pt_s.flag3   // flag is set if overheat temperature is reached
    
#define pt_cali_f   flags_pt_s.flag4   //flag is set after calibration is finished. 
    
#define  pt_cali_wait_f flags_pt_s.flag5 //cali wait finished

/******************************************************************************/
//Shared Variables:
//set_ht_global     set Heating Temperature. 
//                  set in executeCommand if(CHT) in PIC_PI_Prot.c
//                  used for temperatur Control after Measuring the Temperature
//
//set_ct_global     set Cooling Temperature
//                  set in executeCommand if(CCT) in PIC_PI_Prot.c
//                  used for Temperature control after Measuring the Temperature
/******************************************************************************/

//extern int8_t set_ht_global; //Set heating Temperature
//extern int8_t set_ct_global;

int8_t T1 = -127; //Temperature of Sensor 1
int8_t T2 = -127; //Temperature of sensor 2
int8_t T3 = -127; //Temperature of Sensor 3



void main(void) {
_init_();



    uint8_t pwmval_h = 1; //value between 1 and 100 for heating
    uint8_t pwmval_c = 1; //value between 1 and 100 for cooling 
      
    uint8_t serRec = NACK; 
    
    
   

uint8_t c_res = NACK;
    
    ptcT_amb_f = 0; //start value
    pt_n_av_f = 0; //start with wait
    pt_ovh_f = 0; //overheat flag 0, not overheated
    pt_cali_wait_f = 0; //this flag is set to 1 if the cali wait phase is on
    

    uint16_t n_T_meas = 0;          //number of temperature measurements
    
    pwmval_c_top.dc = PWMVAL_C_MAX_DC; //start value
    pwmval_c_bottom.dc = 0;
    pwmval_c_max.dc = PWMVAL_C_MAX_DC;//max duty cycle calculated by calibration
    
    
    
    //flags
    pwmval_c_top.flag = 1; // Kalibration starts with top. at the beginning of 
                           // the Calibration top.flag != bottom.flag
    pwmval_c_bottom.flag = 0;
    
    pwmval_c_max.flag = 0; //is set after max is set in Kalibration, reset after 1 wait period
  
    
    int16_t t_sum_0 = 0;
    //int16_t t_sum_1 = 0;
    int8_t t_av = 127; //average temperatures
    int8_t  t_av_0 = 127; 
    int8_t  t_av_1 = 127;
    float  t_k = 0; //faktor that is added to the room temperature in calibration
    //to give the PT enough time to cool down after kalibration
    uint8_t t_av_n = 0; //average counter
    
    uint16_t pt_cali_wait_start = 0; //start value of waiting time, while calibration is not possible
    



    
//for testing the wdt:
//toggle LED4 and write/read the value to/from EEPR adress 1
    uint8_t led_val = EEPROM_read(1);
    if(led_val){ //if led_val!=0
        LATBbits.LB5 = 0; //turn led on
        led_val = 0;
    }else{
        LATBbits.LB5 = 1;
        led_val = 1;
    }
    EEPROM_write(led_val, 1); //write updated led_val to EEPROM register eith adress 1
    
    //if the pic is not reset, the led state will stay the same if it is reset,
    //the LED will toggle every 4sek
    

    uint16_t loop_counter = 0;
    while(42){
        ClrWdt(); //feed watchdog, time: 4s (presc: 10).                        //added 17.08.20
        //if the timewindow for sth. is to big that might cause the PIC to restart
        //because the wdt is on.
        //so check if the executing time is still much smaller than the wdt period
       
        loop_counter++;
        if(loop_counter >= 65500){
            loop_counter = 0;
        }
        
        LATBbits.LB2 ^= 1; //LED 1 toggle every loop to see if PIC is running   LED1
        
        SER_EN = 0; //disable Serial, to prevent overrun error because bytes cant be processed
        
        //Sensor send Measurement comands
        uint8_t mri = ow_mri();          //1. Master Reset Impuls  
        if(mri == 0){ //Slave available
            ow_wr_byte(Skip_ROM);  //speak to all Sensors
            ow_wr_byte(Convert_T); //Start Temperature Conversion    
        }
      
        
        __delay_ms(1); 
        SER_EN = 1; //enable Serial
        __delay_ms(1); //delay to make sure the Enabeling is finished
        
        //Start Serial Communication by sending IWT
        uint8_t iwt_resp = SerStartEnd(IWT, 50000);                             
        if(iwt_resp == NACK){   //no response
            
#if SAFETY_ON == 1

            DUTY_CYC_H = 0;     //turn off heating
            DUTY_CYC_P = 0;     //turn off pump
            DUTY_CYC_C = 0;    //turn off cooling
          
#endif
           
            
            SerialSend(NACK);   //send nack  
            
        }else if(iwt_resp != NACK && iwt_resp != ACK){ //RPI might have sent Command already
            serRec = iwt_resp;  //save iwt_Resp in serRec
            iwt_resp = ACK;     //set iwt_Resp to ACK
        }
      
        
        for(int i = 0; i < 100; i++){ //Timewindow*settimer                      
            
            CCP2_SetTimer(10000); //reset flags and set 10ms                   
            while(!(CCP2_F)){ //10ms time window

                //serRec = SerialRec(); 
                //serRec updates at the end of the loop, so that the possib-le iwt resp can be computed
                uint8_t value = 0;
                 if(serRec == CPS || serRec == CSS || serRec == CHT || serRec == CCT){
                    //value that follows a change command, input: timewindow
                    value = readValue(20000);                                  
                    if(value == NACK){          //if no value read in Time Window it returns NACK
                        SerialSend(NACK);
                        continue;
                    }
                    if(1 <= value && value <= 100){ //if value in range 1-100   
                    executeCommand(serRec, value); //only executes valid commands
                        SerialSend(ACK);
                    // LATBbits.LB3 ^= 1; //toggle LED 2      
                    }else{                                                     
                        SerialSend(NACK); //no value or bigger than 100         
                    }
            
                } else if (serRec == IWR){     //I wanna Read: Send all system data
                    // Send all temperatures 
                    // current Pumpspeed, set heating, cooling temperature might also be needed
                    // Sugestion:
                    // IWRMT     i wanna read all measured Temperatures
                    // IWRSHT    i wanna read set heating Temperature
                    // IWRSCT    read set cooling temperature
                    // IWRP      read set Pump speed
                    
                    //Temperature Range numbers: 1 - 100
                    //valid measured range -19 to + 79�C
                    //sent 20 means 0�C sent 44 means 24 
                    
                   
                    if(-19 <= T1 && T1 < 80){
                        SerialSend(T1 + 20);  //can send value between 1 and 99
                    }else{
                        SerialSend(100); //nothing should be 80�C hot -> send 100 = to Hot, error
                        
                    }
                    
                    if(-19 <= T2 && T2 < 80){
                        SerialSend(T2 + 20); 
                    }else{
                        SerialSend(100); //nothing should be 80�C hot -> error
                    }
                    
                    if(-19 <= T3 && T3 < 80){
                        SerialSend(T3 + 20); 
                    }else{
                        SerialSend(100); //nothing should be 80�C hot -> error
                    }
                    
                    
                }else if(serRec == S_PT_PWM){ //for debugging. send pt pwm
                    SerialSend(pwmval_c);
                }
                
                serRec = SerialRec(); //update Serial Receive
        
            } //10ms time window inside for 750ms
          
        } //for 750ms
        uint8_t idwt_resp = SerStartEnd(IDWT, 50000);                          
        
        //no safety turnoff because its enough at the beginning of communication 
        
        SER_EN = 0; //Disable serial
        __delay_ms(1);
        
        //read temperature from Sensors
        mri = ow_mri();
        if(mri == 0){
            SelectSensor(adrSensor1);
            T1 = ReadTemperature();
        }
        mri = ow_mri();
        if(mri == 0){
            SelectSensor(adrSensor2);
            T2 = ReadTemperature();
        }
        mri = ow_mri();
        if(mri == 0){
            SelectSensor(adrSensor3);
            T3 = ReadTemperature();
        }
        
        
        
        /**********************************************************************/
        //Temperatur Control each Time a new temperature is (or should be) measured
        /**********************************************************************/
        //Calculate Duty Cycle Percentage
        
        int8_t measHeatingTemp = T3; //2 rings
        //int8_t measRoomTemp = T3; //2 rings
        int8_t measPtCoolTemp  = T2; //1 rings
        int8_t measPtHotTemp   = T1; //0 rings always > 0
        
        /**********************************************************************/
        // PT Calibration
        /**********************************************************************/
   
        /*
        //last Kalibration finished and no cali wait running
        if(pt_cali_f == 1 && pt_cali_wait_f == 0){
            pt_cali_wait_start = loop_counter;
            pt_cali_wait_f = 1; //cali wait running
        }
        
        if(loop_counter - pt_cali_wait_start >= PT_CALI_W && pt_cali_f == 1){
            pt_cali_f = 0; //calibration can be started again
            pt_cali_wait_f = 0; //cali wait finished
        }
        
        
        //t_k =  T_K_Faktor * t_k;
        
        uint8_t t_k_i = (uint8_t) t_k;
        if(t_k + 0.5 >= t_k_i + 1) t_k_i += 1; //aufrunden
        
        //if pt cool temperature is to warm and a new calibration can be started
        if(measPtCoolTemp >= measRoomTemp + t_k_i - PT_T_CALI && pt_cali_f == 0) ptcT_amb_f = 1; //set cooling Temperature >= ambient Temperature flag
                                                    //reset after kalibration
    
            //Calibration
        if(ptcT_amb_f == 1){
        
            n_T_meas++; //increase measurement counter 
            //reset maximum
            pwmval_c_max.dc = PWMVAL_C_MAX_DC;
            //if the pic doesnt wait, its calculating average -> wait != average
        
    
        
            if(n_T_meas <= PT_N_AV && pt_n_av_f == 1){
            t_sum_0 += measPtCoolTemp;
            }else if(pt_n_av_f == 1){
                n_T_meas = 0; //reset measurement counter
                pt_n_av_f = 0; //reset average calc flag
            
                t_av = t_sum_0/PT_N_AV;  
               
                t_av_n++; //inrement average counter
            
                t_sum_0 = 0; //reset sum
                
                
                
          }
        
            
         switch (t_av_n){
               case 1: {
                   t_av_0 = t_av; 
                   break;
               }
               case 2: {
                   t_av_1 = t_av;
                   t_av_n = 0; //reset counter
                
                   //check temperatures
                   //if bottom search should be first, this also needs to
                   //be changed accordingly
                   if(t_av_1 < t_av_0){
                       if(pwmval_c_top.flag == 1){
                           pwmval_c_top.flag = 0;
                           pwmval_c_bottom.flag = 1; //init bottom pwm search
                       }else if(pwmval_c_bottom.flag == 1){
                           pwmval_c_bottom.flag = 0;
                       }
                   }
   
                   //after checking the temperatures: increment or decrement the duty cycle
                   
                   //decrement and set
                   if(pwmval_c_top.flag == 1 && pwmval_c_top.dc > 0){
                    pwmval_c_top.dc -= PT_DC_STEP_TOP;
                    
                    pwmval_c = pwmval_c_top.dc;
                   //increment and set
                   }else if(pwmval_c_bottom.flag == 1 && pwmval_c_bottom.dc < PWMVAL_C_MAX_DC){
                    pwmval_c_bottom.dc += PT_DC_STEP_BOTTOM;    
                     
                    pwmval_c = pwmval_c_bottom.dc;
                   }    
                 break;  
                } //case 2, second average calculated
                default: t_av_n = 0;
            }
       
        //waiting PT_N_W measurements
         if(n_T_meas >= PT_N_W && pt_n_av_f == 0){
              n_T_meas = 0; //reset counter, waiting time is over
              pt_n_av_f = 1; //set average calculation flag
              
              //if everything is finished and new max pwm is set 
              if(pwmval_c_bottom.flag == 0 && pwmval_c_top.flag == 0 && pwmval_c_max.flag == 1 ){
               
                  
                  
              }
              
              
          }
        
         //Bottom, Top search finished?
         if(ptcT_amb_f == 1 && pwmval_c_bottom.flag == 0 && pwmval_c_top.flag == 0 ){
             //calculate new max pwm 
             pwmval_c_max.dc = (uint8_t) ((uint16_t) (pwmval_c_bottom.dc + pwmval_c_top.dc) / 2);
            
             //set pwmval
             pwmval_c = pwmval_c_max.dc;
             //pwmval_c_max.flag = 1; 
          
         
          
             //if everything is finished and pwm set, and 1 wait period passed,
      //reset everything
             
             //if(pwmval_c_max.flag == 0){
            //reset everything to starting values
                 
                ptcT_amb_f = 0; //start value
                pt_n_av_f = 0; //start with wait
                n_T_meas = 0;   
                pwmval_c_top.dc = PWMVAL_C_MAX_DC; //start value
                pwmval_c_bottom.dc = 0;
                
    
//flags
                pwmval_c_top.flag = 1; // Kalibration starts with top. at the beginning of 
                           // the Calibration top.flag != bottom.flag
                pwmval_c_bottom.flag = 0;
    
               // pwmval_c_max.flag = 0; //is set after max is set in Kalibration, reset after 1 wait period
  
    
                t_sum_0 = 0;
  
                
                t_av_n = 0; //average counter      
                
                pt_cali_f = 1; //calibration finished, new calibration cannot be started after flag is set to 0
                
                if(t_av_1 >= measRoomTemp){
                t_k = (float) t_av_1 - measRoomTemp;
                }
        // } //if(pwmval_c_max.flag == 0) inside of 
           //if(ptcT_amb_f == 1 && pwmval_c_bottom.flag == 0 && pwmval_c_top.flag == 0 )
        
    
            
        } //if(ptcT_amb_f == 1 && pwmval_c_bottom.flag == 0 && pwmval_c_top.flag == 0 ) 
         
        } //if(ptcT_amb_f == 1)
         
       
         */
         
    /**************************************************************************/
        
        
        //heating
        if(-30 < T3 && T3 < 120){
            pwmval_h = HeatingTempControl(set_ht_global, T3, 10);
            
        } //else use old value
        
       
        
        //Convert Percentage to Duty cycle value and set duty cycle
        //uint8_t h_res = SetHeatingPWM(pwmval_h); //heating result ACK or NACK, moved down: 17.08.20
        
     
        /*Over heat control of peltier element, checks hot and cool temperature*/
        //measPtCoolTemp = -10; //for testing purposes
        if((-20 < measPtCoolTemp && measPtCoolTemp < 100) && (-20 < measPtHotTemp && measPtHotTemp < 100) ){ //check if Temperatures are in valid range
            if(measPtHotTemp >= PT_OFF_T || measPtCoolTemp >= PT_OFF_T){ //turn off Temperature
            //if(measPtCoolTemp >= PT_OFF_T){   
                pt_ovh_f = 1; //set overheat flag
                
            
            }else if(measPtHotTemp <= PT_OFF_T - 20){
            // }else if( measPtCoolTemp <= PT_OFF_T - 10){
                pt_ovh_f = 0; //reset overheat flag 
            }
            
            if(measPtHotTemp - measPtCoolTemp >= PT_MAX_DT){
                pwmval_c = PT_MIN_PWM_DC; //minimum duty cycle of Pt element so that the cool side doest get to hot but the hot side doesnt over heat
               
            }else if(ptcT_amb_f == 0){ //measHotTemp < overHeat, calibration not running
                pwmval_c = CoolingTempControl(set_ct_global, measPtCoolTemp, 10);
                if(pwmval_c < PT_MIN_PWM_DC) {pwmval_c = PT_MIN_PWM_DC;}
            }
        }else{
           pwmval_c = PT_MIN_PWM_DC;  
          
        }
        
#if SAFETY_ON == 1 //added: 17.08.20
     if(idwt_resp != ACK){
         //set pwm of all to basically 0
         pwmval_h = 1; 
         pwmval_c = 1;
         DUTY_CYC_P = 0;     //turn off pump
     }
#endif
    
    /************************************************************************/
    //Setting pwm Values
    uint8_t h_res = SetHeatingPWM(pwmval_h);  
    
    if(pwmval_c > pwmval_c_max.dc) {pwmval_c = pwmval_c_max.dc;}
    if(pwmval_c == 0) {pwmval_c = 1;} //0 not in protokoll
    if(pt_ovh_f == 1) {pwmval_c = 1;}
    c_res = SetCoolingPWM(pwmval_c);
    
    
      
   
    //while(13); //testing wdt
    
    
    } //while 42
    
    
    return;
}

void _init_(void){
    OSCCONbits.IRCF = 0b111; //16MHz
    
    //LED1 main loop running
    ANSELBbits.ANSB2 = 0;
    TRISBbits.RB2 = 0;
    LATBbits.LB2 = 1;
    
    //LED2 Heating on
    ANSELBbits.ANSB3 = 0;
    TRISBbits.RB3 = 0;
    LATBbits.LB3 = 1;
    
    //LED3 cooling on 
    ANSELBbits.ANSB4 = 0;
    TRISBbits.RB4 = 0;
    LATBbits.LB4 = 1;
    
    //LED4 WDT test
    ANSELBbits.ANSB5 = 0;
    TRISBbits.RB5 = 0;
    //LATBbits.LB5 = 1;
    
    //CCP2 Timer
    CCP2_TimerSetup();
    
    //Serial
    SetupSerial();
    //use SerEnable = 1/0 to enable/disable Serial.
    
    //PWM
    Setup_Pump_PWM();
    Setup_Heating_PWM();
    Setup_Cooling_PWM();
    DUTY_CYC_C = 0; //34; //85% DC
    //Sensor
    ANSELCbits.ANSC5 = 0;
    
    INTCONbits.GIE = 0; // disable interrupt in general                         
    
   
}

///////////////////////////////////////////////////////////////////////////////
// This function writes a byte data_byte to the adress data_adress inside the 
// EEPROM
// the adress can be a value between 0 and 255 (maybe 1 to 256 idk)

void EEPROM_write(uint8_t data_byte, uint8_t data_adress){
    EEADR = data_adress; //set adress
    EEDATA = data_byte;
    EECON1bits.EEPGD = 0; //select EEPROM
    
    EECON1bits.WREN = 1; //enable writing
    INTCONbits.GIE = 0; //disable Interrupts
    EECON1bits.CFGS = 0; //acces EEPR memory
    
    //start sequence
    EECON2 = 0x55;
    EECON2 = 0xaa;
    EECON1bits.WR = 1; //start writing
    
    INTCONbits.GIE = 1; //enable interrupts
    
    while(PIR2bits.EEIF == 0); //wait for writing to finish
    EECON1bits.WREN = 0; //disable writing
    
}


///////////////////////////////////////////////////////////////////////////////
//This function reads a byte data_byte from the EEPROM using the adress
//data_adress
uint8_t EEPROM_read(uint8_t data_adress){
    EEADR = data_adress; //write adress to adress register
    EECON1bits.EEPGD = 0; //select EEPROM
    EECON1bits.RD = 1;
    uint8_t data_byte = EEDATA;
    
    return data_byte;
}
