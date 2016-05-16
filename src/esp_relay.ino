#include <ESP8266WiFi.h>


const char* ssid = "AP-PREDIO320";
const char* password = "naocompilavei";
const char* host = "www.adafruit.com";

int con_count = 0;

WiFiServer server(80);
WiFiClient client;
const int httpPort = 80;

void setup() {
  Serial.begin(115200);
  delay(100);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

}

void loop() {

  WiFiClient client = server.available();
  if (!client) {
    //ping
    Serial.println("no server available");
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      con_count++;
      if(con_count > 50 ){
        digitalWrite(2, 1);
        delay(500);
        digitalWrite(2, 0);
        con_count = 0;
      }
      return;
    }
    return;
  }

  Serial.println("new client");
  while(!client.available()){
    Serial.println("no client available");
  }

  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  String buf = "";

  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  buf += "<h3> ESP8266 Web Server</h3>";
  buf += "<p>GPIO4<form method=\"get\"> <input type=\"submit\" name=\"reboot\" value=\"Reboot\" /> </form> </p>";
  buf += "</html>\n";
// <a href=\"?function=reboot\">    </a>
  client.print(buf);
  client.flush();

  if (req.indexOf("reboot") != -1){
    Serial.println("Desliga tomada");
    digitalWrite(2, 1);
    delay(5000);
    Serial.println("Liga tomada");
    digitalWrite(2, 0);
  }
  else {
    Serial.println("invalid request");
    client.stop();
  }
  Serial.println("Client disonnected");
}
