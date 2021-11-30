/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math. HRS

#include "Adafruit_PM25AQI.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <PulseSensorPlayground.h> 


// struct for data transfer between devices
struct SensorData {
    int AQIData;
    float photoData;
    float USDistance1;
    float USDistance2;
    float USDistance3;
    int BPM;
};
  
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
int photoresistor = 0;              //this variable will hold a value based on the brightness of the ambient light
int threshold = 500;                //if the photoresistor reading is below this value the the light will turn on

// Define pins for ultrasonic and buzzer
int const trigPin1 = 3;
int const echoPin1 = 2;

int const trigPin2 = 5;
int const echoPin2 = 4;

int const trigPin3 = 7;
int const echoPin3 = 6;

SensorData sensorData;
bool requestData = false;
unsigned long lastCheckTime = 0; 

int const PULSE_SENSOR_PIN = 1;   // 'S' Signal pin connected to A0
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

//  Variables for Pulse 
const int PulseWire = 1;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 

void setup() {
  // Wait for serial monitor to open
  Serial.begin(9600);
  while (!Serial) delay(10);
  Serial.println("Adafruit PMSA003I Air Quality Sensor");
  delay(1000);
  
  pinMode(trigPin1, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin1, INPUT);         // echo pin should be input to get pulse width
  pinMode(trigPin2, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin2, INPUT);         // echo pin should be input to get pulse width
  pinMode(trigPin3, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin3, INPUT);         // echo pin should be input to get pulse width
  
  pinMode(LED_BUILTIN,OUTPUT);  // Built-in LED will blink to your heartbeat
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.setThreshold(Threshold);  

  radio.begin();
  radio.openWritingPipe(address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();
}

void loop() {
  if (requestData) {
    //int tempint = 5;
    collectSensorData();
    if (!radio.write(&sensorData, sizeof(SensorData))) {
    //if (!radio.write(&tempint, sizeof(SensorData))) {
      Serial.print("failed");
    } else {
          Serial.println("data sent");
    }
    Serial.print("PM2.5: ");
    Serial.println(sensorData.AQIData);
    Serial.print("Photoresistor: ");
    Serial.println(sensorData.photoData);
    Serial.print("Distance 1: ");
    Serial.println(sensorData.USDistance1);
    Serial.print("Distance 2: ");
    Serial.println(sensorData.USDistance2);
    Serial.print("Distance 3: ");
    Serial.println(sensorData.USDistance3);
    Serial.print("BPM : ");
    Serial.println(sensorData.BPM);

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
  getAQISensor();
  getPhotoresistor();
  getUltrasonic(trigPin1, echoPin1, 1);
  getUltrasonic(trigPin2, echoPin2, 2);
  getUltrasonic(trigPin3, echoPin3, 3);
  getHeartRate();
}

void getAQISensor() {
  PM25_AQI_Data aqidata;
  //Serial.println("AQI reading success");
  //Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(aqidata.particles_25um);
  sensorData.AQIData = aqidata.particles_25um;
  
}

void getPhotoresistor() {
  photoresistor = analogRead(A0);   //set photoresistor to a number between 0 and 1023 based on how bright the ambient light is
  sensorData.photoData = photoresistor;
}

void getUltrasonic(trigPin, echoPin, ultrasonicNum) {
  // Duration will be the input pulse width and distance will be the distance to the obstacle in centimeters
  int duration, distance;
  digitalWrite(trigPin, HIGH);
  delay(5);
  digitalWrite(trigPin, LOW);
  
  // Measure the pulse input in echo pin
  duration = pulseIn(echoPin, HIGH);
  // Distance is half the duration divided by 29.1 (from datasheet)
  distance = (duration/2) / 29.1;
  
  // if distance less than 0.5 meter and more than 0 (0 or less means over range)
  if(ultrasonicNum == 1){
    if (distance <= 10 && distance >= 0) {
      distance = duration*0.034/2;
      sensorData.USDistance1 = distance;
    } 
    else {
      sensorData.USDistance1 = distance;
    }
  }
  else if(ultrasonicNum == 2){
    if (distance <= 10 && distance >= 0) {
      distance = duration*0.034/2;
      sensorData.USDistance2 = distance;
    } 
    else {
      sensorData.USDistance2 = distance;
    }
  }
  else{
    if (distance <= 10 && distance >= 0) {
      distance = duration*0.034/2;
      sensorData.USDistance3 = distance;
    } 
    else {
      sensorData.USDistance3 = distance;
    }
  }
  
  
}

void getHeartRate(){
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
  if (pulseSensor.sawStartOfBeat()) { 
   sensorData.BPM = myBPM;                       // Print the value inside of myBPM.
  }
  delay(500);                    // considered best practice in a simple sketch.
}
