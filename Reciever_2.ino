#include <LoRa.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>
#include <WebServer.h>

// Pin definitions
#define TRIG_PIN 23  // GPIO23
#define ECHO_PIN 22  // GPIO22
#define SS      5    // LoRa NSS
#define RST     14   // LoRa RESET
#define DIO0    26   // LoRa IRQ
#define RELAY_PIN 21 // GPIO21 for relay control

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Create web server
WebServer server(80);

// Global variables
float currentDistance = 0.0;
bool relayState = false;

// HTML for the web page
const char* htmlTemplate = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Distance Monitor & Relay Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; }
    .button { padding: 10px 20px; font-size: 24px; margin: 10px; cursor: pointer; 
              border-radius: 5px; color: white; }
    .on { background-color: #4CAF50; }
    .off { background-color: #D11D53; }
    .card { background-color: #F8F7F9; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); 
            padding: 20px; margin: 10px auto; width: 80%%; max-width: 400px; }
  </style>
  <script>
    setInterval(function() {
      getData();
    }, 2000);
    
    function getData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("distance").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/distance", true);
      xhttp.send();
    }
    
    function toggleRelay(state) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/relay?state=" + state, true);
      xhttp.send();
    }
  </script>
</head>
<body>
  <h1>ESP32 Control Panel</h1>
  <div class="card">
    <h2>Distance Sensor</h2>
    <p id="distance">%DISTANCE%</p>
  </div>
  <div class="card">
    <h2>Relay Control</h2>
    <p>Current State: %RELAY_STATE%</p>
    <button class="button on" onclick="toggleRelay(1)">ON</button>
    <button class="button off" onclick="toggleRelay(0)">OFF</button>
  </div>
</body>
</html>
)rawliteral";

// Task for web server handling
void webServerTask(void *parameter) {
  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, []() {
    String html = String(htmlTemplate);
    html.replace("%DISTANCE%", String(currentDistance) + " cm");
    html.replace("%RELAY_STATE%", relayState ? "ON" : "OFF");
    server.send(200, "text/html", html);
  });

  // Route for getting just the distance data
  server.on("/distance", HTTP_GET, []() {
    server.send(200, "text/plain", String(currentDistance) + " cm");
  });

  // Route for controlling the relay
  server.on("/relay", HTTP_GET, []() {
    if (server.hasArg("state")) {
      String stateValue = server.arg("state");
      
      if (stateValue == "1") {
        digitalWrite(RELAY_PIN, LOW);  // Active LOW to turn ON
        relayState = true;
      } else {
        digitalWrite(RELAY_PIN, HIGH); // HIGH to turn OFF
        relayState = false;
      }
      
      Serial.print("Relay state changed to: ");
      Serial.println(relayState ? "ON" : "OFF");
    }
    server.send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Keep handling web requests
  while (1) {
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

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
  
  // Set up relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Initialize relay as OFF (active LOW)
 
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
  
  // Create the web server task (larger stack since it handles HTML)
  xTaskCreate(
    webServerTask,         // Function that implements the task
    "WebServerTask",       // Text name for the task
    4096,                  // Stack size in words (larger for web server)
    NULL,                  // Parameter passed to the task
    1,                     // Task priority
    NULL                   // Task handle
  );
 
  // RTOS scheduler starts automatically after setup()
  Serial.println("All tasks created and scheduler started!");
}

void loop() {
  // Empty - the tasks are running separately now!
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}