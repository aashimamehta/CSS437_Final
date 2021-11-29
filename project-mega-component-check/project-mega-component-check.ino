#include <LiquidCrystal.h>          //the liquid crystal library contains commands for printing to the display

// Define pins
const int buttonPin = 2;   // Arduino pin connected to button's pin
const int ledPin = 5;       // Arduino pin connected to button's pin
const int buzzerPin = 4;    // Arduino pin connected to button's pin

int pressedCount = 0;
int sensorToReport;
int buttonState = 0; 
bool toReport = false;
LiquidCrystal lcd(12,11,10,9,8,7);

void setup() {
  Serial.begin(9600);
  Serial.print("started ... ");
  lcd.begin(16, 2);                 //tell the lcd library that we are using a display that is 16 characters wide and 2 characters high
  lcd.clear();                      //clear the display
  pinMode(buttonPin, INPUT_PULLUP);       // set arduino pin to input pull-up mode
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, RISING); //interrupt to track button's state
  pinMode(ledPin, OUTPUT);              //set pin 5 as an output that can be set to HIGH or LOW
  pinMode(buzzerPin, OUTPUT);              //set pin 4 as an output that can be set to HIGH or LOW

}

void loop() {

   if (digitalRead(buttonPin) == LOW) {
      Serial.println("Button pressed!");
      delay(600);
   }

  //PERIODICAL SENSOR DATA COLLECTION
  // go thr the array
    //if recieved data == "BUZZ"
    //then BUZZ for 2 seconds
  
    //if recieved data == "LED" then light up

  // 3 button presses

  //ON DEMAND SENSOR DATA COLLECTION  
  if(toReport){
    Serial.println("Entered ISR for button");
    //recieve data
    //display the data from air-quality sensor 
    lcd.setCursor(0, 0);              //set the cursor to the 0,0 position (top left corner)
    lcd.print("Hello, world!");       //print hello, world! starting at that position

    lcd.setCursor(0, 1);              //move the cursor to the first space of the bottom row
    lcd.print(millis() / 1000);       //print the number of seconds that have passed since the last reset
    toReport = false;
  }

}


void buttonInterrupt(){
  toReport = true;
  //Serial.println("button >>> interrupt .. ");
}
