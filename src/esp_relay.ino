#include <ESP8266WiFi.h>


const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWD";
const char* host = "www.google.com";

int con_count = 0;
String buf;
String user_log;

WiFiServer server(80);
WiFiClient client;
const int httpPort = 80;

void setup() {
  Serial.begin(115200);
  delay(5000);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
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

  user_log = "WiFi connected<p>IP: ";

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  user_log += WiFi.localIP().toString();
}

void loop() {

  WiFiClient client = server.available();
  if (!client) {
    //ping
    Serial.println("no server available");
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      con_count++;
      user_log += "<p>ping failed: count=";
      user_log += con_count;
      if(con_count > 10 ){
        Serial.print("contagem: "); Serial.println(con_count);
        WiFi.disconnect();
        reiniciaTomada();
        con_count = 0;
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(":");
        }
      }
      return;
    } else{
      Serial.println("ping success");
      con_count = 0;
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

  buf = "";

  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  buf += "<script>if (window.location.toString().includes(\"reboot\")){setTimeout(function(){window.location=\"http://";
  buf += WiFi.localIP().toString();
  buf += "\"} ,15000);};</script>";
  buf += "<h3> ESP8266 Web Server</h3>";
  buf += "<p>GPIO2<form method=\"get\"> <input type=\"submit\" name=\"reboot\" value=\"Reboot\" />";

// <a href=\"?function=reboot\">    </a>
  //client.print(buf);

  if (req.indexOf("reboot") != -1){
    buf += "<p>Will restart</html>";
    client.print(buf);
    client.flush();
    client.stop();
    delay(1000);
    con_count = 0;

    WiFi.disconnect();
    reiniciaTomada();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(":");
    }

  } else if (req.indexOf("log") != -1){
    buf += "<a href=\"http://";
    buf += WiFi.localIP().toString();
    buf += "\">Hide Log</a></form>";
    buf += "<p><hr><p>count: ";
    buf += con_count;
    buf += "<p>";
    buf += user_log;
    buf += "<hr></html>";
    client.print(buf);
    client.flush();
    client.stop();
  }
  else {
    buf += "<a href=\"?log=ok\">Show Log</a></form>";
    buf += "<p>Ready</html>";
    client.print(buf);
    client.flush();
    Serial.println("invalid request");
    client.stop();
  }
  Serial.println("Client disonnected");
}

void reiniciaTomada(){
  Serial.println("Reinicia tomada");
  digitalWrite(2, 1);
  delay(7000);
  digitalWrite(2, 0);
}
