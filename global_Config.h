#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

//Troubleshooting:
//26.08.20
//lüfter funktioniert nicht sobald peltier elemente angeschlossen werden.
//keine Kürzschlüsse auf der platine, auch nicht bei den Mosfets
//Ursache: bei ca. 1A oder mehr fällt die spannung des netzteils auf ca. 11.5V
//dabei hört der lüfter auf zu arbeiten
//laut datenblatt: kurzschluss schutz im netzteil
//-> kurzschluss strom: kabel geschmoltzen. netzteil funktionier noch wie zuvor
//-> 10V lüfter kaufen
//zum Peltiertest: da der Mosfet auf der Leiterplatte nicht funktioniert habe ich versucht einen 
//mosfet den ich rumliegen hab zu verwenden und den maximalen dutydycle entsprechend
//anzupassen. leider wird der mosfet aber sehr schnell zu heiß
//bei einem dc von max 25% bleibt mein mosfet kalt. der lüfter läuft trotzdem nicht
//die versorgungsspannung ist 12.2v
//wenn peltier ausgesteckt: lüfter strom 120mA
//eingesteckt 0mA
//leds lassen sich mit dem mosfet wunderbar dimmen...

#pragma config FOSC = INTIO67 //  Internal oscillator block bis 16MHz

#pragma config LVP = OFF // Single - Supply ICSP Enable bit (Single - Supply ICSP disabled )

//watchdog timer 17.08.20
//time: 4s
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//code execution of while loop needs to be shorter than wdt period!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
#pragma config WDTEN = ON // Watchdog Timer Enable bits ( Watch dog timer disabled )
#pragma config WDTPS = 1024 //period = 4ms * 1024 = 4s

#include <xc.h>
#include <stdint.h>

//***************************************************************
//Change everything as needed

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 16000000 //16MHz
#endif

#define SAFETY_ON  0 //set to 1 if safety measures should be on
//Safety Measures: turn everything off if there is no Response from the RPI
/*
            DUTY_CYC_H = 0;     //turn off heating
            DUTY_CYC_P = 0;     //turn off pump
            DUTY_CYC_C = PT_MIN_PWM_DC;    //set cooling to some default value -> needs to be tested which is best
 */

/******************************************************************************/


extern int8_t set_ht_global; //Set heating Temperature
extern int8_t set_ct_global; //Set cooling temperature

/******************************************************************************/

/******************************************************************************/
//Safety and default Values
/******************************************************************************/

//Peltier Element
#define PT_MIN_PWM_DC  5   //minimum PWM duty Cycle in %, its to prevent the heat from the hot side to move to the cold side
                            // at this value the peltier element cant overheat. Actual value needs to be tested
#define PT_OH_T        40   //if the Hot side of the peltier element reaches this temperature
                            // the PWM Duty cycle will be set to PT_MIN_PWM_DC
#define PT_OFF_T       70   // at this Temperature the Peltier Element will be turned off to Prevent Damaging it.
                            // if its turned off the heat will travel from the hot side to the cool side causing the continer to heat up fast




/******************************************************************************/
//Serial Communication
/******************************************************************************/

#define SER_EN    RCSTA1bits.SPEN //1: enables EUSART and config Rx/Tx


//Codes
/* debugging codes */
/*
#define ACK         97  //Both: acknowledged
#define NACK        110 //Both: Not acknowledged. old: 102; new: 0 because 0 means nothing, easier to check

#define CPS         112 //'p' //RPI: Change Pump Speed
#define CHT         104 //'h' //RPI: Change Heating Temperature
#define CCT         99  //'c' //RPI: C1hange Cooling Temperature
#define CSS         115 //'s' //RPI: Change System State ON: values can be changed, OFF: values cant be changed

#define IWT         105 //'i' //PIC: I wanna talk
#define IDWT        100 //'d'  //PIC: I dont wanna talk
#define IWR         114 //'r'

#define FRAERR      70  //F //PIC: Framing Error, Restart Communication, last byte is lost
#define OVERERR     79  //O //PIC: Overrun Error, Restart Communication, last byte is lost
*/

/* production codes */

#define ACK         101 //Both: acknowledged
#define NACK        102 //Both: Not acknowledged.

#define CPS         103 //RPI: Change Pump Speed
#define CHT         104 //RPI: Change Heating Temperature
#define CCT         105 //RPI: C1hange Cooling Temperature
#define CSS         106 //RPI: Change System State ON: values can be changed, OFF: values cant be changed

#define IWT         201 //PIC: I wanna talk
#define IDWT        202 //PIC: I dont wanna talk
#define IWR         203 //RPI: I wanna read (the whole System State)

#define FRAERR      0//PIC: Framing Error, Restart Communication, last byte is lost
#define OVERERR     0//PIC: Overrun Error, Restart Communication, last byte is lost

/******************************************************************************/
//PWM:
//Heating = H
//Cooling = C
//Pump    = P
//Cooling RC2 CCP1
//Heating RB0 CCP4
//Pump    RA4 CCP5
/******************************************************************************/
#define DUTY_CYC_H  CCPR4L //set duty cycle for Heating, between 0-PR
#define DUTY_CYC_C  CCPR1L //set duty cycle for cooling, between 0-PR
#define DUTY_CYC_P  CCPR5L //set duty cycle for Pump, between 0-PR

#define DUTY_CYC_C_MAX 10 //Max duty Cycle for cooling, actual value that is Written into the CCPRxL Register,
                          //it needs to be smaller than the Value written in PR_C


//Value to write in PRx register to set frequency, depending on prescaler
#define PR_P 199 //5kHz
#define PR_H PR_P //5kHz, Heating and Pump use the same timer => same frequency, but different duty cycles possible
#define PR_C 40 //255 15.6kHz ,40 97,5kHz

/******************************************************************************/
//Timer: 
//CCP2, Timer1
/******************************************************************************/

//#define CCP2_F PIR2bits.CCP2IF //moved to CCP2_timer

/******************************************************************************/
//DS18B20 Temperature Sensor
//Pin RC5
/******************************************************************************/

#define LAT_DQ          LATCbits.LC5
#define DQ              PORTCbits.RC5
#define TRIS_DQ         TRISCbits.TRISC5

#define Skip_ROM        0xCC //Alle Sensoren am Bus ansprechen
#define Read_ROM        0x33 //Read ROM Address. Use only if one Sensor is on the Bus!
//If more than one sensor is on the bus, all sensor are sending their adresses 
//at the same time causing data collisions

#define Match_ROM       0x55 //address one Sensor with the Address sent after this command
#define Alarm_Search    0xEC //Search for temperature alarm on the Bus

#define Convert_T 0x44 //Leitet die Temperaturmessung ein
#define Read_scratch 0xBE //Liest das Scratchpad inklusive des CRC Bytes aus
#define Write_scratchpad 0x4E //Schreibt 3 Byte in das Scratchpad (Byte1 -> TH, Byte2 -> TL, Byte3 Config)
#define Copy_scratchpad 0x48 //Kopiert TH und TL aus dem EEPROM zum Scratchpad
//#define Recall_E 0xB8 //Startet einen Rückruf von TH und TL vom EEPROM zum Scratchpad
//#define Read_Power_Supply 0xB4 //Signalisiert den Versorgungszustand zum PIC
#define Resolution_11bit 0x5F //9bit 375ms wait!
#define Resolution_12bit 0x7F //12bit 750ms wait! Config-Reg. Bit 6,5 = 11 s. Figure 10



#endif //GLOBAL_CONFIG_H
