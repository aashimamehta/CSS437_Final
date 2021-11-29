#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(8, 9); // CE, CSN         
const byte address[6] = "00001";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.
//int button_pin = 2;
//boolean button_state = 0;
void setup() {
  //pinMode(button_pin, INPUT);
  Serial.begin(9600);
  radio.begin();     //Starting the Wireless communication
  radio.flush()
  
  //radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();          //This sets the module as transmitter
}
void loop()
{

  if (radio.available()) {
    char msg[32] = "";
    radio.read(&msg, sizeof(msg));
    Serial.println(msg);
  } else {
    //Serial.println("here");
  }
  delay(10);
}
