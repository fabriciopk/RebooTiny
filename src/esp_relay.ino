#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWD";
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

void reboot();

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  debug_inc = 0;
  enabled = 1;
  restarts = 0;
  rebooted = 1;

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

  ArduinoOTA.begin();
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
    ArduinoOTA.handle();
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
    buf += "<br>Version: 1.1<hr>";
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
      buf += ";$(\"#enableButton\").css(\"color\", \"green\");$(\"#enableButton\").prop(\"value\", \"Disable RebooTinny\");";
    } else {
      buf += ";$(\"#enableButton\").css(\"color\", \"red\");$(\"#enableButton\").prop(\"value\", \"Enable RebooTinny\");";
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
    buf += " -- seconds<hr></body></html>";
    clientServer.print(buf);
    clientServer.flush();
    clientServer.stop();

  } else if (req.indexOf("config=set") != -1) {
    buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><hr>";
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

    buf += "<body><script src=\"http://ajax.googleapis.com/ajax/libs/jquery/2.1.4/jquery.min.js\"></script><script>";
    buf += "var logShow=false;\r\nvar IP = window.location.hostname;";
    buf += "var enabled = ";
    buf += (int)enabled;
    clientServer.print(buf);
    buf = ";function inc(){$.get(\"http://\".concat(IP), \"increment=ok\", function(ret){$(\"#dynamicPlace\").html(ret);});}";
    buf += "function log(){if(logShow){logShow=false;$(\"#dynamicPlace\").html(\"\");$(\"#logButton\").prop(\"value\", \"Show Log\");";
    buf += "} else{\r\nlogShow=true;$.get(\"http://\".concat(IP), \"log=show\",function(ret){$(\"#dynamicPlace\").html(ret);";
    buf += "$(\"#logButton\").prop(\"value\", \"Hide Log\");});}}";
    buf += "function reboot(){\r\n$.get(\"http://\".concat(IP), \"reboot=now\", function(ret){$(\"#dynamicPlace\").html(ret);});";
    buf += "setTimeout(function(){location.reload();},15000);}";
    buf += "function enable(){if(logShow) log();$.get(\"http://\".concat(IP), \"enable=toggle\", function(ret){$(\"#dynamicPlace\").html(ret);});}";
    buf += "</script>";

    clientServer.print(buf);
    buf = "<h2>RebooTinny</h2>";
    buf += "<input type=\"button\" onclick=\"inc()\" value=\"Increment Debug\"><br>";
    buf += "<input type=\"button\" onclick=\"enable()\" id=\"enableButton\"><br>";
    buf += "<input type=\"button\" onclick=\"log()\" value=\"Show Log\" id=\"logButton\"><br>";
    buf += "<input type=\"button\" onclick=\"reboot()\" value=\"Reboot\" ><br>";
    buf += "<div id=\"dynamicPlace\"></div>";

    buf += "<script>";
    buf += "if(enabled){$(\"#enableButton\").css(\"color\", \"green\");$(\"#enableButton\").prop(\"value\", \"Disable RebooTinny\");";
    buf += "}else{$(\"#enableButton\").css(\"color\", \"red\");$(\"#enableButton\").prop(\"value\", \"Enable RebooTinny\");}";
    buf += "</script></body></html>";

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

