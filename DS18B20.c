
// https://pic-projekte.de/
//Funktioniert nicht -> Funktionen werden umgeschreiben damit sie das 
//gleiche machen wie bei arduino fkt der onewire bib, genauere infos: header


#include "DS18B20.h"

uint8_t ow_mri (void)
{
    uint8_t rec;
    
    
    TRIS_DQ = 1; //input
    
    //wait until wire is high, just in case
    //while(!(DQ)); //code stops at this point if no sensor connected
    
	LAT_DQ = 0;						// set the (I)O to low level
    TRIS_DQ = 0;                    // config the DQ-IO as output (-> low)
    __delay_us(480);				// delay of >=480 us
    TRIS_DQ = 1;                    // config the  DQ-IO as input (high-z -> pull up)
    __delay_us(70);
    rec = DQ;                       // read the level (if low, slave available)
    __delay_us(410);                // wait for end of slot

    return (rec);
}

/*******************************************************************************
One_Wire_Write_Bit
--------------------------------------------------------------------------------
This function will write one single bit on the bus.
*******************************************************************************/

void ow_wr_bit (uint8_t val){

	           

    if(val){                   		// if the bit to transfer is != 0
    
        LAT_DQ = 0;
        TRIS_DQ = 0; //output low
        __delay_us(10);
        LAT_DQ = 1; //output high
        __delay_us(55);				
        
    }else{
        LAT_DQ = 0;
        TRIS_DQ = 0; //output low
        __delay_us(65);
        LAT_DQ = 1; //output high
        __delay_us(5);
    }
 
}

/*******************************************************************************
One_Wire_Write_Byte
--------------------------------------------------------------------------------
This function will write a complete byte on the bus.
*******************************************************************************/

void ow_wr_byte (uint8_t val)
{
    uint8_t i = 1;
    uint8_t mask = 1; //0b00000001

	// write the byte by sending eight bits (LSB first)
    for (i=0; i<8; i++)
    {
            ow_wr_bit(val & mask);  //mask contains seven 0 and one 1,val & mask makes sure that only one bit of val is "selected"
            mask = (mask << 1);     //the 1 in mask shifts each loop one step to the left
    }
}

/*******************************************************************************
One_Wire_Read_Bit
--------------------------------------------------------------------------------
This function will read one single bit from the bus.
*******************************************************************************/

uint8_t ow_rd_bit (void)
{
    uint8_t rec;

	// perform a very short low impuls
    TRIS_DQ = 0;					// config the DQ-IO as output (-> low)
    LAT_DQ = 0;	
    __delay_us(3);
    TRIS_DQ = 1;                    // config the  DQ-IO as input (high-z -> pull up)

    __delay_us(10);
    rec = DQ;                       // read the level on DQ (this is the read bit)
    __delay_us(53);                // wait for end of timeslot (datasheet of sensor)

    return(rec);
}

/*******************************************************************************
One_Wire_Read_Byte
--------------------------------------------------------------------------------
This function will read a complete byte from the bus.
*******************************************************************************/


uint8_t ow_rd_byte (void)
{
    uint8_t value = 0 , i;

    //read the byte by reading eight bits (LSB first)
    for(i=0; i<8; i++)
    {
        if ( ow_rd_bit() )
        {
            value |= 0x01 << i; //vgl. mask in wr_byte
        }
    }

    return(value);
}

int8_t ConvTempInt(uint8_t sensorData_par[]){ 
    int8_t temp_intern = -127; //-127 to +127
    int8_t lsb_intern = sensorData_par[0]; //bit7 2^3 to bit 0 2^-4
    int8_t msb_intern = sensorData_par[1]; //bit7-3 true -> temp<0, bit0 2^4 to bits 2^6
    
    //masks:
    //for this poject the temperature doesnt need to be exact, cutting the temperature at the point is ok
    //25.99°C -> 25°C
    uint8_t lsbMask_intern = 0b11110000;//use with & -> sets all &0 to 0, keeps all &1, remove everything < 1
    uint8_t msbMask_intern = 0b00000111;//only get the numbers
    uint8_t msbMaskSign_intern = 0b11111000;//get sign +,-
    
    
    /* Calculation of temperature:
     * 1. get only bit 7-4 of lsb by using lsbMask
     * 2. shift result to the right by 4 to get the actual number in lsb
     * 3. get only bit 0-1 of msb by using msbMask
     * 4. shift result to left by 4 to get actual number in msb
     * 5. add the actual numbers of lsb and msb to get absolute temperature
     * 6. get only Sign by using msbMaskSign on msb
     * 7. decide if temp is positive or negative depending on sign
     */
    
    //1.-5.
    temp_intern = ((lsb_intern & lsbMask_intern) >> 4) + ((msb_intern & msbMask_intern) << 4);
    
    //6.-7. 
    if(msb_intern & msbMaskSign_intern){
        temp_intern = -127 + temp_intern;
    }
     
     
    return temp_intern;
}

//selects sensor with 64 bit adress: sensorAdress_par[8]
//after this function the command function has to follow directly
void SelectSensor(uint8_t sensorAdress_par[8]){
    ow_wr_byte(Match_ROM);          //adress 1 Sensor 
        for(int i = 0; i < 8; i++){    //with following 64 bit adress
            ow_wr_byte(sensorAdress_par[i]);
        }
}

//ReadTemperature
//Reads Bus and converts data to Temperature between -127 and +127°C

int8_t ReadTemperature(void){
    uint8_t ds18b20_intern[9] = {0,0,0 ,0,0,0 ,0,0,0};
    int8_t T_intern = -127;
    
    ow_wr_byte(Read_scratch); //command: Read 9 bytes from Scratchpad
    for(uint8_t i = 0; i < 9; i++){
        ds18b20_intern[i] = ow_rd_byte(); //read scratchpad into array
    }
    
    T_intern = ConvTempInt(ds18b20_intern); //convert Temperature
    
    return T_intern; 
}





/* Test:
 * T not changed
 * output: 
 * 38
 * 38
 * 38
 * 38
 * 38
 * GND removed:
 * output:
 * none
 * 
 * Sensor removed -> input floating:
 * output:
 * 0
 * 0
 * 0
 * 0
 * 0
 * 
 * everything properly connected again:
 * output:
 * 38
 * 
 * teperature calulation cahnged to old: (<<4 instead of <<5)
 * output
 * 22
 * 22
 * 22
 * 
 * Sensor warmed by holding in my hand:
 * output
 * 23
 * 27
 * 28
 * 29
 * 29
 * 
 * putting sensor in hot tea
 * output
 * 46
 * 62
 * 66
 * 67
 * wait
 * 68
 * 67
 * out of tea
 * 50
 * 48
 * 
 */

