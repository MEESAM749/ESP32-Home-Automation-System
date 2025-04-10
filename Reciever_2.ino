#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>



//-------------------------------------------------------------------
//-------------------------------------------------------------------



// Pin definitions
#define SS      5
#define RST     14
#define DIO0    26
#define RELAY_PIN 21 



//-------------------------------------------------------------------
//-------------------------------------------------------------------


// WiFi creds
const char* ssid = "1122";
const char* password = "pakistan11";

// Web server
WebServer server(80);

// Globals
String latestLoRaMessage = "No data yet";
bool relayState = false;

// Webpage:
const char* htmlTemplate = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 LoRa Receiver</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; padding-top: 30px; }
    .card { background: #f4f4f4; padding: 20px; margin: auto; width: 80%%; max-width: 400px; box-shadow: 2px 2px 10px rgba(0,0,0,0.2); }
    .button { padding: 10px 20px; font-size: 20px; margin: 10px; cursor: pointer; border: none; border-radius: 5px; color: #fff; }
    .on { background-color: #28a745; }
    .off { background-color: #dc3545; }
  </style>
  <script>
    setInterval(() => {
      fetch("/distance").then(res => res.text()).then(data => {
        document.getElementById("distance").innerText = data;
      });
    }, 2000);

    function toggleRelay(state) {
      fetch("/relay?state=" + state);
    }
  </script>
</head>
<body>
  <h1>LoRa Distance Monitor</h1>
  <div class="card">
    <h2>Distance</h2>
    <p id="distance">%DISTANCE%</p>
  </div>
  <div class="card">
    <h2>Relay Control</h2>
    <p>Current State: %RELAY_STATE%</p>
    <button class="button on" onclick="toggleRelay(1)">Turn ON</button>
    <button class="button off" onclick="toggleRelay(0)">Turn OFF</button>
  </div>
</body>
</html>
)rawliteral";



//-------------------------------------------------------------------
//-------------------------------------------------------------------



// Web server task
void webServerTask(void* param) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    String page = htmlTemplate;
    page.replace("%DISTANCE%", latestLoRaMessage);
    page.replace("%RELAY_STATE%", relayState ? "ON" : "OFF");
    server.send(200, "text/html", page);
  });

  server.on("/distance", HTTP_GET, []() {
    server.send(200, "text/plain", latestLoRaMessage);
  });

  server.on("/relay", HTTP_GET, []() {
    if (server.hasArg("state")) {
      String state = server.arg("state");
      if (state == "1") {
        digitalWrite(RELAY_PIN, LOW);  // Active LOW
        relayState = true;
      } else {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = false;
      }
      Serial.print("Relay set to: ");
      Serial.println(relayState ? "ON" : "OFF");
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("Web server started.");

  while (1) {
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------



// LoRa receiver task
void loraReceiveTask(void* param) {
  while (1) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String message = "";
      while (LoRa.available()) {
        message += (char)LoRa.read();
      }
      latestLoRaMessage = message;
      Serial.print("Received message: ");
      Serial.println(message);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------



void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF by default

  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (true); // Die here
  }
  Serial.println("LoRa initialized!");

  xTaskCreate(webServerTask, "WebServer", 4096, NULL, 1, NULL);
  xTaskCreate(loraReceiveTask, "LoRaReceive", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Nothing to do here
}
