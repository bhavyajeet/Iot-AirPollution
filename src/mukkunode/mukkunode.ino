
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "ThingSpeak.h"


#include <ESP8266WiFi.h>

#include <SDS011.h>

float p10,p25;
int error;

SDS011 my_sds;


#define SECRET_SSID "JioFi_20FDD75"    // replace MySSID with your WiFi network name
#define SECRET_PASS "ikhmeawitq"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID 935955     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "IV2X4SZII9B7CN4U"   // replace XYZ with your channel write API Key


#define NOGPS_CH_ID 926773     // replace 0000000 with your channel number
#define NOGPS_WRITE_APIKEY "D41RHQ9LUX1O9QCW"   // replace XYZ with your channel write API Key



char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

unsigned long myChannelNumber1 = NOGPS_CH_ID;
const char * myWriteAPIKey1 = NOGPS_WRITE_APIKEY;



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



#include "DHT.h"

#define DHTPIN D7     // Digital pin connected to the DHT sensor
#define DHTVcc D0 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);




void setup()
{

  
  Serial.begin(115200);
  ss.begin(GPSBaud);

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  my_sds.begin(D5,D6);

  Serial.println(F("DHTxx test!"));
  dht.begin();
  pinMode(DHTVcc,OUTPUT);

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop()
{

    digitalWrite(DHTVcc,HIGH);

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



    error = my_sds.read(&p25,&p10);
  if (! error) {
    Serial.println("P2.5: "+String(p25));
    Serial.println("P10:  "+String(p10));
  }


  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    digitalWrite(DHTVcc,LOW);
    digitalWrite(DHTPIN,LOW);
    delay(5000);
    digitalWrite(DHTVcc,HIGH);
    digitalWrite(DHTPIN,HIGH);    
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  ThingSpeak.setField(1,t);
  ThingSpeak.setField(2, h);


  
  
  ThingSpeak.setField(3, p25);
  ThingSpeak.setField(4, p10);


    
//  int op = ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
//  if(op==200){
//    Serial.println("nogps Channel update successful.");
//  }
//  else{
//    Serial.println("Problem updating nogps channel. HTTP error code " + String(op));
//  }
// 
//  

  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo(t,h,p25,p10);

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  delay(100);
}



void displayInfo(float n4,float n5,float n6,float n7 )
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

    ThingSpeak.setField(1, n4);
    ThingSpeak.setField(2, n5);
    ThingSpeak.setField(3, n6);
    ThingSpeak.setField(4, n7);
    
    ThingSpeak.setField(5,lati);
    ThingSpeak.setField(6, longi);
    ThingSpeak.setField(7, i);

  
  
  int x = ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
  if(x==200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  
}




