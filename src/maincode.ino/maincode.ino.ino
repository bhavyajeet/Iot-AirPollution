
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <SDS011.h>
#include <Wire.h>
#include "MutichannelGasSensor.h"
#include "DHT.h"


#define DHTPIN D3
#define DHTTYPE DHT11
WiFiClient  client;
DHT dht(DHTPIN, DHTTYPE);




char ssid[] = "Tsp";   // your network SSID (name)
char pass[] = "pwkn885881";   // your network password
int keyIndex = 0;
int SECRET_CH_ID =1234;
char * SECRET_WRITE_APIKEY ="1234";
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
SDS011 my_sds;




String CSE_IP      = "192.168.43.9";
// #######################################################

int WIFI_DELAY  = 100; //ms

// oneM2M : CSE params
int   CSE_HTTP_PORT = 8080;
String CSE_NAME    = "in-name";
String CSE_M2M_ORIGIN  = "admin:admin";

// oneM2M : resources' params
String DESC_CNT_NAME = "DESCRIPTOR";
String DATA_CNT_NAME = "DATA";
String CMND_CNT_NAME = "COMMAND";
int TY_AE  = 2;
int TY_CNT = 3;
int TY_CI  = 4;
int TY_SUB = 23;

// HTTP constants
int LOCAL_PORT = 9999;
char* HTTP_CREATED = "HTTP/1.1 201 Created";
char* HTTP_OK    = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms

//MISC
int LED_PIN   = D1;
int SERIAL_SPEED  = 115200;

#define DEBUG

///////////////////////////////////////////




// Global variables
WiFiServer server(LOCAL_PORT); // HTTP Server (over WiFi). Binded to listen on LOCAL_PORT contant
String context = "";
String command = ""; // The received command

// Method for creating an HTTP POST with preconfigured oneM2M headers
// param : url  --> the url path of the targted oneM2M resource on the remote CSE
// param : ty --> content-type being sent over this POST request (2 for ae, 3 for cnt, etc.)
// param : rep  --> the representaton of the resource in JSON format
String doPOST(String url, int ty, String rep)
{

  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
                       "X-M2M-Origin: " + CSE_M2M_ORIGIN + "\r\n" +
                       "Content-Type: application/json;ty=" + ty + "\r\n" +
                       "Content-Length: " + rep.length() + "\r\n"
                                                           "Connection: close\r\n\n" +
                       rep;

  // Connect to the CSE address

  Serial.println("connecting to " + CSE_IP + ":" + CSE_HTTP_PORT + " ...");

  // Get a client
  WiFiClient client2;
  if (!client2.connect("192.168.43.9", CSE_HTTP_PORT))
  {
    Serial.println("Connection failed !");
    return "error";
  }

  // if connection succeeds, we show the request to be send
#ifdef DEBUG
  Serial.println(postRequest);
#endif

  // Send the HTTP POST request
  client2.print(postRequest);

  // Manage a timeout
  unsigned long startTime = millis();
  while (client2.available() == 0)
  {
    if (millis() - startTime > REQUEST_TIME_OUT)
    {
      Serial.println("Client Timeout");
      client2.stop();
      return "error";
    }
  }

  // If success, Read the HTTP response
  String result = "";
  if (client2.available())
  {
    result = client2.readStringUntil('\r');
    //    Serial.println(result);
  }
  while (client2.available())
  {
    String line = client2.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection...");
  return result;
}









// Method for creating an ApplicationEntity(AE) resource on the remote CSE (this is done by sending a POST request)
// param : ae --> the AE name (should be unique under the remote CSE)
String createAE(String ae) {
  String aeRepresentation =
    "{\"m2m:ae\": {"
    "\"rn\":\"" + ae + "\","
    "\"api\":\"org.demo." + ae + "\","
    "\"rr\":\"true\","
    "\"poa\":[\"http://" + WiFi.localIP().toString() + ":" + LOCAL_PORT + "/" + ae + "\"]"
    "}}";
#ifdef DEBUG
  Serial.println(aeRepresentation);
#endif
  return doPOST("/" + CSE_NAME, TY_AE, aeRepresentation);
}

// Method for creating an Container(CNT) resource on the remote CSE under a specific AE (this is done by sending a POST request)
// param : ae --> the targeted AE name (should be unique under the remote CSE)
// param : cnt  --> the CNT name to be created under this AE (should be unique under this AE)
String createCNT(String ae, String cnt) {
  String cntRepresentation =
    "{\"m2m:cnt\": {"
    "\"mni\":\"10\","         // maximum number of instances
    "\"rn\":\"" + cnt + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae, TY_CNT, cntRepresentation);
}















// Method for creating an ContentInstance(CI) resource on the remote CSE under a specific CNT (this is done by sending a POST request)
// param : ae --> the targted AE name (should be unique under the remote CSE)
// param : cnt  --> the targeted CNT name (should be unique under this AE)
// param : ciContent --> the CI content (not the name, we don't give a name for ContentInstances)
String createCI(String ae, String cnt, String ciContent)
{
  String ciRepresentation =
      "{\"m2m:cin\": {"
      "\"con\":\"" +
      ciContent + "\""
                  "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + cnt, TY_CI, ciRepresentation);
}


