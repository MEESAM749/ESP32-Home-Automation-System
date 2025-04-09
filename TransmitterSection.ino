#include <LoRa.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pin definitions
#define TRIG_PIN 23  // GPIO23
#define ECHO_PIN 22  // GPIO22
#define SS      5    // LoRa NSS
#define RST     14   // LoRa RESET
#define DIO0    26   // LoRa IRQ

// Global variable to store distance (simplest way to share data between tasks)
float currentDistance = 0.0;

// Task for reading from ultrasonic sensor
void ultrasonicSensorTask(void *parameter) {
  // This task will run forever
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

// Task for sending data via LoRa
void loraSendTask(void *parameter) {
  // This task will run forever
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

void setup() {
  // Initialize serial communication
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
  
  // Create the ultrasonic sensor task
  xTaskCreate(
    ultrasonicSensorTask,  // Function that implements the task
    "UltrasonicTask",      // Text name for the task
    2048,                  // Stack size in words
    NULL,                  // Parameter passed to the task
    1,                     // Task priority (1 is low)
    NULL                   // Task handle (not needed here)
  );
  
  // Create the LoRa sending task
  xTaskCreate(
    loraSendTask,          // Function that implements the task
    "LoRaTask",            // Text name for the task
    2048,                  // Stack size in words
    NULL,                  // Parameter passed to the task
    1,                     // Task priority (same as other task)
    NULL                   // Task handle (not needed here)
  );
  
  // RTOS scheduler starts automatically after setup()
  Serial.println("Tasks created and scheduler started!");
}

void loop() {
  // Empty - the tasks are running separately now!
  // This is intentionally left empty as FreeRTOS handles the tasks
  
  // The delay below is not necessary but keeps the loop from 
  // running constantly and wasting CPU cycles
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}