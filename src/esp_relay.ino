#include <ESP8266WiFi.h>

const char* ssid = “YOUR_SSID”;
const char* password = “YOUR_PASSWD”;
const char* host = "www.google.com";

int ping_time = 15; // Pins each ping_time seconds
int failure_time = 15; // Seconds without internet to the system reboot
int reboot_time = 60; // Seconds tolerated after each reboot

String buf, req;
String userLog;
int debug_inc, restarts;
char enabled, rebooted;
unsigned long time_count;

WiFiServer server(80);
WiFiClient clientServer;
WiFiClient clientPing;
const int httpPort = 80;

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  debug_inc = 0;
  enabled = 1;
  restarts = 0;
  rebooted = 0;

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  time_count = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    if (millis() - time_count >= reboot_time * 1000) {
      reboot();
      time_count = millis();
    }
    delay(500);
  }

  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  userLog = "IP: ";
  userLog += WiFi.localIP().toString();

  time_count = millis();
}

void loop() {
  while (! (clientServer = server.available())) {
    Serial.println("No client available!");
    if (! enabled) {
      time_count = millis();
      rebooted = 0;
    }
    else if (millis() - time_count >= (ping_time + (rebooted ? reboot_time : 0)) * 1000) {
      if (clientPing.connect(host, httpPort)) { // Ping
        time_count = millis();
        rebooted = 0;

      } else if (millis() - time_count >= (ping_time + failure_time + (rebooted ? reboot_time : 0)) * 1000) {
        reboot();
        time_count = millis();
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(":");
          if (millis() - time_count >= reboot_time * 1000) {
            reboot();
            time_count = millis();
          }
          delay(200);
        }
      }
    }
    delay(200);
  }

  req = clientServer.readStringUntil('\r');
  Serial.println(req);
  clientServer.flush();

  if (req.indexOf("increment=ok") != -1) {
    debug_inc++;

    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    buf += "<script>location.reload()</script>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("log=show") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    buf += "<hr>Enabled: ";
    buf += (int)enabled;
    buf += "<br>Debug count: ";
    buf += debug_inc;
    buf += "<br>Restarts: ";
    buf += restarts;
    buf += "<br>";
    buf += userLog;
    buf += "<hr>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("reboot=now") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    buf += "<hr>Wait while the device reboots<br>";
    buf += "The page will refresh automatically";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
    delay(1000);
    reboot();
    time_count = millis();
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(":");
      if (millis() - time_count >= reboot_time * 1000) {
        reboot();
        time_count = millis();
      }
      delay(200);
    }

  } else if (req.indexOf("enable=toggle") != -1) {
    enabled = enabled ^ 1;
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    buf += "<script>enabled=";
    buf += (int)enabled;
    if (enabled) {
      buf += ";\r\n$(\"#enableButton\").css(\"color\", \"green\");\r\n$(\"#enableButton\").prop(\"value\", \"Disable RebooTinny\");\r\n";
    } else {
      buf += ";\r\n$(\"#enableButton\").css(\"color\", \"red\");\r\n$(\"#enableButton\").prop(\"value\", \"Enable RebooTinny\");\r\n";
    }
    buf += "</script>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("config=get") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>";
    buf += "<hr>ping: ";
    buf += ping_time;
    buf += " -- seconds<br>failure: ";
    buf += failure_time;
    buf += " -- seconds<br>reboot: ";
    buf += reboot_time;
    buf += " -- seconds\r\n<hr></body></html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("config=set") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><hr>\r\n";
    int index, index1, index2, index3, index4;
    String s1, s2, s3;

    index1 = req.indexOf("config=set&ping=");
    index2 = req.indexOf("failure=");
    index3 = req.indexOf("reboot=");
    index4 = req.indexOf("&done");

    if (index1 != -1 && index2 != -1 && index3 != -1 && index4 != -1) {
      s1 = req.substring(index1 + 16, index2 - 1); // ping_time value
      s2 = req.substring(index2 + 8, index3 - 1); // failure_time value
      s3 = req.substring(index3 + 7, index4); // reboot_time

      index1 = s1.toInt();
      index2 = s2.toInt();
      index3 = s3.toInt();

      if (index1 && index2 && index3) {
        ping_time = index1;
        failure_time = index2;
        reboot_time = index3;

        buf += "Values updated!";
      } else
        buf += "Error in the arguments";

    } else {
      buf += "Error in the arguments";
    }

    buf += "\r\n<hr></body></html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
    buf += "<head>\r\n<meta charset=\"utf-8\">\r\n<title>RebooTinny</title>\r\n</head>\r\n";

    buf += "<body>\r\n<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/2.1.4/jquery.min.js\"></script>\r\n<script>\r\n";
    buf += "var logShow=false;\r\nvar IP = \"";
    buf += WiFi.localIP().toString();
    buf += "\";\r\nvar enabled = ";
    buf += (int)enabled;
    buf += ";\r\nfunction inc(){\r\n$.get(\"http://\".concat(IP), \"increment=ok\", function(ret){\r\n$(\"#dynamicPlace\").html(ret);});\r\n}\r\n";
    buf += "function log(){\r\nif(logShow){\r\nlogShow=false;\r\n$(\"#dynamicPlace\").html(\"\");\r\n $(\"#logButton\").prop(\"value\", \"Show Log\"); \r\n";
    buf += "} else{\r\nlogShow=true;\r\n$.get(\"http://\".concat(IP), \"log=show\", function(ret){\r\n$(\"#dynamicPlace\").html(ret);\r\n";
    buf += "$(\"#logButton\").prop(\"value\", \"Hide Log\");\r\n});\r\n}\r\n}\r\n";
    buf += "function reboot(){\r\n$.get(\"http://\".concat(IP), \"reboot=now\", function(ret){$(\"#dynamicPlace\").html(ret);});\r\n";
    buf += "setTimeout(function(){location.reload();},15000);\r\n}\r\n";
    buf += "function enable(){\r\nif(logShow) log();\r\n$.get(\"http://\".concat(IP), \"enable=toggle\", function(ret){$(\"#dynamicPlace\").html(ret);});\r\n}\r\n";
    buf += "</script>\r\n";

    buf += "<h2>RebooTinny</h2>\r\n";
    buf += "<input type=\"button\" onclick=\"inc()\" value=\"Increment Debug\">\r\n<br>\r\n";
    buf += "<input type=\"button\" onclick=\"enable()\" id=\"enableButton\">\r\n<br>\r\n";
    buf += "<input type=\"button\" onclick=\"log()\" value=\"Show Log\" id=\"logButton\">\r\n<br>\r\n";
    buf += "<input type=\"button\" onclick=\"reboot()\" value=\"Reboot\" >\r\n<br>\r\n";
    buf += "<div id=\"dynamicPlace\"></div>\r\n";

    buf += "<script>\r\n";
    buf += "if(enabled){\r\n$(\"#enableButton\").css(\"color\", \"green\");\r\n$(\"#enableButton\").prop(\"value\", \"Disable RebooTinny\");\r\n";
    buf += "}else{\r\n$(\"#enableButton\").css(\"color\", \"red\");\r\n$(\"#enableButton\").prop(\"value\", \"Enable RebooTinny\");\r\n}\r\n";
    buf += "</script>\r\n</body>\r\n</html>";

    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();
  }
  Serial.println("Client disonnected");
}

void reboot() {
  restarts++;
  rebooted = 1;
  Serial.println("Rebooting...");
  WiFi.disconnect();
  digitalWrite(2, 1);
  delay(7000);
  digitalWrite(2, 0);
  WiFi.begin(ssid, password);
}

