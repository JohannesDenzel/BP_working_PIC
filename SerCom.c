/*
 * File:   SerCom.c
 * Author: Megaport
 *
 * Created on 24. Mai 2020, 19:07
 */


#include "SerCom.h"

void SetupSerial(void){
    //RC7_RX, RC6_TX, x=1
    
    //Enable Transmitter RC6_TX
    ANSELCbits.ANSC6 = 0; //digital
    TRISCbits.RC6 = 1; //input
    
    TXSTA1bits.TXEN = 1; //enables transmitter
    TXSTA1bits.SYNC = 0; //config for async op
    RCSTA1bits.SPEN = 0; //1: enables EUSART and config Rx/Tx
    
    
    //Enable Receiver RC7_RX
    ANSELCbits.ANSC7 = 0; //digital
    TRISCbits.RC7 = 1; //input
    
    RCSTA1bits.CREN = 1;  //enables receiver circuit
    //TXSTAxbits.SYNC = 0 enables async mode //muss nur einaml gesetzt werden wenn ich receiver und transmitter nutz
    //RCSTAxbits.SPEN = 1 enables eusart //muss nur 1 mal gesetzt werden

    //Baudgenerator
    //9600baud at FOSC = 16MHz, err = 0,08%
   
    BAUDCON1bits.BRG16 = 0;
    TXSTA1bits.BRGH = 1;
    SPBRG1 = 103;
    
    //another option
    //9600baud	err=0,16%	SPBRGx = 103	SYNC = 0	BRGH = 1	BRG16 = 0
    
}





//if needed perform a type conversion
//function sends one byte via serial
void SerialSend(uint8_t data_par){
        while(!(PIR1bits.TX1IF)); //if data is currently sent, wait till it finishes, 1: buffer empty, 0: full
        
        TXREG1 = data_par;//Send Data 
        __delay_us(1); //wait a little bit so that the IF bit can be set
}

uint8_t SerialRec(void){
    uint8_t recData_intern = NACK; //value for no data Recived = default value
    if(PIR1bits.RC1IF == 1){ //data in FIFO = receiving buffer
               __delay_us(1); //works better with the short delays inbetween
               if(RCSTA1bits.OERR == 0){ //no Overrun error
                    __delay_us(1);
                    if(RCSTA1bits.FERR == 0){ //no Framing Error
                        recData_intern = RCREG1; //read receive register
                        __delay_us(1);
                    }else{ //Framing Error
                        SerialSend(FRAERR); //send Framing error code
                        RCSTA1bits.SPEN = 0;  //reset eusart
                        __delay_ms(1);
                        RCSTA1bits.SPEN = 1;
                    }
               } else{ //overrun error
                 SerialSend(OVERERR);//send Overrun error code
                 //RCSTA1bits.SPEN = 0;  //reset eusart, clear everything
                 RCSTA1bits.CREN = 0; //reset receiver circuit, clearing everything
                 //works better than resetting the whole module
                 __delay_ms(1);
                 //RCSTA1bits.SPEN = 1; 
                 RCSTA1bits.CREN = 1;
               }
            } //Data in FIFO
    
    return recData_intern;
    
}






