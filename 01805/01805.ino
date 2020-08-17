//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h" // mobizt/Firebase-ESP8266
#include <TridentTD_LineNotify.h> // TridentTD/TridentTD_LineNotify
#include <NTPClient.h> //Li เวลาจากอินเตอร์เนต
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//
#define SSID        "Nam&Nano_Wifi2.4"     // SSIDบ้าน
#define PASSWORD    ""                     // PASSบ้าน
//#define SSID        "Health_Office"       // SSID Office
//#define PASSWORD    "z038462300"         // Pass Office

//#define SSID        "TrueGigatexFiber_2.4G_A48"     // SSIDบ้าน
//#define PASSWORD    "dn3p93tk"                     // PASSบ้าน

// ตัวแปรเปลี่ยนค่า -------------------
String ID = "01804";
int calVal=-2; // ค่าคาลิเรท

const int line_send_1 = 9; //ส่งไลน์ครั้งที่1
const int line_send_2 = 15; //ส่งไลน์ครั้งที่2
int lineCount = 0;
int firstCheck = 1;
int firstStart = 1;
double timestamp = 0;

int tempOver = 7;
unsigned long times;
unsigned long timesPush=99;

unsigned long timesUpdate = 90000; //milisec
unsigned long timesPushEvery = 3600000; //milisec
int hours=99;
int minutes=99;

int sendLineEvery = 1200; //sec

float hNow;
float tNow;

const int lcdShowSecond=120; //sec
int lcdShowCount=0;

String strLastTemp="";

#define FIREBASE_HOST "temp-chonburi-sso.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "DncJwCCmiLqu70qzGJUI9Ch4lLyxA963I4PNzRFz"

FirebaseData firebaseData;
FirebaseJson json;

#define LINE_TOKEN  "tLizkeBHy33vJRk2WFl4a0gaEMjyocuwQTHu8gM0azo"   // ใส่ รหัส TOKEN ที่ได้มาจากข้างบน

// อ่านอุณหภูมิ
#include "DHT.h" // adafruit/DHT-sensor-library
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// จอ lcd
#include <SPI.h> //การเรียกใช้ Libary
#include <TFT_22_ILI9225.h> //Nkawu/TFT_22_ILI9225
#include <../fonts/FreeSans9pt7b.h>
#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans24pt7b.h>

#define TFT_RST D2 //ประกาศค่าคงที่
#define TFT_RS  D4
#define TFT_CS  D8  // SS
#define TFT_SDI D7  // MOSI
#define TFT_CLK D5  // SCK
#define TFT_LED -1  //set 0 if wired to +5V directly -> D3=0 is not possible !!

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)
// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);


// ตัวแปร globle
const long utcOffsetInSeconds =  7 * 60 * 60; //3600; //ประกาศค่าคงที่ const ประกาศได้ทุกที่
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define X_DEF 10
#define Y_DEF 10
int16_t x=X_DEF, y=Y_DEF, w, h; //constants for screen printing stuff


void drawScreen(float temperature, float humidity) {
  
  tft.clear(); //เครียหน้าจอ
  tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_BLACK);
  String s1 = "Temperature(C):"; //Humidity
  tft.setGFXFont(&FreeSans9pt7b);
  tft.getGFXTextExtent(s1, x, y, &w, &h);
  y = h;
  x = X_DEF;
  tft.drawGFXText(x, y, s1, COLOR_GREEN);

  String s2 = String(temperature, 2); //Humidity
  tft.setGFXFont(&FreeSans24pt7b);
  tft.getGFXTextExtent(s2, x, y, &w, &h);
  y += h + 10;
  x = 50;
  tft.drawGFXText(x, y, s2, COLOR_GREEN);
  if(temperature > tempOver) tft.drawGFXText(x, y, s2, COLOR_RED);

  String s3 = "Humidity(%):"; //Humidity
  tft.setGFXFont(&FreeSans9pt7b);
  tft.getGFXTextExtent(s3, x, y, &w, &h);
  y += h + 20;
  x = X_DEF;
  tft.drawGFXText(x, y, s3, COLOR_BLUE);

  String s4 = String(humidity, 2); //
  tft.setGFXFont(&FreeSans24pt7b);
  tft.getGFXTextExtent(s4, x, y, &w, &h);
  y += h + 10;
  x = 50;
  tft.drawGFXText(x, y, s4, COLOR_BLUE);

  String s5 = "Device ID: "+ID; //
  tft.setGFXFont(&FreeSans9pt7b);
  tft.getGFXTextExtent(s5, x, y, &w, &h);
  y += h + 30;
  x = 10;
  tft.drawGFXText(x, y, s5, COLOR_WHITE);

}

