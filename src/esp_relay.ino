#include <ESP8266WiFi.h>


const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWD";
const char* host = "www.google.com";

int con_count = 0;
String buf;
String user_log;

int ping_count = 0;
int last_ping = 0;

WiFiServer server(80);
WiFiClient clientPing;
const int httpPort = 80;

void setup() {
  Serial.begin(9600);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  delay(5000);


  // Connect to WiFi network

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  user_log = "WiFi connected<br>IP: ";

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  user_log += WiFi.localIP().toString();

}

void loop() {
  WiFiClient clientServer = server.available();
  if (!clientServer) {
    //ping
    Serial.println("no server available");
    if (ping_count++ >= 50) {
      if (!clientPing.connect(host, httpPort)) {
        Serial.println("connection failed");
        //user_log += "<p>ping failed: count=";
        //user_log += ++con_count;
        if (con_count++ >= 10 ) {
          Serial.print("contagem: "); Serial.println(con_count);
          user_log += "<br>restarting";
          WiFi.disconnect();
          reiniciaTomada();
          con_count = 0;
          WiFi.begin(ssid, password);

          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(":");
          }
        }
      } else {
        Serial.println("ping success");
        con_count = 0;
        ping_count = 0;
      }
    }
    delay(200);
    return;
  }

  Serial.println("new client");
  while (!clientServer.available()) {
    Serial.println("no client available");
  }

  String req = clientServer.readStringUntil('\r');
  Serial.println(req);
  clientServer.flush();

  buf = "";

  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  buf += "<script>if (window.location.toString().includes(\"reboot\")){setTimeout(function(){window.location=\"http://";
  buf += WiFi.localIP().toString();
  buf += "\"} ,15000);};</script>";
  buf += "<h3> ESP8266 Web Server</h3><p><a href=\"?debug=ok\">Debug Inc</a></form><p>";
  buf += "<p>GPIO2<form method=\"get\"> <input type=\"submit\" name=\"reboot\" value=\"Reboot\" />";

  if (req.indexOf("reboot") != -1) {
    buf += "<p>Will restart<br>The page will refresh automatically</html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
    delay(1000);
    con_count = 0;

    WiFi.disconnect();
    reiniciaTomada();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(":");
    }

  } else if (req.indexOf("log=ok") != -1) {
    buf += "<a href=\"http://";
    buf += WiFi.localIP().toString();
    buf += "\">Hide Log</a></form>";
    buf += "<br><hr>count: ";
    buf += con_count;
    buf += "<br>";
    buf += user_log;
    buf += "<hr></html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
  }
  else {
    buf += "<a href=\"?log=ok\">Show Log</a></form>";
    buf += "<p>Ready</html>";
    clientServer.print(buf);
    clientServer.flush();
    Serial.println("invalid request");
    clientServer.stop();
  }
  Serial.println("Client disonnected");
}

void reiniciaTomada() {
  Serial.println("Reinicia tomada");
  digitalWrite(2, 1);
  delay(7000);
  digitalWrite(2, 0);
}
