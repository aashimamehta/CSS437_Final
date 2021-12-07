/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math. HRS

//#include "Adafruit_PM25AQI.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <PulseSensorPlayground.h>
#include <math.h>         //loads the more advanced math functions


// struct for data transfer between devices
struct SensorData {
  float photoData;
  int BPM;
  float temperatureData;
  float USDistance_Front;
  float USDistance_Left;
  float USDistance_Right;
};

  
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";

int photoresistor = 0;              //this variable will hold a value based on the brightness of the ambient light
int threshold = 500;                //if the photoresistor reading is below this value the the light will turn on

// Define pins for ultrasonic and buzzer
int const trigPin1 = 2;
int const echoPin1 = 3;

int const trigPin2 = 5;
int const echoPin2 = 4;

int const trigPin3 = 6;
int const echoPin3 = 7;

SensorData sensorData;
bool requestData = false;
unsigned long lastCheckTime = 0; 
unsigned long lastHeartBeat;


PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

//  Variables for Pulse 

const int PulseWire = 1;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 1
int Threshold = 348;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 

void setup() {
  // Wait for serial monitor to open
  Serial.begin(9600);
  
  pinMode(trigPin1, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin1, INPUT);         // echo pin should be input to get pulse width
  pinMode(trigPin2, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin2, INPUT);         // echo pin should be input to get pulse width
  pinMode(trigPin3, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin3, INPUT);         // echo pin should be input to get pulse width
  pinMode(LED_BUILTIN,OUTPUT);  // Built-in LED will blink to your heartbeat
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.setThreshold(Threshold);  
  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }

  radio.begin();
  radio.openWritingPipe(address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();
}

void loop() {
  getHeartRate();
  if (requestData) {
    collectSensorData();
    if (!radio.write(&sensorData, sizeof(SensorData))) {
      Serial.println("failed to send data to mega...");
    } else {
      Serial.println("data sent.");
    }

    Serial.print("Photoresistor: ");
    Serial.println(sensorData.photoData);
    Serial.print("BPM : ");
    Serial.println(sensorData.BPM);
    Serial.print("Temperature: ");
    Serial.println(sensorData.temperatureData);
    Serial.print("Distance: ");
    Serial.print(sensorData.USDistance_Front);
    Serial.print(", ");
    Serial.print(sensorData.USDistance_Left);
    Serial.print(", ");
    Serial.println(sensorData.USDistance_Right);

    requestData = false;
  } else {
    unsigned long currentTime = millis();
    // if it's been 1000ms since last checked sensors
    if ((currentTime - lastCheckTime) >= 2000) {
      requestData = true;
      lastCheckTime = currentTime;
    }
  }
  // if receiving data request from centural device
  if (radio.available()) {
    byte recv;
    radio.read(&recv, sizeof(byte));
    requestData = true;
  }
}

void collectSensorData() {
  getPhotoresistor();
  getTemperature();
  
  getUltrasonic(trigPin1, echoPin1, 1);
  getUltrasonic(trigPin2, echoPin2, 2);
  getUltrasonic(trigPin3, echoPin3, 3);
}

void getPhotoresistor() {
  photoresistor = analogRead(A0);   //set photoresistor to a number between 0 and 1023 based on how bright the ambient light is
  sensorData.photoData = photoresistor;
}


void getTemperature() {
  float voltage = analogRead(A2) * 0.004882813;   //convert the analog reading, which varies from 0 to 1023, back to a voltage value from 0-5 volts
  float degreesC = (voltage - 0.5) * 100.0;       //convert the voltage to a temperature in degrees Celsius
  float degreesF = degreesC * (9.0 / 5.0) + 32.0; //convert the voltage to a temperature in degrees Fahrenheit
  Serial.print("this is the degreeF");
  Serial.println(degreesF);
  sensorData.temperatureData = degreesF;
}


void getUltrasonic(int trigPin, int echoPin, int ultrasonicNum) {
  // Duration will be the input pulse width and distance will be the distance to the obstacle in centimeters
  int duration, distance;
  digitalWrite(trigPin, HIGH);
  delay(5);
  digitalWrite(trigPin, LOW);
  
  // Measure the pulse input in echo pin
  duration = pulseIn(echoPin, HIGH);
  // Distance is half the duration divided by 29.1 (from datasheet)
  distance = (duration/2) / 29.1;
  distance = duration*0.034/2;

  // record to the correct entry
  if (ultrasonicNum == 1) {
    sensorData.USDistance_Front = distance;
  } else if (ultrasonicNum == 2) {
    sensorData.USDistance_Left = distance;
  } else {
    sensorData.USDistance_Right = distance;
  }
}

void getHeartRate(){
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
  sensorData.BPM = myBPM;                       // "myBPM" hold this BPM value now. 
  if (pulseSensor.sawStartOfBeat()) { 
   sensorData.BPM = myBPM;                       // Print the value inside of myBPM.
  }
}

/*void getHeartRate() {
  int Signal = analogRead(PulseWire);
  //Serial.println(Signal);
  if (Signal > Threshold) {
//    if (lastHeartBeat != 0) {
//      long hbGap = millis() - lastHeartBeat;
//      if (hbGap < 1200 && hbGap > 300) {
//        sensorData.BPM = 60000 / hbGap;
//        lastHeartBeat = millis();
//      }
      Serial.println(Signal);
    } else {
      lastHeartBeat = millis();
    }
  }
}*/
