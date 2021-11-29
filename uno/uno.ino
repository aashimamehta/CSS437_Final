/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#include "Adafruit_PM25AQI.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// struct for data transfer between devices
struct SensorData {
    int AQIData;
    float photoData;
    float USDistance;
};
  
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
int photoresistor = 0;              //this variable will hold a value based on the brightness of the ambient light
int threshold = 500;                //if the photoresistor reading is below this value the the light will turn on

// Define pins for ultrasonic and buzzer
int const trigPin = 3;
int const echoPin = 2;

SensorData sensorData;
bool requestData = false;
unsigned long lastCheckTime = 0; 

void setup() {
  // Wait for serial monitor to open
  Serial.begin(9600);
  while (!Serial) delay(10);
  Serial.println("Adafruit PMSA003I Air Quality Sensor");
  delay(1000);
  
  pinMode(trigPin, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin, INPUT);         // echo pin should be input to get pulse width

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
    Serial.print("Distance: ");
    Serial.println(sensorData.USDistance);

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
  getUltrasonic();
}

void getAQISensor() {
  PM25_AQI_Data aqidata;
  //Serial.println("AQI reading success");
  //Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(aqidata.particles_25um);
  sensorData.AQIData = aqidata.particles_25um;
  
}

void getPhotoresistor() {
  //read the brightness of the ambient light
  //Serial.println("Reading data for photoresistor now");
  photoresistor = analogRead(A0);   //set photoresistor to a number between 0 and 1023 based on how bright the ambient light is

  //if the photoresistor value is below the threshold turn the light on, otherwise turn it off
  if (photoresistor < threshold) {
    //Serial.println("Its dark, therefore turning the LED on");
    //sensorData.isDark = true;
    //digitalWrite(4, HIGH);         // Turn on the LED
  } else {
    //Serial.println("Its bright, therefore turning the LED off");
    //sensorData.isDark = false;
    //digitalWrite(4, LOW);          // Turn off the LED
  }
  sensorData.photoData = photoresistor;
}

void getUltrasonic() {
  // Duration will be the input pulse width and distance will be the distance to the obstacle in centimeters
  int duration, distance;
  //Serial.println("Reading data from ultrasonic sensor now");
  // Output pulse with 1ms width on trigPin
  digitalWrite(trigPin, HIGH);
  delay(5);
  digitalWrite(trigPin, LOW);
  
  // Measure the pulse input in echo pin
  duration = pulseIn(echoPin, HIGH);
  // Distance is half the duration divided by 29.1 (from datasheet)
  distance = (duration/2) / 29.1;
  
  // if distance less than 0.5 meter and more than 0 (0 or less means over range)
  if (distance <= 10 && distance >= 0) {
    distance = duration*0.034/2;
    sensorData.USDistance = distance;
  } 
  else {
    //Serial.println("Nothing is nearby");
    sensorData.USDistance = distance;
  }
}