String createSUB(String ae) {
  String subRepresentation =
    "{\"m2m:sub\": {"
    "\"rn\":\"SUB_" + ae + "\","
    "\"nu\":[\"" + CSE_NAME + "/" + ae  + "\"], "
    "\"nct\":1"
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + CMND_CNT_NAME, TY_SUB, subRepresentation);
}



int hh=0;
//Variables
int chk, error;
float hum, temp, p10, p25, c, flat, flon;
String data;
unsigned long age;

int post(String ae, String container, String data) {
  String server = "192.168.43.9:8080";
  String cse = "/~/in-cse/in-name/";
  char m2m[200];
  String Data;
  Data = "{\"m2m:cin\": {"
    "\"con\":\"" + data + "\""
    "}}";
  HTTPClient http;
  Serial.println("\nConnecting to : " + server+cse+ae+container);
  if(http.begin(server+cse+ae+container)) {
    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-type", "application/json;ty=4");
    Serial.println("Uploading data: " + data);
    int response = http.POST(Data);
    http.end();
    return response;
  }
  else {
    return 1;
  }
}

void update(int val) {
  if(val == 201) Serial.println("Data updated to OneM2M successfully\n");
  else Serial.println("There was an error while uploading Data to OneM2M. Error Code: " + String(val));
}




// Method to register a module (i.e. sensor or actuator) on a remote oneM2M CSE
void registerModule(String module, bool isActuator, String intialDescription, String initialData) {
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("registermodule");
    
    String result;
    // 1. Create the ApplicationEntity (AE) for this sensor
    result = createAE(module);
    if (result == HTTP_CREATED) {
#ifdef DEBUG
      Serial.println("AE " + module + " created  !");
#endif

      // 2. Create a first container (CNT) to store the description(s) of the sensor
      result = createCNT(module, DESC_CNT_NAME);
      if (result == HTTP_CREATED) {
#ifdef DEBUG
        Serial.println("CNT " + module + "/" + DESC_CNT_NAME + " created  !");
#endif


        // Create a first description under this container in the form of a ContentInstance (CI)
        result = createCI(module, DESC_CNT_NAME, intialDescription);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CI " + module + "/" + DESC_CNT_NAME + "/{initial_description} created !");
#endif
        }
      }

      // 3. Create a second container (CNT) to store the data  of the sensor
      result = createCNT(module, DATA_CNT_NAME);
      if (result == HTTP_CREATED) {
#ifdef DEBUG
        Serial.println("CNT " + module + "/" + DATA_CNT_NAME + " created !");
#endif

        // Create a first data value under this container in the form of a ContentInstance (CI)
        result = createCI(module, DATA_CNT_NAME, initialData);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CI " + module + "/" + DATA_CNT_NAME + "/{initial_aata} created !");
#endif
        }
      }

      // 3. if the module is an actuator, create a third container (CNT) to store the received commands
      if (isActuator) {
        result = createCNT(module, CMND_CNT_NAME);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CNT " + module + "/" + CMND_CNT_NAME + " created !");
#endif

          // subscribe to any ne command put in this container
          result = createSUB(module);
          if (result == HTTP_CREATED) {
#ifdef DEBUG
            Serial.println("SUB " + module + "/" + CMND_CNT_NAME + "/SUB_" + module + " created !");
#endif
          }
        }
      }
    }
  }
}





void init_IO() {
  // Configure pin 0 for LED control
  pinMode(LED_PIN, OUTPUT);
//  digitalWrite(LED_PIN, OFF_STATE);
}
void task_IO() {
}

void init_WiFi() {
  Serial.println("Connecting to  " + String(ssid) + " ...");
  WiFi.persistent(false);
  WiFi.begin(ssid, pass);

  // wait until the device is connected to the wifi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_DELAY);
    Serial.print(".");
  }

  // Connected, show the obtained ip address
  Serial.println("WiFi Connected ==> IP Address = " + WiFi.localIP().toString());
}
void task_WiFi() {
}

void init_HTTPServer() {
  server.begin();
  Serial.println("Local HTTP Server started !");
}
void task_HTTPServer() {
  // Check if a client is connected
  //Serial.print("'");
  client = server.available();
  if (!client)
    return;

  // Wait until the client sends some data
  Serial.println("New client connected. Receiving request <=== ");
  
  while (!client.available()) {
#ifdef DEBUG
    Serial.print(".");
#endif
    delay(5);
  }

  // Read the request
  client.setTimeout(500);
  String request = client.readString();
  Serial.println(request);

  int start, end;
  // identify the right module (sensor or actuator) that received the notification
  // the URL used is ip:port/ae
  start = request.indexOf("/");
  end = request.indexOf("HTTP") - 1;
  context = request.substring(start+1, end);
#ifdef DEBUG
  Serial.println(String() + "context = [" + start + "," + end + "] -> " + context);
#endif

  // ingore verification messages 
  if (request.indexOf("vrq") > 0) {
    client.flush();
    client.print(HTTP_OK);
    delay(5);
    client.stop();
    Serial.println("Client disconnected");
    return;
  }

  //Parse the request and identify the requested command from the device
  //Request should be like "[operation_name]"
  start = request.indexOf("[");  
  end = request.indexOf("]"); // first occurence fo 
  command = request.substring(start+1, end);
#ifdef DEBUG
  Serial.println(String() + + "command = [" +  start + "," + end + "] -> " + command);
#endif

  client.flush();
  client.print(HTTP_OK);
  delay(5);
  client.stop();
  Serial.println("Client disconnected");
}


