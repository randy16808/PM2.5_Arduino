#include <SoftwareSerial.h>
#define TxEnablePin 3
SoftwareSerial mySerial(10, 11); // RX, TX

unsigned char buffer[8];
unsigned char receive[8];
void setup() {
  // put your setup code here, to run once:
  buffer[0] = 0x01;
  buffer[1] = 0x03;
  buffer[2] = 0x00;
  buffer[3] = 0x02;
  buffer[4] = 0x00;
  buffer[5] = 0x01;
  buffer[6] = 0x25;
  buffer[7] = 0xCA;

  mySerial.begin(9600);
  Serial.begin(9600,SERIAL_8E1);
  Serial.setTimeout(500);
   pinMode(TxEnablePin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.read() != -1);
  digitalWrite(TxEnablePin,1);
  for(int i=0;i<8;i++)
  {
    Serial.write(buffer[i]);
  }
  
  Serial.flush();
  digitalWrite(TxEnablePin, 0);

  //delay(1000); 
  
  /*if(Serial.available())
  {
    mySerial.println("hello");
      int i = 0;
      while(Serial.available())
      {
        receive[i] = Serial.read();
        i++;
        //delay(50);
      }
     
      mySerial.println(i);
  }*/
  int n = Serial.readBytes(receive, 7);
  //mySerial.println(n);
 /*for(int i=0;i<n;i++)
 {
  int k = receive[i];
  //mySerial.print(k,HEX);
 }*/
 //mySerial.println("hello2");
 //Serial.flush();
 
  delay(500);
  
  
  
}
