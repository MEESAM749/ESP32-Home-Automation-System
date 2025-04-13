#include <LoRa.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



//-------------------------------------------------------------------
//-------------------------------------------------------------------



#define TRIG_PIN 23  
#define ECHO_PIN 22  
#define SS      5    
#define RST     14   
#define DIO0    26   


// Global variable to store distance
float currentDistance = 0.0;



//-------------------------------------------------------------------
//-------------------------------------------------------------------



// Task for reading from ultrasonic sensor
void ultrasonicSensorTask(void *parameter) {
  while (1) {
    // --- START ULTRASONIC MEASUREMENT ---
    // 1. Clear the trigger pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    
    // 2. Set the trigger pin HIGH for 10 microseconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // 3. Read the echo pin to get the pulse duration
    long duration = pulseIn(ECHO_PIN, HIGH);
    
    // 4. Calculate distance in cm
    currentDistance = duration * 0.0343 / 2;
    
    // 5. Print to serial monitor
    Serial.print("Measured Distance: ");
    Serial.print(currentDistance);
    Serial.println(" cm");
    
    // 6. Wait a bit before next measurement (500ms)
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------



// Task for sending data via LoRa
void loraSendTask(void *parameter) {
  while (1) {
    // 1. Send the current distance reading via LoRa
    LoRa.beginPacket();
    LoRa.print("Distance: ");
    LoRa.print(currentDistance);  
    LoRa.println(" cm");
    LoRa.endPacket();
    // 2. Print confirmation to serial
    Serial.println("LoRa packet sent!");
    // 3. Wait a bit before next transmission (1 second)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------



void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  
  // Set up ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Set up LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {  
    Serial.println("LoRa init failed. Check wiring.");
    while (1);  // If LoRa fails, stop here
  }
  Serial.println("LoRa initialized successfully!");
  
  //Ultrasonic sensor task
  xTaskCreate(
    ultrasonicSensorTask, 
    "UltrasonicTask",      
    2048,                  
    NULL,                  
    1,                     
    NULL                   
  );
  xTaskCreate(
    loraSendTask,          
    "LoRaTask",            
    2048,                  
    NULL,                  
    1,                     
    NULL                   
  );
  Serial.println("Tasks created and scheduler started!");
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------



void loop() {
  // The delay below is not necessary but keeps the loop from running constantly and wasting CPU cycles
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------