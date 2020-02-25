//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define SSID        "Nam&Nano_Wifi2.4"     // SSIDบ้าน
#define PASSWORD    ""                     // PASSบ้าน
//#define SSID        "Health_Office"       // SSID Office
//#define PASSWORD    "z038462300"         // Pass Office

#define FIREBASE_HOST "temperature-of-refrigerator.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "6fDiOSK9ApIDH851m3vLmC65oG2G7g5uKFTdyhW4"

#define LINE_TOKEN  "zbCOnOMLL8dNy1ClwK8MNNTuSS3TOiJF3suuVkh4Ri9"   // บรรทัดที่ 13 ใส่ รหัส TOKEN ที่ได้มาจากข้างบน
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11

#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <../fonts/FreeSans9pt7b.h>
#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans24pt7b.h>

#define TFT_RST D2
#define TFT_RS  D4
#define TFT_CS  D8  // SS
#define TFT_SDI D7  // MOSI
#define TFT_CLK D5  // SCK
#define TFT_LED -1

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);

DHT dht(DHTPIN, DHTTYPE);

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;

const long utcOffsetInSeconds =  7 * 60 * 60; //3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

String ID = "01867";

int lineCount = 0;
int firstCheck = 1;
int firstStart = 1;
double timestamp = 0;

int tempOver = 29;

unsigned long times;
unsigned long timesPush;

unsigned long timesUpdate = 60000; //milisec
unsigned long timesPushEvery = 3600000; //milisec
int hours=99;
int minutes=99;

#define X_DEF 10
#define Y_DEF 10
int16_t x=X_DEF, y=Y_DEF, w, h; //constants for screen printing stuff