void init_luminosity() {
  String initialDescription = "Name = LuminositySensor\t"
                              "Unit = Lux\t"
                              "Location = Home\t";
  String initialData = "0";
  registerModule("LuminositySensor", false, initialDescription, initialData);
}

int LUM_PIN= 1;

void task_luminosity() {
  int sensorValue;
  sensorValue = analogRead(LUM_PIN);
  //sensorValue = random(10, 20);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("luminosity value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("LuminositySensor", DATA_CNT_NAME, ciContent);
}
void command_luminosity(String cmd) {
}

void init_led() {
  String initialDescription = "Name = node_1_humdifier\t"
                              "Location = Home\t";
  String initialData = "switchOff";
  registerModule("node_1_humdifier", true, initialDescription, initialData);
}
void task_led() {

}
void command_led(String cmd) {
  if (cmd == "switchOn") {
#ifdef DEBUG
    Serial.println("Switching on the LED ...");
#endif
   // digitalWrite(LED_PIN, ON_STATE);
  }
  else if (cmd == "switchOff") {
#ifdef DEBUG
    Serial.println("Switching off the LED ...");
#endif
 //   digitalWrite(LED_PIN, OFF_STATE);
  }
}


void init_dht() {
  String initialDescription = "Name = temperature and humidity Sensor\t"
                              "Unit = deg C,% hum \t"
                              "Location = Home\t";
  String initialData = "0";
  registerModule("node1_dht", false, initialDescription, initialData);
}

void init_novapm() {
  String initialDescription = "Name = particulate matter Sensor\t"
                              "Unit = ppm,ppm\t"
                              "Location = Home\t";
  String initialData = "0";
  registerModule("node1_novapm", false, initialDescription, initialData);
}

void init_mics() {
  String initialDescription = "Name = gas sensor \t"
                              "Unit = ppm,ppm,ppm\t"
                              "Location = Home\t";
  String initialData = "0";
  registerModule("node1_mics", false, initialDescription, initialData);
}








void setup() {
  my_sds.begin(D5, D6); //RX, TX
  Serial.begin(115200);  // Initialize serial
  WiFi.mode(WIFI_AP_STA);
  dht.begin();
  //Wire.begin(D5,D6);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
//  Serial.begin(115200);
/*
  gas.begin(0x04);//the default I2C address of the slave is 0x04
  gas.powerOn();
  Serial.print("Firmware Version = ");
  Serial.println(gas.getVersion());

*/
  Serial.print("i am here");
    if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }

    Serial.println("\nConnected.");
  }
  init_dht();
  init_novapm();
  init_mics();



  
}


float h;
float co;
float no2;
float nh3;


void loop() {
  Serial.println(".");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }

    Serial.println("\nConnected.");
  }

  Serial.println("\nConnected.");
  h = dht.readHumidity();
  co = gas.measure_CO();
  no2 = gas.measure_NO2();
  nh3 = gas.measure_NH3();
  temp = dht.readTemperature();
  my_sds.read(&p25, &p10);
  data= "{\"TEMP\":\""+String(temp)+"\",\"HUM\":\"" + String(h) +"\"}";
  data = String(temp)+","+String(h);
  Serial.println("LALALLA"+data+"GAGAGA");
  createCI("node1_dht","DATA",data);
  data = "{ PM2.5 : " + String(p25) + ", PM10 : " + String(p10) + "}";
  hh++;
  data = String(hh*100);
  hh = hh%5;
  createCI("node1_novapm","DATA",data);
  data = "{ CO : " + String(co) + ", NO2 : " + String(no2) + ", NH3 : " + String(nh3) + "}";
  data = String(co)+","+String(no2)+","+String(nh3);
  createCI("node1_mics","DATA",data);
    Serial.println("P2.5: " + String(p25));
    Serial.println("P10:  " + String(p10));
    Serial.println("temp:" + String(temp));
    Serial.println("hum:" + String(h));
    Serial.println("co:" + String(co));
    Serial.println("no2:" + String(no2));
    Serial.println("nh3:" + String(nh3));
    ThingSpeak.setField(1, (float)temp);
    ThingSpeak.setField(2, (float)h);
    ThingSpeak.setField(3, (float)p25);
    ThingSpeak.setField(4, (float)p10);
    ThingSpeak.setField(5, (float)co);
    ThingSpeak.setField(6, (float)no2);
    ThingSpeak.setField(7, (float)nh3);
    delay(1000);
    

//    update(post("Team20_Outdoor_air_pollution_2/", "node_4", data));
    
    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    \
    else {
  //    Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
  
//  else {
//    Serial.println("error");
//  }
  delay(10000);
}
