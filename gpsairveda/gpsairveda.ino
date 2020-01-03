#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "ThingSpeak.h"


#include <ESP8266WiFi.h>

#include <SDS011.h>

float p10,p25;
int error;



#define SECRET_SSID "iPhone"    // replace MySSID with your WiFi network name
#define SECRET_PASS "hello1234abc"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID 00000     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "2EWU9KYGQ0UM6VHY"   // replace XYZ with your channel write API Key

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = D3, TXPin = D4;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

 float lati;
 float longi;

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop()
{

    if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }



  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  delay(100);
}

void displayInfo()
{
  String i ="";
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    lati=gps.location.lat();
    longi=gps.location.lng();
    Serial.print(gps.location.lat(), 6);
    Serial.print("    ");
    Serial.print(lati);
    Serial.print("    ");
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    
    i+=gps.date.day();
    i+="/";
    i+=gps.date.month();
    i+="/";
    i+=gps.date.year();
    
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    i+=" ";
    i+=gps.time.hour();
    i+=":";
    i+=gps.time.minute();
    i+=":";
    i+=gps.time.second();
    Serial.print(i);
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  
  Serial.println();
  Serial.println(i);

    ThingSpeak.setField(1,lati);
  ThingSpeak.setField(2, longi);
  ThingSpeak.setField(3, i);
//  ThingSpeak.setField(4, number4);
  
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x==200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  
}