void drawScreen(float temperature, float humidity) {
  tft.clear();

  String s1 = "Temperature(°C):"; //Humidity
  tft.setGFXFont(&FreeSans12pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y = h;
  x = X_DEF;
  tft.drawGFXText(x, y, s1, COLOR_GREEN);

  String s2 = String(temperature, 2); //Humidity
  tft.setGFXFont(&FreeSans24pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y += h + 10;
  x = 50;
  tft.drawGFXText(x, y, s1, COLOR_GREEN);
  if(temperature > tempOver) tft.drawGFXText(x, y, s1, COLOR_RED);

  String s3 = "Humidity(%):"; //Humidity
  tft.setGFXFont(&FreeSans12pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y += h + 20;
  x = X_DEF;
  tft.drawGFXText(x, y, s1, COLOR_BLUE);

  String s4 = String(humidity, 2); //
  tft.setGFXFont(&FreeSans24pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y += h + 10;
  x = 50;
  tft.drawGFXText(x, y, s1, COLOR_BLUE);

  String s5 = "Device ID: "+ID; //
  tft.setGFXFont(&FreeSans9pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y += h + 20;
  x = 50;
  tft.drawGFXText(x, y, s1, COLOR_WHITE);
  
  // Draw first string in big font
//  String s1 = "6789";
//  tft.setGFXFont(&FreeSans24pt7b); // Set current font FreeSans9pt7b
//  tft.getGFXTextExtent(s1, x, y, &w, &h); // Get string extents
//  y = h; // Set y position to string height
//  tft.drawGFXText(x, y, s1, COLOR_RED); // Print string
//
//  // Draw second string in smaller font
//  tft.setGFXFont(&FreeSans12pt7b);  // Change font
//  String s2 = "Hello"; // Create string object
//  tft.getGFXTextExtent(s2, x, y, &w, &h); // Get string extents
//  y += h + 10; // Set y position to string height plus shift down 10 pixels
//  tft.drawGFXText(x, y, s2, COLOR_BLUE); // Print string
//  
//  // Draw third string in same font
//  String s3 = "World!"; // Create string object
//  y += h + 2; // Set y position to previous string height plus shift down 2 pixels
//  tft.drawGFXText(x, y, s3, COLOR_GREEN); // Print string
}

void setup() {

  ESP.wdtDisable(); ESP.wdtEnable(WDTO_8S);
//  dht.begin();
  Serial.begin(115200); Serial.println();
  Serial.println(LINE.getVersion());

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());

//  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);
  String LineText = "Sensor ID: " + ID + " Restart..";
  LINE.notify(LineText);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  /*
  This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
  Firewall that allows only GET and POST requests.
  
  Firebase.enableClassicRequest(firebaseData, true);
  */

   timeClient.begin();
   
   tft.begin();
   tft.clear();
 
}

void loop() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
 // t = 55;
 // h = 40;
 // f = 80;
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(500);
    return;
  }

   if ( t<0 || t>100 || h<0 || h>100) {
    Serial.println("Failed data error! ------------------" );
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.println(t);
    delay(1000);
    return;
  }
  
  float hif = dht.computeHeatIndex(f, h);
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

//  for (int i = 0; i <10; i++) {
//    ESP.wdtFeed();
//    delay(1000);
//  }
//  return;
  
  //OLED
  drawScreen(t, h);
  
  //OLED
  
  if (t > tempOver) {
    if(firstCheck || lineCount>=6) {
    
      lineCount = 0;
      firstCheck = 0;
      
      String LineText;
      String string1 = " ********Error******อุณหภูมิ เกินกำหนด! ขณะนี้ ";
      String string2 = " C°";
      String string3 = " ความชื้น";
      String string4 = " %";
      LineText = "SensorID:" + ID + string1 + t + string2  + string3 + h + string4;
      Serial.print("Line");
      Serial.println(LineText);
      LINE.notify(LineText);
    }
    lineCount++;
  } else {
    firstCheck = 1;
  }


  if(millis() - times > timesUpdate || firstStart)  {
    times = millis();    
    
    String path = "/Temp";
 
    Serial.println("------------------------------------");
    Serial.println("Set Timestamp test...");
  
    if (Firebase.setTimestamp(firebaseData, path + "/Timestamp"))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
  
      //Timestamp saved in millisecond, get its seconds from intData()
      Serial.print("TIMESTAMP (Seconds): ");
      Serial.println(firebaseData.intData());    
  
      //Or print the total milliseconds from doubleData()
      //Due to bugs in Serial.print in Arduino library, use printf to print double instead.
      printf("TIMESTAMP (milliSeconds): %.0lf\n", firebaseData.doubleData());
      
      timestamp = firebaseData.doubleData();
      printf("TIMESTAMP : %.0lf\n", timestamp);
  
      //Or print it from payload directly
      Serial.print("TIMESTAMP (milliSeconds): ");
      Serial.println(firebaseData.payload());
  
      //Due to some internal server error, ETag cannot get from setTimestamp
      //Try to get ETag manually
      Serial.println("ETag: " + Firebase.getETag(firebaseData, path + "/Set/Timestamp"));
      Serial.println("------------------------------------");
      Serial.println();
  
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
      timestamp = 0;
    }

    json.clear().add("id", ID).add("temperature", t).add("humidity", h).add("timestamp", timestamp);

    Serial.println("------------------------------------");
    Serial.println("Update test...");
    
    if (Firebase.updateNode(firebaseData, path + "/Realtime/"+ID, json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      //No ETag available
      Serial.print("VALUE: ");
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    
    if(millis() - timesPush > timesPushEvery || firstStart)  {
      timesPush = millis();
      
      //Also can use Firebase.push instead of Firebase.pushJSON
      //Json string is not support in v 2.6.0 and later, only FirebaseJson object is supported.
      
      if (Firebase.pushJSON(firebaseData, path + "/Push/"+ID, json))
      {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.print("PUSH NAME: ");
        Serial.println(firebaseData.pushName());
        Serial.println("ETag: " + firebaseData.ETag());
        Serial.println("------------------------------------");
        Serial.println();
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }
    }

    timeClient.update();
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());
    //Serial.println(timeClient.getFormattedTime());
    if(timeClient.getHours()== 3 || timeClient.getHours()== 5 ) {
      Serial.print("timeClient.getHours() "+ String(timeClient.getHours()) );
      //if(hours != timeClient.getHours() && minutes != timeClient.getMinutes() ) {
      if( hours != timeClient.getHours() ) {
        String first = "";
        if(hours == 99) { first = " ..."; }
        hours = timeClient.getHours();
        minutes = timeClient.getMinutes();
        
        String LineText;
        String string1 = " อุณหภูมิ ขณะนี้ ";
        String string2 = " C°";
        String string3 = " ความชื้น";
        String string4 = " %";
        LineText = "SensorID:" + ID + string1 + t + string2  + string3 + h + string4 + first;
        Serial.print("Line "+ String(hours) + String(minutes) );
        Serial.println(LineText);
        LINE.notify(LineText);
      }
    }
  
    firstStart= 0 ;
  }


  for (int i = 0; i <10; i++) {
    ESP.wdtFeed();
    delay(1000);
  }
}
