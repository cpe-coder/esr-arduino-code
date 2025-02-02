#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>

Servo servo;
#define powerPin D1
#define extractPin D2
#define boilPin D3
#define dryPin D4
#define littersPin D5
#define startExtractionPin D5

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
bool powerState = false;
bool extractState = false;
bool boilState = false;
bool dryState = false;
bool startExtractionState = false;
int littersSize = 0;


unsigned long sendDataPrevMillis = 0;

void setup() {
  servo.attach(D0);
  servo.write(0);
  pinMode(powerPin, OUTPUT);
  pinMode(extractPin, OUTPUT);
  pinMode(boilPin, OUTPUT);
  pinMode(dryPin, OUTPUT);
  pinMode(littersPin, OUTPUT);
  pinMode(startExtractionPin, OUTPUT);
  Serial.begin(9600);
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

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int sizeValue;

    if (Firebase.RTDB.getBool(&fbdo, "Controls/power")) {
      if (fbdo.dataType() == "boolean"){
      powerState = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + powerState + "(" + fbdo.dataType() + ")");
      digitalWrite(powerPin, powerState);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

      if (Firebase.RTDB.getBool(&fbdo, "Controls/extract")) {
        if (fbdo.dataType() == "boolean"){
          extractState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + extractState + "(" + fbdo.dataType() + ")");
          digitalWrite(extractPin, extractState);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

      if (Firebase.RTDB.getBool(&fbdo, "Controls/boil")) {
        if (fbdo.dataType() == "boolean"){
          boilState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + boilState + "(" + fbdo.dataType() + ")");
          digitalWrite(boilPin, boilState);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
      
      if (Firebase.RTDB.getBool(&fbdo, "Controls/dry")) {
        if (fbdo.dataType() == "boolean"){
          dryState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + dryState + "(" + fbdo.dataType() + ")");
          digitalWrite(dryPin, dryState);
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.getInt(&fbdo, "Sizes/litters")) {
          littersSize = fbdo.intData();
          Serial.print("Seccess: ");
          Serial.println(littersSize);
          sizeValue = littersSize;
         
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.getBool(&fbdo, "Controls/startExtraction")) {
        if (fbdo.dataType() == "boolean"){
          startExtractionState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startExtractionState + "(" + fbdo.dataType() + ")");
          digitalWrite(dryPin, startExtractionState);
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

    Serial.println("_______________________________________");
  }
}