#include <WifiLocation.h>
const char* googleApiKey = "AIzaSyAtioUDNbceW3OZ5XatI-ylRJrCTCPnOng"; //AIzaSyCx5vN4VV2ePypTBi1FFl9Gk94JKqZZT4U //AIzaSyAtioUDNbceW3OZ5XatI-ylRJrCTCPnOng //bulantech
//WifiLocation location(googleApiKey);
float latitude=0;
float longitude=0;
int accuracy=0;
void setup() {
  ESP.wdtDisable(); ESP.wdtEnable(WDTO_8S); //เฝ้าไม่ให้contolerทำงานค้าง 8วิ

  Serial.begin(115200); Serial.println(); //อันดับแรกต้องกำหนด sn port
  
  Serial.println(LINE.getVersion());

  tft.begin(); //เริ่มต้น Libary จอLCD
//  tft.clear(); //เครียหน้าจอ

  tft.clear(); //เครียหน้าจอ
  tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_RED);
  tft.setFont(Terminal6x8);
  tft.setBackgroundColor(COLOR_RED);
  tft.drawText(10, 10, "WiFi connect..", COLOR_WHITE);  
  
  WiFi.begin(SSID, PASSWORD); //เรียกฟังชั่นการเชื่อมต่อ wifi ให้ใส่ ssid กับ pass wifi
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) { //ให้แสดง Status การเชื่อมตอ่
    Serial.print("."); // ถ้าเชื่อมไม่ได้ให้ .....
    delay(500);
  }
  Serial.printf("\nWiFi connected\nIP : "); 
  Serial.println(WiFi.localIP()); //แสดง IP ที่ได้รับ

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);
//  String LineText = "Sensor ID: " + ID + " Restart..";
//  LINE.notify(LineText);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //ใส่ urlของProject,Token
  Firebase.reconnectWiFi(true); //ให้เชื่อมต่อใหม่เมื่อหลุด

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024); //กำหนด Buffer ส่งข้อมูล

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024); //กำหนด Buffer ตัวรับข้อมูล

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

   timeClient.begin(); //Setใช้งาน NTP

   dht.begin(); //เริ่มต้น LIbary อ่านอุณหภูมิ

  tft.clear(); //เครียหน้าจอ
  tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);
  tft.setFont(Terminal6x8);  
  tft.setBackgroundColor(COLOR_GREEN);
  tft.drawText(10, 10, "get Geographic..", COLOR_WHITE);
  
  getGeographicPosition(); //อ่าน Lat,long จากwifi Location
  Serial.println(latitude, 7);
  Serial.println(longitude, 7);
  Serial.println(accuracy);
  if(!latitude) {
    Serial.println("Not get Geographic Position ESP.reset()!");
    delay(5000);
    ESP.reset();    
  }

  tft.clear(); //เครียหน้าจอ
  tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_BLUE);
  tft.setFont(Terminal6x8);
  tft.setBackgroundColor(COLOR_BLUE);
  tft.drawText(10, 10, "Setup ok!", COLOR_WHITE);
  
}

