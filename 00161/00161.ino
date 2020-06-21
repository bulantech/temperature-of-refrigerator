//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h" // mobizt/Firebase-ESP8266
#include <TridentTD_LineNotify.h> // TridentTD/TridentTD_LineNotify
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//#define SSID        "Nam&Nano_Wifi2.4"     // SSIDบ้าน
//#define PASSWORD    ""                     // PASSบ้าน
//#define SSID        "Health_Office"       // SSID Office
//#define PASSWORD    "z038462300"         // Pass Office

// bulantech
#define SSID        "true_home2G_350"     // SSIDบ้าน
#define PASSWORD    "88f02350"                     // PASSบ้าน

#define FIREBASE_HOST "temperature-of-refrigerator.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "6fDiOSK9ApIDH851m3vLmC65oG2G7g5uKFTdyhW4"

//#define LINE_TOKEN  "zbCOnOMLL8dNy1ClwK8MNNTuSS3TOiJF3suuVkh4Ri9"   // บรรทัดที่ 13 ใส่ รหัส TOKEN ที่ได้มาจากข้างบน 
#define LINE_TOKEN  "pcYETzb9kUJI4kieSLQ2VGkjXqKm1BHQxdWUXBzLPVS"   // บรรทัดที่ 13 ใส่ รหัส TOKEN ที่ได้มาจากข้างบน
//char LINE_TOKEN[45] = "pcYETzb9kUJI4kieSLQ2VGkjXqKm1BHQxdWUXBzLPVS";

#include "DHT.h" // adafruit/DHT-sensor-library
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include "SPI.h"
#include "TFT_22_ILI9225.h" //Nkawu/TFT_22_ILI9225
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

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;

const long utcOffsetInSeconds =  7 * 60 * 60; //3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

String ID = "00161";

int lineCount = 0;
int firstCheck = 1;
int firstStart = 1;
double timestamp = 0;

int tempOver = 7;

unsigned long times;
unsigned long timesPush=99;

unsigned long timesUpdate = 60000; //milisec
unsigned long timesPushEvery = 3600000; //milisec
int hours=99;
int minutes=99;

int sendLineEvery = 1200; //sec

float hNow;
float tNow;

#define X_DEF 10
#define Y_DEF 10
int16_t x=X_DEF, y=Y_DEF, w, h; //constants for screen printing stuff

const int lcdShowSecond=120; //sec
int lcdShowCount=0;

#include <WifiLocation.h>
const char* googleApiKey = "AIzaSyAtioUDNbceW3OZ5XatI-ylRJrCTCPnOng";
//WifiLocation location(googleApiKey);
float latitude=0;
float longitude=0;
int accuracy=0;

#include <ArduinoJson.h>  // V 5.13.4
#include <WiFiManager.h> // v.0.15.0 https://github.com/tzapu/WiFiManager
#define SW_PIN  0
//#define LED 13
#define LED 2 //nodemcu v3

String line_token;
//String googleApiKey;
int calVal;
char google_Api_Key[45] = ""; //39
char cal_val[4] = "0";
bool shouldSaveConfig = false;

