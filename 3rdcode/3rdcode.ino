#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>


#define dryingLinearActuator1 D0
#define dryingLinearActuator2 D2
#define pushLinearActuator1 D3
#define pushLinearActuator2 D4
#define openLinearActuator1 D5
#define openLinearActuator2 D6
#define pulvorizer D7

#define WIFI_SSID "So Good"
#define WIFI_PASSWORD "helloworld"

#define API_KEY "AIzaSyDIUvTegr1EgYJ9qgw7lqKSV2UoG75HKRk"
#define DATABASE_URL "e-sugar-rush-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long sendDataPrevMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(dryingLinearActuator1, OUTPUT);
  pinMode(dryingLinearActuator2, OUTPUT);
  pinMode(pushLinearActuator1, OUTPUT);
  pinMode(pushLinearActuator2, OUTPUT);
  pinMode(openLinearActuator1, OUTPUT);
  pinMode(openLinearActuator2, OUTPUT);
  pinMode(pulvorizer, OUTPUT);
  digitalWrite(dryingLinearActuator1, HIGH);
  digitalWrite(dryingLinearActuator2, HIGH);
  digitalWrite(pushLinearActuator1, HIGH);
  digitalWrite(pushLinearActuator2, HIGH);
  digitalWrite(openLinearActuator1, HIGH);
  digitalWrite(openLinearActuator2, HIGH);
  digitalWrite(pulvorizer, HIGH);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase connection successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void dryingController(int boilSizeValue, bool isDrying) {
long startTime;
if (boilSizeValue != 0 && isDrying) {
long dryingTime = 20000 * boilSizeValue;
 while (millis() - startTime < dryingTime) {
        digitalWrite(dryingLinearActuator1, LOW);
        digitalWrite(dryingLinearActuator2, HIGH);
        delay(10000);  
        digitalWrite(dryingLinearActuator1, HIGH);
        digitalWrite(dryingLinearActuator2, LOW);
        delay(10000);  
    }

    digitalWrite(dryingLinearActuator1, HIGH);
    digitalWrite(dryingLinearActuator2, HIGH);
    Serial.println("Timer Ended. Both Relays OFF.");
  while (true);
}
}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int boilSizeValue;
    bool isDrying;

    if (Firebase.RTDB.getInt(&fbdo, "Sizes/boilSize")) {
          int boilSize = fbdo.intData();
          Serial.print("Seccess! Boil: ");
          Serial.println(boilSize);
          boilSizeValue = boilSize;
         
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

     if (Firebase.RTDB.getBool(&fbdo, "Pass/isDrying")) {
          bool boilSize = fbdo.boolData();
          Serial.print("Seccess! Boil: ");
          Serial.println(boilSize);
          isDrying = boilSize;
         
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }



    dryingController(boilSizeValue, isDrying); 

    Serial.println("_______________________________________");
  }
}




