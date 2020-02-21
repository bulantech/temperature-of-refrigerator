#include "FirebaseESP8266.h"
#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHT.h"
#define DHTPIN D3
#define DHTTYPE DHT11
//OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 16
#define SSID        "Nam&Nano_Wifi2.4"     // SSIDบ้าน
#define PASSWORD    ""                     // PASSบ้าน
//#define SSID        "Health_Office"       // SSID Office
//#define PASSWORD    "z038462300"         // Pass Office
#define LINE_TOKEN  "zbCOnOMLL8dNy1ClwK8MNNTuSS3TOiJF3suuVkh4Ri9"   // บรรทัดที่ 13 ใส่ รหัส TOKEN ที่ได้มาจากข้างบน
Adafruit_SSD1306 display(OLED_RESET); //OLED
//OLED
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;

const long utcOffsetInSeconds =  7 * 60 * 60; //3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define FIREBASE_HOST "temperature-of-refrigerator.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "6fDiOSK9ApIDH851m3vLmC65oG2G7g5uKFTdyhW4"

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;

//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
String ID = "01866";

int lineCount = 0;
int firstCheck = 1;
int tempOver = 25;

unsigned long times;
unsigned long timesPush;

unsigned long timesUpdate = 60000; //milisec
unsigned long timesPushEvery = 3600000; //milisec
unsigned long timesReadDHTEvery = 2000; //milisec
unsigned long timesReadDHT;

int hours=99;
int minutes=99;
int firstStart = 1;
double timestamp = 0;

void setup() {
   Serial.begin(115200); 
   Serial.println(LINE.getVersion());
   Serial.println("DHTxx test!");
   dht.begin();
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

  timeClient.begin();
}  
void loop() {
  if(millis() - timesReadDHT > timesReadDHTEvery || firstStart)  {
    timesReadDHT =  millis();  
  
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   float f = dht.readTemperature(true);
   if (isnan(h) || isnan(t) || isnan(f)) {
     Serial.println("Failed to read from DHT sensor!");
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
   float hi = dht.computeHeatIndex(f, h);
   Serial.print("Humidity: "); 
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperature: "); 
   Serial.print(t);
   Serial.print(" *C ");
   Serial.print(f);
   Serial.print(" *F\t");
   Serial.print("Heat index: ");
   Serial.print(hi);
   Serial.println(" *F");
   delay(2000);
    
  //OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
    display.clearDisplay(); // ลบภาพในหน้าจอทั้งหมด
    display.setTextSize(1); // กำหนดขนาดตัวอักษร
    display.setTextColor(WHITE);
    display.setCursor(1,1); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
    display.println(" temperature&Humidity ");
      display.setTextSize(1); // กำหนดขนาดตัวอักษร
    display.setTextColor(WHITE);
    display.setCursor(15,10); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
    display.println("PCU CODE :"+ID);

    display.setCursor(20,20);
    display.setTextSize(2.5); 
    display.print(t);
    display.println(" C");
    display.setCursor(20,39);
    display.setTextSize(2.5);
    
    //display.setTextColor(BLACK, WHITE); //กำหนดข้อความสีขาว ฉากหลังสีดำ
    display.print(h);
    display.println(" %");
    display.setCursor(10,56);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    //display.setTextColor(BLACK, WHITE); //กำหนดข้อความสีขาว ฉากหลังสีดำ
    display.println("Edit by:0899357371");
    display.display();
  //OLED

  if (t > tempOver) {
    if(firstCheck || lineCount>=30) {
    
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

  } //if(millis() - timesReadDHT > timesReadDHTEvery || firstStart)  {
  
//  for (int i = 0; i <20; i++) {
//    ESP.wdtFeed();
//    delay(1000);
//  }
}
