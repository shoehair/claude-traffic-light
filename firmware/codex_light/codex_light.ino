// Claude Code + Codex traffic light (USB only).
// Takes a color (red, yellow, green, or off) as a line of text over USB serial.

// The Open-Smart module's LEDs have built-in resistors and are active-high.
const int PIN_RED = 11;
const int PIN_YELLOW = 12;
const int PIN_GREEN = 13;

bool isValidState(const String& state) {
  return state == "red" || state == "yellow" || state == "green" || state == "off";
}

void setLight(const String& state) {
  digitalWrite(PIN_RED, state == "red");
  digitalWrite(PIN_YELLOW, state == "yellow");
  digitalWrite(PIN_GREEN, state == "green");
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
}

void loop() {
  readCommands(Serial, usbBuffer);
  readCommands(Serial0, uartBuffer);
}
