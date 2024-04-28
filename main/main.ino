#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Button.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "secrets.h"

const char* ssid = my_SSID;
const char* password = my_pw;
const char* host = "http://api.timezonedb.com/v2/get-time-zone?key=YAHNY649CET0&format=xml&fields=formatted&by=zone&zone=America/New_York";
const char* monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
const char* weekdays[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

String payload;
tmElements_t tm;
bool is12Hour = true;

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
}  // setup

void wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();           // Clear any existing connection
  WiFi.begin(ssid, password);  // Access WiFi

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ...");

  while (WiFi.status() != WL_CONNECTED) {  // Wait for WiFi to connect
    delay(1000);
  }

  Serial.println('\n');
  Serial.println("WiFi connection established");
  Serial.print("Device's IP address is ");
  Serial.println(WiFi.localIP());  // Show device's IP address
}  // wifi

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
  http.end();                  // Close connection to timezonedb
  WiFi.mode(WIFI_OFF);         // Close connection to WiFi
}  // tzdb

void parse_response() {
  int index = payload.indexOf(':');

  tm.Day = payload.substring(index - 5, index - 3).toInt();
  tm.Month = payload.substring(index - 8, index - 6).toInt();
  tm.Year = payload.substring(index - 13, index - 9).toInt();

  tm.Hour = payload.substring(index - 2, index).toInt();
  tm.Minute = payload.substring(index + 1, index + 3).toInt();
  tm.Second = payload.substring(index + 4, index + 6).toInt();

  // Set the RTC
  if (RTC.write(tm)) {
    Serial.println("RTC Configured");
    Serial.println(__DATE__);
    Serial.println(__TIME__);
  }
}  // parse_response

String make_time() {
  RTC.read(tm);
  String timeString = "";
  int hour = tm.Hour;
  if (is12Hour && tm.Hour > 12) {
    timeString += tm.Hour - 12;
  } else {
    timeString += tm.Hour;
  }
  timeString += ":";
  timeString += leadingZero(tm.Minute);
  timeString += ":";
  timeString += leadingZero(tm.Second);

  if (is12Hour && tm.Hour > 11) {
    timeString += " PM";
  } else {
    timeString += " AM";
  }

  return timeString;
}  // make_time

String make_date() {
  String dateString = "";
  dateString += weekdays[tm.Wday];
  dateString += " ";
  dateString += monthName[tm.Month - 1];
  dateString += " ";
  dateString += tm.Day;

  int onesDigit = tm.Day % 10;
  if ((tm.Day > 3 && tm.Day < 21) || onesDigit > 3) {
    dateString += "th, ";
  } else if (onesDigit == 1) {
    dateString += "st, ";
  } else if (onesDigit == 2) {
    dateString += "nd, ";
  } else {
    dateString += "rd, ";
  }  // if-else
  return dateString;
}  // make_date

String leadingZero(int x) {
  if (x < 10) {
    return "0" + String(x);
  } else {
    return String(x);
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {
    if (buttonList[i].pressed()) {
      Serial.println(make_date());
      Serial.println(make_time());
      Serial.println(message[i]);
    }
  }
}
