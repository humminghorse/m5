#include <M5StickCPlus.h>
#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <IOXhop_FirebaseESP32.h>
#include "ArduinoJson.h"
#include "time.h"

#define WIFI_SSID "D80F99DAA5B4-2G"
#define WIFI_PASSWORD "2215001705815"
#define FIREBASE_DB_URL "https://titech-2020-imahashi.firebaseio.com/" 

float accX = 0;
float accY = 0;
float accZ = 0;

const int numOfSample = 20;
float sample[numOfSample];
float threshold = 0;
int countSample = 0;
float range = 50.0;

uint8_t countStep = 0;

int loopCount = 0;

float getDynamicThreshold(float *s) {
    float maxVal = s[0];
    float minVal = s[0];
    for (int i=1; i<sizeof(s); i++) {
        maxVal = max(maxVal, s[i]);
        minVal = min(minVal, s[i]);
    }
    return (maxVal + minVal) / 2.0;
}


float getFilterdAccelData() {
    static float y[2] = {0};
    M5.IMU.getAccelData(&accX,&accY,&accZ);
    y[1] = 0.8 * y[0] + 0.2 * (abs(accX) + abs(accY) + abs(accZ)) * 1000.0;
    y[0] = y[1];
    return y[1];
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(0, 0, 2);
  M5.IMU.Init();
  
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  M5.Lcd.print("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(1000);
  }
  M5.Lcd.println("");
  M5.Lcd.println("Connected to");
  M5.Lcd.println(WiFi.localIP());

  Firebase.begin(FIREBASE_DB_URL);

  configTime(9 * 3600, 0, "ntp.nict.jp"); // Set ntp time to local

  sample[countSample] = getFilterdAccelData();
}

void loop() {
  M5.update();
  
  delay(100);
  countSample++;
  sample[countSample] = getFilterdAccelData();
  if (abs(sample[countSample] - sample[countSample-1]) < range) {
    sample[countSample] = sample[countSample-1];
    countSample--;
  }
  if (sample[countSample] < threshold && sample[countSample-1] > threshold) {
    countStep++;
  }
  if (countSample == numOfSample) {
    threshold = getDynamicThreshold(&sample[0]);
    countSample = 0;
    sample[countSample] = getFilterdAccelData(); }
  
  loopCount++;

  if (loopCount == 100) {
    struct tm timeInfo;
    getLocalTime(&timeInfo);
    String year = (String) (timeInfo.tm_year + 1900);
    String month = (String) (timeInfo.tm_mon + 1);
    String day = (String) (timeInfo.tm_mday);
    String today = year + month + day;
    
    Firebase.setInt("/" + today + "/steps", countStep);
    
    M5.Lcd.print((String) countStep + ", ");
    
    loopCount = 0;
  }
  
  
}
