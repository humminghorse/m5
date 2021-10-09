#include <M5StickCPlus.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <IOXhop_FirebaseESP32.h>
#include "ArduinoJson.h"
#define WIFI_SSID "D80F99DAA5B4-2G"
#define WIFI_PASSWORD "2215001705815"
#define FIREBASE_DB_URL "https://titech-2020-imahashi.firebaseio.com/" 

WiFiMulti WiFiMulti;
int count = 1;  

#define SAMPLE_PERIOD 20    // サンプリング間隔(ミリ秒)
#define SAMPLE_SIZE 150     // 20ms x 150 = 3秒


float interval, preInterval;
int stepcount,state,laststate=0;
float total,threshold,hysteresis=0;
float acc_x = 0.0F; 
float acc_y = 0.0F;  
float acc_z = 0.0F;

 
void setup() {
  M5.begin(); 
  M5.Lcd.setRotation(3);
//  M5.Lcd.setCursor(0, 0, 2);
  
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  M5.Lcd.print("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(1000);
  }
  M5.Lcd.println("");
  M5.Lcd.println("Connected to");
  M5.Lcd.println(WiFi.localIP());
  delay(500);
  
  Firebase.begin(FIREBASE_DB_URL);   // ④
}

void loop() {
  M5.update();
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    M5.IMU.getAccelData(&acc_x,&acc_y,&acc_z);
    
    float accel;
    accel = sqrt( sq(acc_x) + sq(acc_y) + sq(acc_z));
    // 歩数カウントしきい値設定
    if (i != 100) {
      total += accel;
    } else {
      threshold = total/i;
      hysteresis = threshold / 10;
      total = 0;
    }
    
    // 歩数カウントしきい値判定
    if ( accel > (threshold + hysteresis) ) {
      state = true;
    } else if ( accel < (threshold - hysteresis) ) {
      state = false;
    }
    
    // 歩数カウント
    if (laststate == false && state == true) {
      stepcount++;
      laststate = state;
    } else if (laststate == true && state == false) {
      laststate = state;
    }
    Serial.print(i);
    Serial.print("\t");
    Serial.print(accel);
    Serial.print("\t");
    Serial.print("steps:");
    Serial.print(stepcount);
    Serial.print("\n");
  }
  // 歩数の出力
  Serial.print("Steps:");
  Serial.print(stepcount);
  Serial.print("\t");
  Firebase.setInt("/button", stepcount);  
 
}
