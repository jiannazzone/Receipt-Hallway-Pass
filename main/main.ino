
#include <ESP8266WiFi.h>
#include <Button2.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Preferences.h>
#include <Regexp.h>
#include "ArduinoJson.h"
#include "html.h"
#include <DNSServer.h>

AsyncWebServer server(80);
DNSServer dnsServer;
IPAddress apIP(8, 8, 4, 4); // The default android DNS
const char *ssidAP = "hall-pass-printer";
bool serverRunning = false;

Preferences prefs;

const char* monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
const char* weekdays[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

time_t t;
bool is12Hour = true;

String teacherName;
String school;

class IndexRequestHandler : public AsyncWebHandler
{

public:
    IndexRequestHandler() {}
    virtual ~IndexRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_GET)
        {
            String url = request->url();
            if (url == "/")
            {
                return true;
            }
        }
        return false;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", indexHTML);
    }
};

class TimeRequestHandler : public AsyncWebHandler
{
private:
    bool setSuccess = false;

public:
    TimeRequestHandler() {}
    virtual ~TimeRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_PUT || request->method() == HTTP_GET)
        {
            MatchState ms;
            String url = request->url();
            char urlCharArr[100];
            url.toCharArray(urlCharArr, url.length() + 1);
            ms.Target(urlCharArr);
            char result = ms.Match("/time");
            if (result > 0)
            {
                return true;
            }
        }
        return false;
    }

    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
        if (request->method() == HTTP_PUT)
        {

            StaticJsonDocument<128> doc;

            DeserializationError error = deserializeJson(doc, data);

            if (error)
            {
                return;
            }
            int newHour = hour();
            int newMinute = minute();
            int newSecond = second();
            int newDay = day();
            int newMonth = month();
            int newYear = year();

            if (doc["hour"] != nullptr)
            {
                newHour = doc["hour"];
            }
            if (doc["minute"] != nullptr)
            {
                newMinute = doc["minute"];
            }
            if (doc["second"] != nullptr)
            {
                newSecond = doc["second"];
            }
            if (doc["day"] != nullptr)
            {
                newDay = doc["day"];
            }
            if (doc["month"] != nullptr)
            {
                newMonth = doc["month"];
            }
            if (doc["year"] != nullptr)
            {
                newYear = doc["year"];
            }
            setTime(newHour, newMinute, newSecond, newDay, newMonth, newYear);
            RTC.set(now());
            setSuccess = true;
        }
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_PUT || request->method() == HTTP_GET)
        {
            if (setSuccess || request->method() == HTTP_GET)
            {

                String output;

                StaticJsonDocument<128> doc;

                JsonObject object = doc.createNestedObject();

                object["hour"] = hour();
                object["minute"] = minute();
                object["second"] = second();
                object["day"] = day();
                object["month"] = month();
                object["year"] = year();

                serializeJson(doc, output);
                request->send(200, "application/json", output);
            }
            else
            {
                request->send(400, "application/json", "BAD INPUTS ERROR MESSAGE");
            }
        }
    }
};

class ServerKillRequestHandler : public AsyncWebHandler
{
private:
    bool setSuccess = false;

public:
    ServerKillRequestHandler() {}
    virtual ~ServerKillRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        setSuccess = false;
        if (request->method() == HTTP_GET)
        {
            MatchState ms;
            String url = request->url();
            char urlCharArr[100];
            url.toCharArray(urlCharArr, url.length() + 1);
            ms.Target(urlCharArr);
            char result = ms.Match("/serverkill");
            if (result > 0)
            {
                return true;
            }
        }
        return false;
    }

    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_GET)
        {

            String output = "success";
            server.end();
            WiFi.softAPdisconnect(true);
            serverRunning = false;

            request->send(200, "application/json", output);
        }
    }
};

class SettingsRequestHandler : public AsyncWebHandler
{
private:
    bool setSuccess = false;

public:
    SettingsRequestHandler() {}
    virtual ~SettingsRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        setSuccess = false;
        if (request->method() == HTTP_PUT || request->method() == HTTP_GET)
        {
            MatchState ms;
            String url = request->url();
            char urlCharArr[100];
            url.toCharArray(urlCharArr, url.length() + 1);
            ms.Target(urlCharArr);
            char result = ms.Match("/settings");
            if (result > 0)
            {
                return true;
            }
        }
        return false;
    }

    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
        if (request->method() == HTTP_PUT)
        {

            StaticJsonDocument<256> doc;

            DeserializationError error = deserializeJson(doc, data);

            if (error)
            {
                return;
            }
            if (doc["teacherName"] != nullptr)
            {
                teacherName = doc["teacherName"].as<String>();
                prefs.putString("teacherName", teacherName);
                Serial.println("Setting teacher to: " + teacherName);
            }
            if (doc["school"] != nullptr)
            {
                school = doc["school"].as<String>();
                prefs.putString("school", school);
                Serial.println("Setting school to: " + school);
            }
            setSuccess = true;
        }
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_PUT || request->method() == HTTP_GET)
        {
            if (setSuccess || request->method() == HTTP_GET)
            {

                String output;

                StaticJsonDocument<128> doc;

                JsonObject object = doc.createNestedObject();

                object["teacherName"] = teacherName;
                object["school"] = school;
               
                
                serializeJson(doc, output);
                request->send(200, "application/json", output);
            }
            else
            {
                request->send(400, "application/json", "BAD INPUT ERROR MESSAGE");
            }
        }
    }
};
// notFound will return an error message if any non existant endpoints are accessed on the webserver.
void notFound(AsyncWebServerRequest *request)
{
    request->redirect("/");
}

void startServer()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidAP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    server.begin();
    dnsServer.start(53, "*", WiFi.softAPIP());
}

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
  if (timeStatus() != timeSet) {
    if (setTimeWithCompiler(__DATE__, __TIME__)) {
        Serial.println(__DATE__);
        Serial.println(__TIME__);
        Serial.println("Time set by compiler");
    }
  } else {
    Serial.println("RTC has set the system time");
  }

  prefs.begin("data", false);

  teacherName = prefs.getString("teacherName", "NO TEACHER SET");
  school = prefs.getString("school", "NO SCHOOL SET");

  Serial.println("TEACHER: " + teacherName);
  Serial.println("SCHOOL: " + school);

  server.onNotFound(notFound);
  server.addHandler(new IndexRequestHandler());
  server.addHandler(new TimeRequestHandler());
  server.addHandler(new ServerKillRequestHandler());
  server.addHandler(new SettingsRequestHandler());

  startServer();
  serverRunning = true;
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
  printer.println(school.c_str());
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
  printer.println(teacherName.c_str());
  printer.setSize('S');
  printer.println("Teacher Signature");
  printer.feed(4);
}

void loop() {
  if (serverRunning)
    {
        dnsServer.processNextRequest();
    }
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
