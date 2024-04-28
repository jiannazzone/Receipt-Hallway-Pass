#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Button.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#include "secrets.h"

// RTC and time
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

// Thermal Printer
#define TX_PIN 2
#define RX_PIN 0
SoftwareSerial printer_connection(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&printer_connection);

// Buttons
Button buttonList[4] = { Button(15), Button(14), Button(12), Button(13) };
String destination[4] = { "BATHROOM", "CAFETERIA", "CLASSROOM", "OTHER" };

void setup() {
  for (int i = 0; i < 4; i++) {
    buttonList[i].begin();
  }
  Serial.begin(9600);
  delay(10);
  Serial.println("Serial ready.");

  // Set RTC time
  wifi();
  tzdb();
  parse_response();

  // Start thermal printer
  printer_connection.begin(9600);
  printer.begin();
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
  timeString += leading_zero(tm.Minute);
  timeString += ":";
  timeString += leading_zero(tm.Second);

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
    dateString += "th";
  } else if (onesDigit == 1) {
    dateString += "st";
  } else if (onesDigit == 2) {
    dateString += "nd";
  } else {
    dateString += "rd";
  }  // if-else
  return dateString;
}  // make_date

String leading_zero(int x) {
  if (x < 10) {
    return "0" + String(x);
  } else {
    return String(x);
  }
}

void print_pass(int i) {
  // Heading
  printer.inverseOn();
  printer.setSize('L');
  printer.justify('C');
  printer.println(F("Highland"));
  printer.println(F("Hall Pass"));
  printer.inverseOff();
  printer.feed(1);

  // Date and Time
  printer.setSize('M');
  printer.println(make_date());
  printer.println(make_time());
  printer.feed(2);

  // Student Name
  printer.setSize('L');
  printer.println(F("______________"));
  printer.setSize('S');
  printer.println("Student Name");
  printer.feed(2);

  // Destination
  printer.setSize('L');
  if (i < 2) {
    // Bathroom or Cafeteria
    printer.inverseOn();
    printer.println(destination[i]);
    printer.inverseOff();
  } else if (i == 2) {
    // Classroom
    printer.print(F("______________"));
    printer.setSize('S');
    printer.println(F("'s"));
    printer.setSize('L');
    printer.inverseOn();
    printer.println(F("Classroom"));
    printer.inverseOff();

  } else {
    // Other
    printer.println(F("______________"));
    printer.setSize('S');
    printer.println(F("Fill in destination"));
  }
  printer.feed(1);

  // Teacher Signature
  printer.setSize('L');
  printer.println(F("______________"));
  printer.setSize('S');
  printer.println("Teacher Signature");
  printer.feed(4);
}

void loop() {
  for (int i = 0; i < 4; i++) {
    if (buttonList[i].pressed()) {
      print_pass(i);
    }
  }
}
