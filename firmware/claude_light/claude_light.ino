// Claude Code traffic light.
// Joins WiFi, advertises itself as http://claude-light.local, and sets the
// light from GET /set?state=red|yellow|green|off.

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "secrets.h"  // defines WIFI_SSID and WIFI_PASSWORD

// The Open-Smart module's LEDs have built-in resistors and are active-high.
const int PIN_RED = 25;
const int PIN_YELLOW = 26;
const int PIN_GREEN = 27;

WebServer server(80);
String currentState = "off";

void setLight(const String& state) {
  digitalWrite(PIN_RED, state == "red");
  digitalWrite(PIN_YELLOW, state == "yellow");
  digitalWrite(PIN_GREEN, state == "green");
  currentState = state;
}

void handleSet() {
  String state = server.arg("state");
  if (state != "red" && state != "yellow" && state != "green" && state != "off") {
    server.send(400, "text/plain", "state must be red, yellow, green, or off\n");
    return;
  }
  setLight(state);
  server.send(200, "text/plain", state + "\n");
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  // Cycle the lights while connecting so you can tell it's alive.
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    const char* cycle[] = {"red", "yellow", "green"};
    setLight(cycle[i++ % 3]);
    Serial.print(".");
    delay(300);
  }
  Serial.printf("\nConnected, IP %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  connectWifi();
  if (MDNS.begin("claude-light")) {
    MDNS.addService("http", "tcp", 80);
  }

  server.on("/set", handleSet);
  server.on("/", []() { server.send(200, "text/plain", currentState + "\n"); });
  server.begin();
  setLight("green");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
    setLight("green");
  }
  server.handleClient();
}
