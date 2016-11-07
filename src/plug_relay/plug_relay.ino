#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#define httpPort 80
#define ssid "YOUR_SSID"
#define password "YOUR_PASSWORD"
#define host "google.com"

String buf, req;

WiFiServer server(80);
WiFiClient clientServer;
WiFiClient clientPing;

uint8_t status;

void turn_relay(int s);

void setup() {
  pinMode(2, OUTPUT);
  turn_relay(0); // 0 = on
  status = 0;

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Start the server
  server.begin();

  ArduinoOTA.begin();
}

void loop() {
  while (! (clientServer = server.available())) {
    delay(200);
    ArduinoOTA.handle();
  }

  req = clientServer.readStringUntil('\r');
  clientServer.flush();

  if (req.indexOf("rest/get") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
    buf += "{\"enabled\":";
    buf += status;
    buf += ",\"version\":0.1}";

    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("rest/post/toggle") != -1) {

    buf = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
    buf += "{\"success\":true}";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
    delay(200);
    status = status ^1;
    turn_relay(status);
  }
}

void turn_relay(int s) {
  digitalWrite(2, s);
}

