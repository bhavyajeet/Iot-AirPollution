/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/



#define SECRET_SSID "JioFi_20F8DDE"    // replace MySSID with your WiFi network name
#define SECRET_PASS "j6nwnxj92h"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID_WEATHER_STATION 948389             //MathWorks weather station

#define SECRET_CH_ID_COUNTER 948389          //Test channel for counting
#define SECRET_READ_APIKEY_COUNTER "BKSS9VUTQKU3Z3EQ" //API Key for Test 


#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;



#include "ThingSpeak.h"

#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

// Weather station channel details
unsigned long dhtChannelNumber = 948389;
unsigned int temperatureFieldNumber = 1;
unsigned int humFieldNumber = 2;

// Counting channel details
unsigned long grChannelNumber = 948389;
unsigned int tvocFieldNumber = 1;
unsigned int ecoFieldNumber = 2;

 unsigned long counterChannelNumber = 948389;
unsigned int p25FieldNumber = 1; 
unsigned int p10FieldNumber = 2; 




// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void setup(){
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  Serial.begin(115200);  // Initialize serial

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop(){


   String c = "hello world";
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print(c);
  Serial.println("lol");
  delay(1000);
  // clears the display to print new message
  lcd.clear();
  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print("Hello, World!");
  delay(1000);
  lcd.clear(); 



  lcd.clear();    
  int statusCode = 0;
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected");
  }

  // Read in field 4 of the public channel recording the temperature
  float temperatureInF = ThingSpeak.readFloatField(dhtChannelNumber, temperatureFieldNumber);
  float hum = ThingSpeak.readFloatField(dhtChannelNumber, humFieldNumber);  
  String tp="";
  tp+=String(temperatureInF);
  tp+= " degC   ";
  tp+=String(hum);
  tp+= "%";
  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Temperature at MathWorks HQ: " + String(temperatureInF) + " deg F");
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }


  //  String c = "hello world";
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print(tp);
  Serial.println("lol");
  delay(1000);
  
   // No need to read the temperature too often.

  String pm = "";
  // Read in field 1 of the private channel which is a counter  
  float p25c = ThingSpeak.readFloatField(counterChannelNumber, p25FieldNumber);
  float count = ThingSpeak.readFloatField(counterChannelNumber, p10FieldNumber);  
  pm+= String(p25c);
  pm+="    ";
  pm+= String(count);
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Counter: " + String(count));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  // clears the display to print new message
//  lcd.clear();
  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print(pm);
//

  String tvoc = " ";
  // Read in field 1 of the private channel which is a counter  
  float tvc = ThingSpeak.readFloatField(grChannelNumber, tvocFieldNumber);
  float eco2 = ThingSpeak.readFloatField(grChannelNumber, ecoFieldNumber);  
  tvoc+= String(tvc);
  tvoc+="ppb ";
  tvoc+= String(eco2);
  tvoc+="ppm";
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Counter: " + String(count));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  // clears the display to print new message
//  lcd.clear();
  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print(pm);
//
  lcd.setCursor(0,2);

  
  lcd.print(tvoc);
  
  delay(5000);
   

 
  delay(1500);
}

