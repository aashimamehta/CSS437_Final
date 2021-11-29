#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";
//boolean button_state = 0;
//int led_pin = 3;

void setup() {
  //pinMode(6, OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  //radio.startListening();              //This sets the module as receiver
  radio.stopListening();
}
void loop()
{

  char dot[] = "embedded";
  radio.write(&dot, strlen(dot));
  delay(5);
}
