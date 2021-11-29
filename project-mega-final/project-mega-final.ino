#include <LiquidCrystal.h>          //the liquid crystal library contains commands for printing to the display
#include <nRF24L01.h>
#include <RF24.h>

// struct for data transfer between devices
struct SensorData {
    int AQIData;
    float photoData;
    float USDistance;
};

// Define pins
RF24 radio(8, 9);          // CE, CSN
LiquidCrystal lcd(12,11,10,9,8,7);
const int buttonPin = 2;   // Arduino pin connected to button's pin
const int ledPin = 5;       // Arduino pin connected to button's pin
const int buzzerPin = 4;    // Arduino pin connected to button's pin

int sensorToReport;
bool toReport = false;
bool toRequest = false;
unsigned long buttonDownTime = 0;
unsigned long buttonUpTime = 0;
bool buttonClick = false;
int displayIndex = 0; //0 for AQI, 1 for photoresistor, 2 for distance
SensorData sensorData;

const byte requestByte = 1;
const String labelArray[] = {"PM2.5: ", "Photoresistor: ", "Distance: "};
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  Serial.print("started ... ");
  lcd.begin(16, 2);                 //tell the lcd library that we are using a display that is 16 characters wide and 2 characters high
  lcd.clear();                      //clear the display
  pinMode(buttonPin, INPUT_PULLUP);       // set arduino pin to input pull-up mode
  //attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, RISING); //interrupt to track button's state
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonDown, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonUp, RISING);
  pinMode(ledPin, OUTPUT);              //set pin 5 as an output that can be set to HIGH or LOW
  pinMode(buzzerPin, OUTPUT);              //set pin 4 as an output that can be set to HIGH or LOW
  radio.begin();
  radio.openWritingPipe(address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  //radio.startListening();              //This sets the module as receiver
  radio.stopListening();

}

void loop() {

  if (toRequest) {
    radio.write(&requestByte, sizeof(byte));
    noInterrupts();
    toRequest = false;
    interrupts();
  }

  // read sensor information from peripheral
  if (radio.available()) {
    radio.read(&sensorData, sizeof(SensorData));
  }
  // if a button click happened and is not processed yet
  if (buttonClick) {
    Serial.println("button is clicked");
    processButtonEvent();
    buttonClick = false;
  }
  

  //ON DEMAND SENSOR DATA COLLECTION  
  if(toReport){
    //Serial.println("Entered ISR for button");
    //recieve data
    //display the data from air-quality sensor 

    Serial.print("PM2.5: ");
    Serial.println(sensorData.AQIData);
    Serial.print("Photoresistor: ");
    Serial.println(sensorData.photoData);
    Serial.print("Distance: ");
    Serial.println(sensorData.USDistance);
    
    toReport = false;
  }

  lcd.setCursor(0, 0);              //set the cursor to the 0,0 position (top left corner)
  lcd.print(labelArray[displayIndex]);       
  
  lcd.setCursor(0, 1);              //move the cursor to the first space of the bottom row
  switch (displayIndex) {
    case 0:
      lcd.print(sensorData.AQIData);  
      break;
    case 1:
      lcd.print(sensorData.photoData);  
      break;
    case 2:
      lcd.print(sensorData.USDistance);  
      break;
  }
    
}

void buttonDown() {
  noInterrupts();
  buttonDownTime = millis();
  interrupts();
}

void buttonUp() {
  noInterrupts();
  buttonUpTime = millis();
  buttonClick = true;
  interrupts();
}

void buttonInterrupt(){
  toReport = true;
  //Serial.println("button >>> interrupt .. ");
}

void processButtonEvent() {
  noInterrupts();
  long timeInterval = buttonUpTime - buttonDownTime; // get the duration the button is pressed down
  if (timeInterval > 1000) { // if pressed for > 1s, request data from peripheral
    toRequest = true;
  } else {                    // if pressed shorter than 1s, rotate display
    ++displayIndex;
    if (displayIndex > 2){
      displayIndex = 0;
    }
  }
  interrupts();
}
