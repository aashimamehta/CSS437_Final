#include <LiquidCrystal.h>          //the liquid crystal library contains commands for printing to the display
#include <nRF24L01.h>
#include <RF24.h>

#define REQUEST_HOLD_TIME 1000    // hold down the button this many milliseconds to request data
#define ALERT_DISTANCE 3.0       // distance when buzzer and vibration motor will go off

int msgRecievedCount = 0;

// struct for data transfer between devices
struct SensorData {
  float photoData;
  int BPM;
  int stepData;
  float temperatureData;
  float USDistance_Front;
  float USDistance_Left;
  float USDistance_Right;
};

// Define pins
RF24 radio(36,38);          // CE, CSN
const byte address[6] = "00001";
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
const int buttonPin = 2;   // Arduino pin connected to button's pin
const int ledPin = 5;       // Arduino pin connected to button's pin
const int buzzerPin = 4;    // Arduino pin connected to buzzer's pin
const int FRONT_MOTOR_PIN = 44;
const int LEFT_MOTOR_PIN = 46;
const int RIGHT_MOTOR_PIN = 48;

const byte requestByte = 1;
const String labelArray[] = {"Photoresistor: ", "Heart Rate: ", "Step count: ", "Temperature: ", "Front Distance: ", "Left Distance: ", "Right Distance: "};


int sensorToReport;
bool toReport = false;
bool toRequest = false;
unsigned long buttonDownTime = 0;
unsigned long buttonUpTime = 0;
bool buttonClick = false;
int displayIndex = 0;     //0 for AQI, 1 for photoresistor, 2 for distance
SensorData sensorData;
bool buttonIsUp = true;
bool indexChanged = false;
bool isDark = false;
bool isClose = false;
bool frontIsClose = false;
bool leftIsClose = false;
bool rightIsClose = false;

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
  pinMode(FRONT_MOTOR_PIN, OUTPUT); 
  pinMode(RIGHT_MOTOR_PIN, OUTPUT); 
  pinMode(LEFT_MOTOR_PIN, OUTPUT); 
}

void loop() {


  if (toRequest) {                          // request new data from peripheral
    Serial.println("requesting..");
    radio.write(&requestByte, sizeof(byte));
    noInterrupts();
    toRequest = false;
    interrupts();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Requesting...");
    delay(300);
    displayDataOnLCD(displayIndex);
  }

  // read sensor information from peripheral
  if (radio.available()) {
    //SensorData temp;
    //int tempint;
    noInterrupts();
    radio.read(&sensorData, sizeof(SensorData));
    interrupts();
    displayDataOnLCD(displayIndex);

    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.print("  Photoresistor: ");
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.println(sensorData.photoData);
    Serial.print("  BPM : ");
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.println(sensorData.BPM);
    Serial.print("  Step count: ");
    Serial.println(sensorData.stepData);
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.print("  Temperature: ");
    Serial.println(sensorData.temperatureData);
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.print("  Distance Front : ");
    Serial.println(sensorData.USDistance_Front);
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.print("  Distance Left : ");
    Serial.println(sensorData.USDistance_Left);
    Serial.print("Message Recieved Count: ");
    Serial.print(msgRecievedCount++);
    Serial.print("  Distance Right: ");
    Serial.println(sensorData.USDistance_Right);
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

  if (sensorData.USDistance_Front < ALERT_DISTANCE && sensorData.USDistance_Front > 0) {
    isClose = true;
    frontIsClose = true;
    digitalWrite(FRONT_MOTOR_PIN, HIGH);
  } else {
    frontIsClose = false;
    digitalWrite(FRONT_MOTOR_PIN, LOW);
  }

  if (sensorData.USDistance_Left < ALERT_DISTANCE && sensorData.USDistance_Left > 0) {
    isClose = true;
    leftIsClose = true;
    digitalWrite(LEFT_MOTOR_PIN, HIGH);
  } else {
    leftIsClose = false;
    digitalWrite(LEFT_MOTOR_PIN, LOW);
  }

  if (sensorData.USDistance_Right < ALERT_DISTANCE && sensorData.USDistance_Right > 0) {
    isClose = true;
    rightIsClose = true;
    digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  } else {
    rightIsClose = false;
    digitalWrite(RIGHT_MOTOR_PIN, LOW);
  }

  if (!(frontIsClose || leftIsClose || rightIsClose)) {
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

    Serial.print("Photoresistor: ");
    Serial.println(sensorData.photoData);
    Serial.print("BPM: ");
    Serial.println(sensorData.BPM);
    Serial.print("Step count: ");
    Serial.println(sensorData.stepData);
    Serial.print("Temperature: ");
    Serial.println(sensorData.temperatureData);
    Serial.print("Front Distance: ");
    Serial.println(sensorData.USDistance_Front);
    Serial.print("Left Distance: ");
    Serial.println(sensorData.USDistance_Left);
    Serial.print("Right Distance: ");
    Serial.println(sensorData.USDistance_Right);

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
      lcd.print(sensorData.photoData);
      break;
    case 1:
      lcd.print(sensorData.BPM);
      break;
    case 2:
      lcd.print(sensorData.stepData);
      break;
    case 3:
      lcd.print(sensorData.temperatureData);
      break;
    case 4:
      lcd.print(sensorData.USDistance_Front);
      break;
    case 5:
      lcd.print(sensorData.USDistance_Left);
      break;
    case 6:
      lcd.print(sensorData.USDistance_Right);
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
    if (buttonDuration >= REQUEST_HOLD_TIME) {
      toRequest = true;
    } else {
      ++displayIndex;
      if (displayIndex > 6) {
        displayIndex = 0;
      }
      indexChanged = true;
    }
  }
  interrupts();
}
