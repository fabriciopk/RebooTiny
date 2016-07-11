#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWD";

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
}