void getGeographicPosition() {
  WifiLocation location(googleApiKey);
  
  Serial.println("Location request data..");
//  WifiLocation location(googleApiKey);
  location_t loc = location.getGeoFromWiFi();
//  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson());

  latitude = loc.lat;
  longitude = loc.lon;
  accuracy = loc.accuracy;
  
  String Latitude = String(latitude, 7);
  String Longitude = String(longitude, 7);
  String Accuracy = String(accuracy);
  
  Serial.println("Latitude: " + Latitude);
  Serial.println("Longitude: " + Longitude);
  Serial.println("Accuracy: " + Accuracy); 
}

void loop() {
//tft.clear(); //เครียหน้าจอ
  // test
//  Serial.println(F("loop().."));
//  delay(5000);
//  return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
//  t = 10;
//  h = 40;
//  f = 80;
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(1000);
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
  Serial.print(F(" "));
  Serial.print(t+calVal);

  t = t+calVal;//อ่านค่าที่ได้จริง ไปบวกค่าที่คาริเบท
  
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //lcd
  if(!lcdShowCount) {
    
    drawScreen(t, h);
    lcdShowCount = lcdShowSecond;
  }
  --lcdShowCount;  
  //lcd end

//  delay(3000); return; //test
  
  // check Temperature
  if (t > tempOver) {
//    Serial.println("========== (t > tempOver)");
    if(firstCheck || lineCount>=sendLineEvery) { //5 นาที
//      Serial.println("========== (firstCheck || lineCount>=sendLineEvery)");
      lineCount = 0;
      firstCheck = 0;
      
      String LineText;
      String string1 = " ********Error******อุณหภูมิ เกินกำหนด! ขณะนี้ ";
      String string2 = " C°";
      String string3 = " ความชื้น";
      String string4 = " %";
      LineText = "SensorID:" + ID + string1 + t + string2  + string3 + h + string4;

      //แสดงค่าที่แล้ว เมื่อแจ้งเตือนครั้งแรก
      if(strLastTemp != "") {
        LineText += strLastTemp;
        strLastTemp = "";
      }
            
      Serial.print("Line");
      Serial.println(LineText);
      LINE.notify(LineText); //comment for test
 
    }
    lineCount++;
  } else {
    if(!firstCheck) {
      if(++lineCount >= sendLineEvery) {
        firstCheck = 1;
        lineCount = 0;
      }
    }
    else {
      firstCheck = 1;
      String str = " อุณหภูมิเดิม:";
      strLastTemp = str + t; //เก็บค่าปกติล่าสุด
    }
    
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
      //Serial.println("ETag: " + Firebase.getETag(firebaseData, path + "/Set/Timestamp"));
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

    json.clear().add("id", ID).add("temperature", t).add("humidity", h).add("timestamp", timestamp).add("latitude", latitude).add("longitude", longitude).add("accuracy", accuracy);
//    json.clear().add("id", ID).add("temperature", t).add("humidity", h).add("timestamp", timestamp);

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
    

    timeClient.update();
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());
    //Serial.println(timeClient.getFormattedTime());

    // ส่ง line ตามเวลาที่กำหนด
    if(timeClient.getHours()== line_send_1 || timeClient.getHours()== line_send_2 ) {
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
        LINE.notify(LineText); //comment for test
      }
    }

//    if(millis() - timesPush > timesPushEvery || firstStart)  {
    if(timesPush != timeClient.getHours())  {
      timesPush = timeClient.getHours();
      
      //Also can use Firebase.push instead of Firebase.pushJSON
      //Json string is not support in v 2.6.0 and later, only FirebaseJson object is supported.
      
      if (Firebase.pushJSON(firebaseData, path + "/Push/"+ID, json))
      {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.print("PUSH NAME: ");
        Serial.println(firebaseData.pushName());
        //Serial.println("ETag: " + firebaseData.ETag());
        Serial.println("------------------------------------xxxxxxxxxxxxxxxxxxxxxxxxx");
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
  
    firstStart= 0 ;
  }

  delay(1000);
}
