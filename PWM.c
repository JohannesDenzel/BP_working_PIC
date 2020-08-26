/*
 * File:   PIC_PWM3x.c
 * Author: Megaport
 *
 * Created on 26. Mai 2020, 22:03
 */



#include "PWM.h"


   
     /*
      * the PR value corresponds only to the freq. but the freq can be the same for all pwm 
      * Signals.
      * only the duty cycle has to be changen and that doesnt change anything for the timer!
      * 
      * Possible CCP pins:
      * CCP1    RC2 <----
      * CCP2    RB3 RC1 mit LED 2 verbunden X---
      * CCP3    RB5 RC6 = TX X---
      * CCP4    RB0 mit Encoder und C=100nF verbuden <---
      * CCP5    RA4 RA4  <-----
      * 
      * 
      * 
      * 
      * PWM Period = (PRx + 1)*4*TOSC*(TMRx Prescale Value)
      * TOSC = 1/FOSC
      * FOSC = 16MHz -> TOSC = 0,0625us
      * 
      * duty cycle Resolution = log(4*(PRx+1))/log(2) bits
      * 
      * Prescaler   PR2     PWM Period      PWM Freq        Resolution
      * 00 1        0       0,25us          4000kHz         2bits
      *             255     64us            15,6kHz         10bits (8bits)
      * 01 4        0       1us             1000kHz         2bits
      *             255     256us           3,9kHz          10bits (8bits)
      * 1x 16       0       4us             250kHz          2bits
      *             255     1024us          0,977kHz        10bits (8bits)
      * 
      * set Prescaler: 1x 4
      * Period/1us - 1 = PR2
      * (1/(1us * F_PWM)) - 1 = PR2
      * F_PWM = 10kHz -> PR2 = 99 exakt
      * Resolution = 8,6 bits -> 8 bits: 255 steps -> 1,5625% duty cycle resolution
      * 
      * working LED4 1khz:
      * PR2 = 240
      * prescaler: 10 16
      * 
      */
    


//same Timer as Heating
void Setup_Pump_PWM(void){
    
    /* CCP5 RA4
     * Timer 2
     * 5kHz
     * Prescaler: 01 1:4
     * PR: 199
     * 
     * 
    */
    
    //1. Pins
    //first Input then after config output
    
    TRISAbits.RA4 = 1; //Input
    

    
    //2. select Timer2 for CCP2
    CCPTMRS1bits.C5TSEL = 0b00; 
    
    //3. slect PR value for Timer2 0-255(IR set after TMR2 = PR2), for Changing Freq
    PR2 =  PR_P; 
    
    //4. CCP2 0b11xx PWM Mode
    CCP5CONbits.CCP5M = 0b1100;
    
    //5. start value for dutycycle 0 to PR
    DUTY_CYC_P = 0;
    
    //6. config and enable Timer2
    
    //timer needs to be enabled before PWM output (Tris = 0)
    //its ok if timer is enabled and configured multible times.
    //the last configuration is the used configuration!
    
    PIR1bits.TMR2IF = 0;       //clear IR Flag
    T2CONbits.T2CKPS = 0b01;   //Prescaler 00 1:1, 01 1:4, 1x 1:16, for changing Freq
    T2CONbits.TMR2ON = 1;      //Timer2 on

    //while(!PIR1bits.TMR2IF); //wait until timer overflows, only important if phase is important
    TRISAbits.RA4 = 0; //enable output driver on RC2
    
    
}

//Same Timer as Pump
void Setup_Heating_PWM(void){
    /* CCP4 RB0
     * Timer 2
     * 5kHz
     * Prescaler: 01 1:4
     * PR: 199
    */
    //1. Pins
    //first Input then after config output
    ANSELBbits.ANSB0 = 0; //Digital
    TRISBbits.RB0 = 1; //Input
    
    //2. select Timer2 for CCP4
    CCPTMRS1bits.C4TSEL = 0b00; 
    
    //3. slect PR value for Timer2 0-255(IR set after TMR2 = PR2), for Changing Freq
    PR2 =  PR_H; 
    
    //4. CCP4 0b11xx PWM Mode
    CCP4CONbits.CCP4M = 0b1100;
    
    //5. start value for dutycycle 0 to PR
    DUTY_CYC_H = 0;
    
    //6. config and enable Timer2
   
    //timer needs to be enabled before PWM output
    //its ok if timer is enabled and configured multible times
    
    PIR1bits.TMR2IF = 0;       //clear IR Flag
    T2CONbits.T2CKPS = 0b01;   //Prescaler 00 1:1, 01 1:4, 1x 1:16, for changing Freq
    T2CONbits.TMR2ON = 1;      //Timer2 on

    //while(!PIR1bits.TMR2IF); //wait until timer overflows, only important if phase is important
    TRISBbits.RB0 = 0; //enable output driver on RC2
    
    
    
}


void Setup_Cooling_PWM(void){
    /* CCP1 RC2
     * Timer 4
     * 100kHz -> 97,5kHz
     * Prescaler: 00 1:1
     * PR: 40
     */
    
    //1. Pins
    //first Input then after config output
    ANSELCbits.ANSC2 = 0; //Digital
    TRISCbits.RC2 = 1; //Input
    
    //2. select Timer4 for CCP1
    CCPTMRS0bits.C1TSEL = 0b01; 
    
    //3. slect PR value for Timer2 0-255(IR set after TMR2 = PR2), for Changing Freq
    PR4 =  PR_C; 
    
    //4. CCP1 0b11xx PWM Mode
    CCP1CONbits.CCP1M = 0b1100;
    
    //5. start value for dutycycle 0 to PR
    DUTY_CYC_C = 0;
    
    //6. config and enable Timer2
   
    //timer needs to be enabled before PWM output
    //its ok if timer is enabled and configured multible times
    
    PIR5bits.TMR4IF = 0;       //clear IR Flag
    T4CONbits.T4CKPS = 0b00;   //Prescaler 00 1:1, 01 1:4, 1x 1:16, for changing Freq
    T4CONbits.TMR4ON = 1;      //Timer4 on

    //while(!PIR5bits.TMR4IF); //wait until timer overflows, only important if phase is important
    TRISCbits.RC2 = 0; //enable output driver on RC2
    
    
    
    
    
}



