/*
 * There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.
 * 
 * U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
 * U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
 * U2UXD is unused and can be used for your projects.
 * https://circuits4you.com/2018/12/31/esp32-hardware-serial2-example/
*/

#define RXD2 16
#define TXD2 17

//Debugging
/*
#define NACK  'n'
#define ACK   'a'
#define IWT   'i'
#define IDWT  'd'
#define IWR   'r'
#define CPS   'c'
#define CHT   'h'
*/

//Production
#define NACK  102
#define ACK   101
#define IWT   201
#define IDWT  202
#define IWR   203
#define CPS   103
#define CHT   104
#define CCT   105 //RPI: C1hange Cooling Temperature

#define S_PT_PWM    107 //send pt pwm used only for debugging. if received the pic will send the pwmval_c

//Temperatures: set, measured temperatures +20

#define REC_TW_PIC 20 //serial timewindow on pic

int ledpin = 2;
int pinstate = LOW;

uint8_t iwrF = 0; //i wanna read flag
uint8_t iwtF = 0; 

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, pinstate);
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //Serial2.begin(9600, SERIAL_8N1);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));
}



char rec = NACK;
char lastrec = NACK;
uint8_t val_buffer[] = {0,0,0,0,0}; //3 temperatures and pwmval
uint8_t temp_buf[] = {0,0,0};
uint8_t byte_counter = 0;
//Communication per user input geht nicht
void loop() { //Choose Serial1 or Serial2 as required
  char usercom = Serial.read();
  char userdata = 0;
  if(usercom == 'p'){ //user wants to read serial monitor
    char p = usercom;
    while(p != 's'){
      usercom = Serial.read();
      if(usercom == 's'){
        p = 's'; //s for start
      }else if(usercom == IWR || usercom == CPS || CHT){ 
        userdata = usercom;
      }
    }
  }
  
  
  if (Serial2.available()) { //1 byte gets lost
    rec = Serial2.read();
    pinstate = !pinstate;
    digitalWrite(ledpin, pinstate);
    

    //only print pwmval
    float zeit = millis()/1000; //seconds since starting
    rec = (uint8_t) rec;
    if(rec == IWT) iwtF = 1;
    if(rec == IDWT) iwtF = 0;
    if(iwtF == 0) byte_counter = 0;
    if(iwtF == 1 && rec <= 100 && rec > 0){
      val_buffer[byte_counter] = rec;
      byte_counter++;
      
    }

    if(iwtF == 0){ //everything received
      uint8_t t_c_h = val_buffer[0] - 20;
      uint8_t t_c_c = val_buffer[1] - 20;
      uint8_t t_r = val_buffer[2] - 20;
      uint8_t pwmval_c = val_buffer[3];
      Serial.println(String((float) zeit) + "  " + String(t_c_h)+ " " + String(t_c_c)+ " " + String(t_r)+ " " + String(pwmval_c));
    }
    

    
    if (rec == IWT || rec == IDWT){
      char lastrec = rec; //IWT or IDWT
      delay(REC_TW_PIC / 2); 
      Serial2.write(ACK); //PIC answers with ACK after receiving this ACK
      //Serial.println("ESP ACK: " + String(ACK));

      if(rec == IWT || iwrF == 0){

      delay(5);
      Serial2.write(IWR);

      delay(5);
      //Serial.println("ESP IWR: " + String(IWR));
      iwrF = 1; //next 3 rec are temperaures
      iwrF = 0;
      }
      Serial2.write(S_PT_PWM); //request pt pwm
      delay(10);
      Serial2.write(CCT);
      Serial2.write(10); //pic sends 101, means 18°C
      delay(5);
      Serial2.write(CHT);
      Serial2.write(70);//50°C heating

      delay(1); //wait 1ms 
    }
    

    
  }
}