void drawScreen(float temperature, float humidity) {
  tft.clear();

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

void ledBlink() {
  digitalWrite(LED, 0); //on
  delay(10);
  digitalWrite(LED, 1); //off
}

void setup() {

  ESP.wdtDisable(); ESP.wdtEnable(WDTO_8S);

  Serial.begin(115200); Serial.println();

  pinMode(SW_PIN, INPUT_PULLUP);
//  pinMode(SW_DET, INPUT_PULLUP);
  pinMode(LED, OUTPUT); 
  
  Serial.println(LINE.getVersion());

//  WiFi.begin(SSID, PASSWORD);
//  Serial.printf("WiFi connecting to %s\n",  SSID);
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
//    delay(500);
//  }
//  Serial.printf("\nWiFi connected\nIP : ");
//  Serial.println(WiFi.localIP());

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  Serial.println("Reset wifi config?:");
  for(int i=5; i>0; i--){
    Serial.print(String(i)+" "); 
    ledBlink();
    delay(1000);
  }
  
  //reset saved settings
  if(digitalRead(SW_PIN) == LOW) // Press button
  {
    Serial.println();
    Serial.println("Reset wifi config");
    digitalWrite(LED, 0); //on
    wifiManager.resetSettings(); 
    SPIFFS.format();
  }    

//  wifiManager.autoConnect("AutoConnectAP");
//  Serial.println("connected...yeey :)");

  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
        
//          strcpy(LINE_TOKEN, json["LINE_TOKEN"]);
//          strcpy(google_Api_Key, json["google_Api_Key"]);
          strcpy(cal_val, json["cal_val"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
       
//  WiFiManagerParameter custom_LINE_TOKEN("LINE_TOKEN", "LINE_TOKEN", LINE_TOKEN, 43);
//  WiFiManagerParameter custom_google_Api_Key("google_Api_Key", "google_Api_Key", google_Api_Key, 39);
  WiFiManagerParameter custom_cal_val("cal_val", "cal_val", cal_val, 2);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
//  WiFiManagerParameter line_text("<br><span>Line token</span>");
//  wifiManager.addParameter(&line_text);
//  wifiManager.addParameter(&custom_LINE_TOKEN);

//  WiFiManagerParameter google_Api_Key_text("<br><span>Google API Key</span>");
//  wifiManager.addParameter(&google_Api_Key_text);
//  wifiManager.addParameter(&custom_google_Api_Key);

  WiFiManagerParameter cal_val_text("<br><span>Calibrate Value</span>");
  wifiManager.addParameter(&cal_val_text);
  wifiManager.addParameter(&custom_cal_val);

  if (!wifiManager.autoConnect("ESPwifi")) {
    Serial.println("failed to connect and hit timeout");
//    delay(3000);
    for(int i=0; i<10; i++) {
      digitalWrite(LED_BUILTIN, 0); //on
      delay(50);
      digitalWrite(LED_BUILTIN, 1); //off
      delay(500); 
    }
    
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
//  strcpy(LINE_TOKEN, custom_LINE_TOKEN.getValue());
//  strcpy(google_Api_Key, custom_google_Api_Key.getValue());
  strcpy(cal_val, custom_cal_val.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
//    Serial.println("LINE_TOKEN -> "+String(LINE_TOKEN));
//    json["LINE_TOKEN"] = LINE_TOKEN;
//    json["google_Api_Key"] = google_Api_Key;
    json["cal_val"] = cal_val;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

//  line_token = String(LINE_TOKEN);
//  googleApiKey = String(google_Api_Key);
  calVal = String(cal_val).toInt();

  Serial.println();
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());

//  Serial.print("line_token: ");
//  Serial.println(line_token);
//  Serial.print("googleApiKey: ");
//  Serial.println(googleApiKey);
  Serial.print("calVal: ");
  Serial.println(calVal);
  // end WifiManager  

  
//  // กำหนด Line Token
//  LINE.setToken(LINE_TOKEN);
  LINE.setToken(line_token);
//  String LineText = "Sensor ID: " + ID + " Restart..";
//  LINE.notify(LineText);

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

   dht.begin();
   
   tft.begin();
   tft.clear();

  getGeographicPosition();
  Serial.println(latitude, 7);
  Serial.println(longitude, 7);
  Serial.println(accuracy);
  if(!latitude) {
    Serial.println("Not get Geographic Position ESP.reset()!");
    delay(5000);
    ESP.reset();    
  }
  
}

void getGeographicPosition() {
  Serial.println("Location request data..");
  WifiLocation location(googleApiKey);
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
  
String strLastTemp="";

void loop() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
 // t = 55;
 // h = 40;
 // f = 80;
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
  Serial.print(t+calVal);
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
  
//  //OLED
//  if(tNow != t) {
//    tNow = t;
//    drawScreen(t, h);
//  }

  //OLED
  if(!lcdShowCount) {
    drawScreen(t, h);
    lcdShowCount = lcdShowSecond;
  }
  --lcdShowCount;  
  //OLED end

  // check Temperature
  if (t > tempOver) {
    if(firstCheck || lineCount>=sendLineEvery) { //5 นาที
    
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
      LINE.notify(LineText);
 
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
    
    if(timeClient.getHours()== 9 || timeClient.getHours()== 15 ) {
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


//  for (int i = 0; i <10; i++) {
//    ESP.wdtFeed();
//    delay(1000);
//  }
  delay(1000);
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
