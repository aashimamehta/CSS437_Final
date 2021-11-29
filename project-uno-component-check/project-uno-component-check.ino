/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#include "Adafruit_PM25AQI.h"

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
int photoresistor = 0;              //this variable will hold a value based on the brightness of the ambient light
int threshold = 500;                //if the photoresistor reading is below this value the the light will turn on

// Define pins for ultrasonic and buzzer
int const trigPin = 3;
int const echoPin = 2;

void setup() {
  // Wait for serial monitor to open
  Serial.begin(9600);
  while (!Serial) delay(10);
  Serial.println("Adafruit PMSA003I Air Quality Sensor");
  delay(1000);
  
  pinMode(trigPin, OUTPUT);        // trig pin will have pulses output
  pinMode(echoPin, INPUT);         // echo pin should be input to get pulse width

}
void loop() {
  PM25_AQI_Data data;
  if (! aqi.read(&data)) {
    delay(500); // try again in a bit!
  }
  Serial.println("AQI reading success");
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  delay(1000);

  //read the brightness of the ambient light
  Serial.println("Reading data for photoresistor now");
  photoresistor = analogRead(A0);   //set photoresistor to a number between 0 and 1023 based on how bright the ambient light is

  //if the photoresistor value is below the threshold turn the light on, otherwise turn it off
  if (photoresistor < threshold) {
    Serial.println("Its dark, therefore turning the LED on");
    //digitalWrite(4, HIGH);         // Turn on the LED
  } else {
    Serial.println("Its bright, therefore turning the LED off");
    //digitalWrite(4, LOW);          // Turn off the LED
  }

  delay(1000);                       //short delay to make the printout easier to read

  // Duration will be the input pulse width and distance will be the distance to the obstacle in centimeters
  int duration, distance;
  Serial.println("Reading data from ultrasonic sensor now");
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
    Serial.println("Something is nearby");
    // Calculating the distance
    distance= duration*0.034/2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance from the object = ");
    Serial.print(distance);
    Serial.println(" cm");
  } 
  else {
    Serial.println("Nothing is nearby");
  }

  delay(1000);
}
