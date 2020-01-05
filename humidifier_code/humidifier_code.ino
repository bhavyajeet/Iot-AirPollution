#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
 
const char* ssid = "moto g";
const char* password = "justmonika";
 
void setup () {
 
Serial.begin(115200);
Serial.print(password);
WiFi.begin(ssid, password);
 
while (WiFi.status() != WL_CONNECTED) {
 
delay(1000);
Serial.print("Connecting..");
 
}
 
}
 
void loop() {
 
if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
HTTPClient http;  //Declare an object of class HTTPClient
 
http.begin("http://192.168.43.9:8080/~/in-cse/in-name/Project_1_Indoor_Air_Pollution_Management/Classroom_1/la");  //Specify request destination
http.addHeader("Content-Type", "application/json");
    http.addHeader("X-M2M-Origin", "admin:admin");
int httpCode = http.GET();                                                        
if (httpCode > 0) { //Check the returning code
 
String payload = http.getString();   //Get the request response payload
Serial.println(payload);                     //Print the response payload
 
}
 
http.end();   //Close connection
 
}
 
delay(30000);    //Send a request every 30 seconds
 
}
