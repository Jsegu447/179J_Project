/*
 * Make sure you have the ESP8266 board add-on installed.
 * You also need to install the ArduinoJson version 5.13.5 library. IMPORTANT CHECK VERSION CODE NOT COMPATIBLE WITH v.6
 * This uses OpenWeatherMap API, b/c Weather Underground has more hoops
 * http://api.openweathermap.org/data/2.5/forecast?q={your city},{your country code}&APPID={your API Key}&mode=json&units=metric&cnt=2
 * http://api.openweathermap.org/data/2.5/forecast?q=Riverside,US&APPID=f01ee08d79969282700910b2f7257a77&mode=json&units=imperial&cnt=2
 * 
 *
 * program should output to serial the weather condition a.k.a sunny, rain, etc.
 * 
 * 
 * References: https://github.com/ThingPulse/esp8266-weather-station
 *             https://www.youtube.com/watch?v=8xqgdXvn3yw
 *             https://randomnerdtutorials.com/esp8266-weather-forecaster/
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <WiFiClient.h> //from esp8266

WiFiClient client;

#define apiKey ""

char temp1 = 0;
char temp2 = 0;
String hum = "";

const char* ssid = ""; // Replace with your ssid and pass
const char* password = "";

// Open Weather Map API server name
const char server[] = "api.openweathermap.org";
String location = ""; //Location [City,Country]

String text;

int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;

#define JSON_BUFF_DIMENSION 2500

unsigned long lastConnectionTime = 60*1000;//1 minute//10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 60*1000;//10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)


//TIME STUFF
unsigned int localPort = 2390; //UDP PORT
IPAddress timeServerIP;  // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;




void setup() {
  Serial.begin(115200); 
  text.reserve(JSON_BUFF_DIMENSION);
  WiFi.begin(ssid, password);
  
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi network with following info: ");
//  printWiFiStatus();

  //TIME STUFF
  udp.begin(localPort);
 // Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void loop() {
  // OWM requires 10mins between request intervals
  // check if 10mins has passed then conect again and pull
  // Send an HTTP POST request depending on timerDelay
  getTime();
  if ((millis() - lastConnectionTime) > postInterval) {
    // note the time that the connection was made:
    lastConnectionTime = millis();
    makehttpRequest();
  }
  Serial.print(temp1);
  Serial.print(temp2);
  Serial.print(hum);
  Serial.print("\n");
  delay(100);
}

// print Wifi status
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// to request data from OWM
void makehttpRequest() {
  // close any connection before send a new request to allow client make connection to server
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /data/2.5/weather?q=" + location + "&APPID=" + apiKey + "&mode=json&units=imperial&cnt=2 HTTP/1.1");
    //client.println("GET /data/2.5/forecast?q=" + location + "&APPID=" + apiKey + "&mode=json&units=imperial&cnt=2 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    char c = 0;
    while (client.available()) {
      c = client.read();
      // since json contains equal number of open and close curly brackets, this means we can determine when a json is completely received  by counting
      // the open and close occurences,
      delay(1);// bug
     // Serial.print(c);// 
      if (c == '{') {
        startJson = true;         // set startJson true to indicate json message has started
        jsonend++;
      }
      if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        text += c;
      }
      // if jsonend = 0 then we have have received equal number of curly braces 
      if (jsonend == 0 && startJson == true) {
        //Serial.println("parse");
        parseJson(text.c_str());  // parse c string text in parseJson function
        text = "";                // clear text string for the next time
        startJson = false;        // set startJson to false to indicate that a new message has not yet started
      }//*/
    }
  }
  else {
    // if no connection was made:
    Serial.println("connection failed!");
    return;
  }
}

//to parse json data recieved from OWM
void parseJson(const char * jsonString) {
  //StaticJsonBuffer<4000> jsonBuffer;
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720; //720
  DynamicJsonBuffer jsonBuffer(bufferSize);

  // FIND FIELDS IN JSON TREE
  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  
  float coord_lon = root["coord"]["lon"]; // -116.0508
float coord_lat = root["coord"]["lat"]; // 33.7333

JsonObject& weather_0 = root["weather"][0];
int weather_0_id = weather_0["id"]; // 800
const char* weather_0_main = weather_0["main"]; // "Clear"
const char* weather_0_description = weather_0["description"]; // "clear sky"
const char* weather_0_icon = weather_0["icon"]; // "01d"

const char* base = root["base"]; // "stations"

JsonObject& main = root["main"];
String main_temp = main["temp"]; // 60.93 //float
float main_feels_like = main["feels_like"]; // 57.9
float main_temp_min = main["temp_min"]; // 58.86
float main_temp_max = main["temp_max"]; // 62.6
int main_pressure = main["pressure"]; // 1016
String main_humidity = main["humidity"]; // 25 //int

int visibility = root["visibility"]; // 10000

int wind_speed = root["wind"]["speed"]; // 0
int wind_deg = root["wind"]["deg"]; // 0

int clouds_all = root["clouds"]["all"]; // 0

long dt = root["dt"]; // 1678146388

JsonObject& sys = root["sys"];
int sys_type = sys["type"]; // 2
long sys_id = sys["id"]; // 2007138
const char* sys_country = sys["country"]; // "US"
long sys_sunrise = sys["sunrise"]; // 1678111608
long sys_sunset = sys["sunset"]; // 1678153469

int timezone = root["timezone"]; // -28800
long id = root["id"]; // 5387890
const char* name = root["name"]; // "Riverside"
int cod = root["cod"]; // 200

 // Serial.println();//
//  Serial.println("Date: " + "ERROR");
 // Serial.println("Current temperature: " + main_temp + "°F");
//  Serial.println("Current Humididty: " + main_humidity + "%");
  temp1 = main_temp[0];
  temp2 = main_temp[1];
  hum = main_humidity;
  //Serial.print(main_temp[0]);
  //Serial.print(main_temp[1]);
  //Serial.print(main_humidity);
  //Serial.println("Date: " + list_1_dt_txt);
 // Serial.println();
}


void sendNTPpacket(IPAddress& address) {
 // Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);  // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void getTime(){

 // get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP);  // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    //Serial.println("no packet yet");
  } else {
   // Serial.print("packet received, length=");
    //Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    //  or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
   // Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears - 28800 ; // 28800 sec accounts for 8 hr dif from utc -> pst
    // print Unix time:
    //Serial.println(epoch);


    // print the hour, minute and second:
    //Serial.print("The PST time is ");       // PST
    //Serial.print((epoch % 86400L) / 3600);  // print the hour (86400 equals secs per day)
    int hour = (epoch % 86400L) / 3600;
   // Serial.print(':');
   // if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
     // Serial.print('0');
   // }
    //Serial.print((epoch % 3600) / 60);
    int min = (epoch % 3600) / 60;  // print the minute (3600 equals secs per minute)
   // Serial.print(':');
   // if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
     // Serial.print('0');
  //  }
    // Serial.println(epoch % 60);  // print the second
    int second = epoch % 60;
    //Serial.print("Time:");
    Serial.print(hour);
    if (min < 10) {
      Serial.print('0');
    }
    Serial.print(min);
    if(second < 10){
      Serial.print("0");
    }
    Serial.print(second);
    //Serial.print("\n");
  }
  // wait ten seconds before asking for the time again
  //delay(1000);
}
