/*
 * File:   CCP4_Timer.c
 * Author: denzel
 *
 * Created on 28. Mai 2020, 21:51
 */


#include "CCP2_Timer.h"


void CCP2_TimerSetup(void){
   // setup timer 1
    T1CONbits.TMR1CS = 0; // Timer1 clock source is oscillator clock (Fosc/4). 4MHz
    T1CONbits.T1CKPS = 0b10; // 2^2 -> 1:4 Prescale value -> 1MHz
    T1CONbits.TMR1ON = 1; // Timer on
    T1GCONbits.TMR1GE = 0; // Timer1 counts regardless of gate function

    // setup capture and compare module CCP2
    CCP2CONbits.CCP2M = 0b1011; // Compare mode with Special Event Trigger, CCP2IF is set
    CCPTMRS0bits.C2TSEL = 0; // timer <-> ccp module (CCP2 / TMR1)
    CCPR2 = 64000; // compare value for setting CCP2IF default 

    //clear overflow flag bits, called IF
    //Timer 1
    PIR1bits.TMR1IF = 0; // clear overflow bit of timer 1
    
    //CCP2 Module
    PIR2bits.CCP2IF = 0; // clear match of compare module CCP1
 
}

//sets timer and resets flags
//max Resolution 1us, 
//imput time in us
//1 increment is 1us -> up to 65000us possible
void CCP2_SetTimer(uint16_t timeSet_par){
    //Timer 1
    PIR1bits.TMR1IF = 0; // clear overflow bit of timer 1
    
    //CCP2 Module
    PIR2bits.CCP2IF = 0; // clear match of compare module CCP2
    CCPR2 = timeSet_par;
}

//timer counts until match occours
//so a reset timer function might be needed to set the timer back to 0 
//if the match is not needed anymore, or before setting it

//resets timer by letting a match occour after a short amount of time (1us)
void CCP2_Reset(void){
    //set timer with 1us
    CCP2_SetTimer(1);
    while(!PIR2bits.CCP2IF); //wait until match occours -> timer counter is automatically reset
    //reset Flags
    //Timer 1
    PIR1bits.TMR1IF = 0; // clear overflow bit of timer 1, actually not needed because TMR1 never overflows
    //but to be save...
    
    //CCP2 Module
    PIR2bits.CCP2IF = 0; // clear match of compare module CCP2
}





