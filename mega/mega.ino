#include <LiquidCrystal.h>          //the liquid crystal library contains commands for printing to the display
#include <nRF24L01.h>
#include <RF24.h>

// struct for data transfer between devices
struct SensorData {
  int AQIData;
  float photoData;
  float USDistance1;
  float USDistance2;
  float USDistance3;
  int BPM;
};

// Define pins
RF24 radio(36,38);          // CE, CSN
const byte address[6] = "00001";
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
const int buttonPin = 2;   // Arduino pin connected to button's pin
const int ledPin = 5;       // Arduino pin connected to button's pin
const int buzzerPin = 4;    // Arduino pin connected to button's pin
const byte requestByte = 1;
const String labelArray[] = {"PM2.5: ", "Photoresistor: ", "Distance1: ", "Distance2: ", "Distance3: ", "BPM: "};


int sensorToReport;
bool toReport = false;
bool toRequest = false;
unsigned long buttonDownTime = 0;
unsigned long buttonUpTime = 0;
bool buttonClick = false;
int displayIndex = 0; //0 for AQI, 1 for photoresistor, 2 for distance
SensorData sensorData;
bool buttonIsUp = true;
bool indexChanged = false;
bool isDark = false;
bool isClose = false;

void setup() {
  Serial.begin(9600);
  Serial.println("started ... ");
  lcd.begin(16, 2);                 //tell the lcd library that we are using a display that is 16 characters wide and 2 characters high
  lcd.clear();                      //clear the display
  pinMode(buttonPin, INPUT_PULLUP);       // set arduino pin to input pull-up mode
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonChange, CHANGE);
  pinMode(ledPin, OUTPUT);              //set pin 5 as an output that can be set to HIGH or LOW
  pinMode(buzzerPin, OUTPUT);              //set pin 4 as an output that can be set to HIGH or LOW
  
  radio.begin();                  //Starting the Wireless communication
  //radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();          //This sets the module as transmitter
  
  displayDataOnLCD(displayIndex);
}

void loop() {


  if (toRequest) {
    Serial.println("requesting..");
    radio.write(&requestByte, sizeof(byte));
    noInterrupts();
    toRequest = false;
    interrupts();
  }

  // read sensor information from peripheral
  if (radio.available()) {
    //SensorData temp;
    //int tempint;

    radio.read(&sensorData, sizeof(SensorData));
    displayDataOnLCD(displayIndex);

    Serial.println(sensorData.AQIData);
    Serial.println(sensorData.photoData);
    Serial.println(sensorData.USDistance1);
    Serial.println(sensorData.USDistance2);
    Serial.println(sensorData.USDistance3);
    Serial.println(sensorData.BPM);
  }

  if (sensorData.photoData < 500) {
    isDark = true;
  } else {
    isDark = false;
  }

  if(isDark) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  if (sensorData.USDistance < 50.0) {
    isClose = true;
  } else {
    isClose = false;
  }

  if(isClose) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  // if a button click happened and is not processed yet
  if (buttonClick) {
    Serial.println("button clicked");
    //processButtonEvent();
    buttonClick = false;
  }

  //Serial.println(labelArray[displayIndex]);

  //ON DEMAND SENSOR DATA COLLECTION
  if (toReport) {

    Serial.print("PM2.5: ");
    Serial.println(sensorData.AQIData);
    Serial.print("Photoresistor: ");
    Serial.println(sensorData.photoData);
    Serial.print("Distance: ");
    Serial.println(sensorData.USDistance1);
    Serial.print("Distance: ");
    Serial.println(sensorData.USDistance2);
    Serial.print("Distance: ");
    Serial.println(sensorData.USDistance3);
    Serial.println("BPM: ");
    Serial.println(sensorData.BPM);

    toReport = false;
  }

  if (indexChanged) {
    displayDataOnLCD(displayIndex);
    indexChanged = false;
  }

  delay(100);
}

void displayDataOnLCD(int index) {
  lcd.clear();
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
      lcd.print(sensorData.USDistance1);
      break;
    case 3:
      lcd.print(sensorData.USDistance2);
      break;
    case 4:
      lcd.print(sensorData.USDistance3);
      break;
    case 5:
      lcd.print(sensorData.BPM);
      break;
  }
}



void buttonChange() {
  noInterrupts();
  if (buttonIsUp) {
    buttonIsUp = false;
    buttonDownTime = millis();
  } else {
    buttonClick = true;
    buttonIsUp = true;
    long buttonDuration = millis() - buttonDownTime;
    if (buttonDuration >= 1000) {
      toRequest = true;
    } else {
      ++displayIndex;
      if (displayIndex > 2) {
        displayIndex = 0;
      }
      indexChanged = true;
    }
  }
  interrupts();
}
