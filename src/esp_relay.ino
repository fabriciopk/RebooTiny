#include <ESP8266WiFi.h>


const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWD";
const char* host = "www.google.com";

int con_count = 0;
int debug_test = 0;
char enable;
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

  enable = 1;

}

void loop() {
  WiFiClient clientServer = server.available();
  if (!clientServer) {
    //ping
    Serial.println("no server available");
    if (ping_count++ >= 100) {
      if (!clientPing.connect(host, httpPort)) {
        Serial.println("connection failed");
        if (con_count++ >= 10 ) {
          Serial.print("contagem: "); Serial.println(con_count);
          if (enable) {
            user_log += "<br>restarting";
            WiFi.disconnect();
            reiniciaTomada();
            con_count = 0;
            ping_count = 0;
            WiFi.begin(ssid, password);

            while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print(":");

              if (con_count++ >= 40) {
                con_count = 0;
                reiniciaTomada();
              }
            }
            con_count = 0;
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
  buf += "\"} ,15000);}</script>";
  buf += "<h3> ESP8266 Web Server</h3><br><a href=\"?debug=ok\">Debug Inc</a><br><a href=\"?enable=toggle\">Toggle Enable</a><br>";
  buf += "<p>GPIO2<form method=\"get\"> <input type=\"submit\" name=\"reboot\" value=\"Reboot\" />";

  if (req.indexOf("debug=ok") != -1) {
    debug_test++;
  }
  if (req.indexOf("enable=toggle") != -1) {
    enable = enable ^ 1; // Toggle enable
  }
  if (req.indexOf("log=reset") != -1) {
    user_log = "WiFi connected<br>IP: ";
    user_log += WiFi.localIP().toString();
  }

  if (req.indexOf("reboot") != -1) {
    buf += "<p>Will restart<br>The page will refresh automatically</html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
    delay(1000);
    con_count = 0;
    ping_count = 0;

    WiFi.disconnect();
    reiniciaTomada();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(":");

      if (con_count++ >= 40) {
        con_count = 0;
        reiniciaTomada();
      }
    }

  } else if (req.indexOf("log=ok") != -1) {
    buf += "<br><a href=\"http://";
    buf += WiFi.localIP().toString();
    buf += "\">Hide Log</a></form>";
    buf += "<br><hr><a href=\"?log=reset\">Reset Log</a><br>count: ";
    buf += con_count;
    buf += "<br>";
    buf += (enable == 1) ? "Enabled" : "Disabled";
    buf += "<br>";
    buf += user_log;
    buf += "<br>debug test count:";
    buf += debug_test;
    buf += "<hr></html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
  }
  else {
    buf += "<br><a href=\"?log=ok\">Show Log</a></form>";
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
