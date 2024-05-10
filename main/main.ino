#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Button2.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>
#include "secrets.h"
#include <Wire.h>

// RTC and time
const String host = "http://api.timezonedb.com/v2/get-time-zone?key=" + String(my_apiKey) + "&format=xml&fields=formatted&by=zone&zone=America/New_York";
const char* monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
const char* weekdays[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
String payload;
time_t t;
bool is12Hour = true;
bool wifiSuccess = true;

// Thermal Printer
#define TX_PIN 2
#define RX_PIN 0
SoftwareSerial printer_connection(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&printer_connection);

// Buttons
Button2 bathroomButton;
Button2 cafeButton;
Button2 classroomButton;
Button2 otherButton;

String destination[4] = { "BATHROOM", "CAFETERIA", "CLASSROOM", "OTHER" };

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Serial ready.");

  setSyncProvider(RTC.get);
  if (setTimeWithCompiler(__DATE__, __TIME__)) {
      Serial.println(__DATE__);
      Serial.println(__TIME__);
      Serial.println("Time set by compiler");
  }
  
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  wifi();
  if (wifiSuccess) {
    tzdb();
    parse_response();
  }
  // Start thermal printer
  
  printer_connection.begin(9600);
  printer.begin();
  
  bathroomButton.begin(15, INPUT, false);  // GPIO15 behaves different than the others. Must be wired to VCC instead of GND
  cafeButton.begin(14);
  classroomButton.begin(12);
  otherButton.begin(13);
  bathroomButton.setClickHandler(click);
  cafeButton.setClickHandler(click);
  classroomButton.setClickHandler(click);
  otherButton.setClickHandler(click);
  
}  // setup

void wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // Clear any existing connection
  if (my_pw != "") {
    WiFi.begin(my_SSID, my_pw);
  } else {
    WiFi.begin(my_SSID);
  }

  Serial.print("Connecting to ");
  Serial.print(my_SSID);
  Serial.print(" ...");

  int connectAttemptCount = 0;
  while (WiFi.status() != WL_CONNECTED) {  // Wait for WiFi to connect
    Serial.println("Waiting to connect");
    connectAttemptCount += 1;
    if (connectAttemptCount > 10) {
      Serial.println("Error connecting to WiFi. Continuing anyway.");
      wifiSuccess = false;
      break;
    }
    delay(1000);
  }

  if (wifiSuccess) {
    Serial.println('\n');
    Serial.println("WiFi connection established");
    Serial.print("Device's IP address is ");
    Serial.println(WiFi.localIP());  // Show device's IP address
  }
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
  Serial.println("Time fetched.");
  Serial.println(payload);
}  // tzdb

void parse_response() {
  int index = payload.indexOf(':');

  int day = payload.substring(index - 5, index - 3).toInt();
  int month = payload.substring(index - 8, index - 6).toInt();
  int year = payload.substring(index - 13, index - 9).toInt();

  int hour = payload.substring(index - 2, index).toInt();
  int min = payload.substring(index + 1, index + 3).toInt();
  int sec = payload.substring(index + 4, index + 6).toInt();

  setTime(hour, min, sec, day, month, year);
  t = now();
  RTC.set(t);
}  // parse_response

String make_time() {
  t = now();
  String timeString = "";
  int hr = hour(t);
  if (is12Hour && hr > 12) {
    timeString += hr - 12;
  } else {
    timeString += hr;
  }
  timeString += ":";
  timeString += leading_zero(minute(t));
  timeString += ":";
  timeString += leading_zero(second(t));

  if (is12Hour && hr > 11) {
    timeString += " PM";
  } else {
    timeString += " AM";
  }

  return timeString;
}  // make_time

String make_date() {
  String dateString = "";
  dateString += weekdays[weekday(t) - 1];
  dateString += " ";
  dateString += monthName[month(t) - 1];
  dateString += " ";
  dateString += day(t);

  int onesDigit = day() % 10;
  if ((day() > 3 && day() < 21) || onesDigit > 3) {
    dateString += "th";
  } else if (onesDigit == 1) {
    dateString += "st";
  } else if (onesDigit == 2) {
    dateString += "nd";
  } else {
    dateString += "rd";
  }  // if-else

  dateString += ", ";
  dateString += year(t);

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
  printer.println(F("ADAM IANNAZZONE"));
  printer.setSize('S');
  printer.println("Teacher Signature");
  printer.feed(4);
}

void loop() {
  bathroomButton.loop();
  cafeButton.loop();
  classroomButton.loop();
  otherButton.loop();
}

void click(Button2& btn) {
  if (btn == bathroomButton) {
    Serial.println("bathroom");
    print_pass(0);
  } else if (btn == cafeButton) {
    Serial.println("cafe");
    print_pass(1);
  } else if (btn == classroomButton) {
    Serial.println("class");
    print_pass(2);
  } else if (btn == otherButton) {
    Serial.println("other");
    print_pass(3);
  }
}

bool setTimeWithCompiler(const char *datestr, const char *timestr) {
  const char *monthName[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;
  int Hour, Min, Sec;

  // Get date
  if (sscanf(datestr, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;

  // Get time
  if (sscanf(timestr, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  setTime(Hour, Min, Sec+1 , Day, monthIndex + 1, Year);
  t = now();
  RTC.set(t);
  return true;
}
