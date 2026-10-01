// Claude Code traffic light.
// Takes a color (red, yellow, green, or off) as a line of text over USB serial.
// If WiFi is set in secrets.h, it also serves http://claude-light.local/set?state=<color>.

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "secrets.h"  // defines WIFI_SSID and WIFI_PASSWORD (leave SSID "" for USB only)

// The Open-Smart module's LEDs have built-in resistors and are active-high.
const int PIN_RED = 11;
const int PIN_YELLOW = 12;
const int PIN_GREEN = 13;

WebServer server(80);
String currentState = "off";
bool serverStarted = false;

bool isValidState(const String& state) {
  return state == "red" || state == "yellow" || state == "green" || state == "off";
}

void setLight(const String& state) {
  digitalWrite(PIN_RED, state == "red");
  digitalWrite(PIN_YELLOW, state == "yellow");
  digitalWrite(PIN_GREEN, state == "green");
  currentState = state;
}

void handleSet() {
  String state = server.arg("state");
  if (!isValidState(state)) {
    server.send(400, "text/plain", "state must be red, yellow, green, or off\n");
    return;
  }
  setLight(state);
  server.send(200, "text/plain", state + "\n");
}

// S3 boards have two USB ports: one is the chip's own USB (Serial), the other a
// USB-to-serial chip wired to UART0 (Serial0). Listen on both so either cable works.
void readCommands(Stream& port, String& buffer) {
  while (port.available()) {
    char c = port.read();
    if (c == '\n' || c == '\r') {
      buffer.trim();
      if (isValidState(buffer)) {
        setLight(buffer);
        port.println(buffer);
      }
      buffer = "";
    } else if (buffer.length() < 16) {
      buffer += c;
    }
  }
}

String usbBuffer;
String uartBuffer;

void setup() {
  Serial.begin(115200);
  Serial0.begin(115200);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Quick red-yellow-green flash so you can tell it booted.
  setLight("red"); delay(200);
  setLight("yellow"); delay(200);
  setLight("green");

  if (strlen(WIFI_SSID) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // connects in the background
  }
}

void loop() {
  readCommands(Serial, usbBuffer);
  readCommands(Serial0, uartBuffer);

  if (WiFi.status() == WL_CONNECTED) {
    if (!serverStarted) {
      MDNS.begin("claude-light");
      MDNS.addService("http", "tcp", 80);
      server.on("/set", handleSet);
      server.on("/", []() { server.send(200, "text/plain", currentState + "\n"); });
      server.begin();
      serverStarted = true;
      Serial.printf("WiFi connected, IP %s\n", WiFi.localIP().toString().c_str());
    }
    server.handleClient();
  }
}
