#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Button.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

const char* ssid = "Get Off My LAN";
const char* password = "youdamnkids2";
const char* host = "http://api.timezonedb.com/v2/get-time-zone?key=YAHNY649CET0&format=xml&fields=formatted&by=zone&zone=America/New_York";

String payload;
tmElements_t tm;

Button buttonList[4] = { Button(16), Button(14), Button(12), Button(13) };
String message[4] = { "bathroom", "classroom", "cafeteria", "other" };


void setup() {
  for (int i = 0; i < 4; i++) {
    buttonList[i].begin();
  }
  Serial.begin(9600);
  delay(10);
  Serial.println("Serial ready.");

  wifi();
  tzdb();
  parse_response();
}

void wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();           // Clear any existing connection
  WiFi.begin(ssid, password);  // Access WiFi

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ...");

  while (WiFi.status() != WL_CONNECTED) {  // Wait for WiFi to connect
    delay(1000);
    Serial.print(".");
  }

  Serial.println('\n');
  Serial.println("WiFi connection established");
  Serial.print("Device's IP address is ");
  Serial.println(WiFi.localIP());  // Show device's IP address
}

void tzdb() {
  int httpCode = 0;  // Variable to hold received data
  HTTPClient http;   // Declare an object of class HTTPClient
  WiFiClient client;
  Serial.println("Connecting to TimezoneDB...");

  http.begin(client, host);  // Connect to site
  httpCode = http.GET();     // Check if data is coming in

  while (httpCode == 0) {      // if no data is in
    delay(1000);               // wait a sec
    http.begin(client, host);  // and try again
    httpCode = http.GET();
  }

  payload = http.getString();  // Save response as string
  Serial.println(payload);     // Show response

  //Sample response:
  //<?xml version="1.0" encoding="UTF-8"?>
  //<result><status>OK</status><message/><formatted>2018-02-08 14:24:16</formatted></result>

  http.end();           // Close connection to timezonedb
  WiFi.mode(WIFI_OFF);  // Close connection to WiFi
}

void parse_response() {
  int colon = payload.indexOf(':');  // Set the first colon in time as reference point
  int d;                             // Variables to hold data as integers
  int mo;
  int y;
  int h;
  int mi;
  int s;

  String nowday = payload.substring(colon - 5, colon - 3);  // Get data as substring
  tm.Day = nowday.toInt();                                       // and convert to int
  String nowmonth = payload.substring(colon - 8, colon - 6);
  tm.Month = nowmonth.toInt();
  String nowyear = payload.substring(colon - 13, colon - 9);
  tm.Year = nowyear.toInt();

  String nowhour = payload.substring(colon - 2, colon);
  tm.Hour = nowhour.toInt();
  String nowmin = payload.substring(colon + 1, colon + 3);
  tm.Minute = nowmin.toInt();
  String nowsec = payload.substring(colon + 4, colon + 6);
  tm.Second = nowsec.toInt();

  // Set the RTC
  if (RTC.write(tm)) {
    Serial.println("RTC Configured");
    Serial.println(__DATE__);
    Serial.println(__TIME__);
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {
    if (buttonList[i].pressed()) {
      Serial.println(message[i]);
    }
  }
  RTC.read(tm);
  Serial.print(tm.Hour);
  Serial.print(":");
  Serial.print(tm.Minute);
  Serial.print(":");
  Serial.println(tm.Second);
  delay(30000);
}
