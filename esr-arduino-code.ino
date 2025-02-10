#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>

Servo servo;
#define powerPin D0
#define extractPin D1
#define boilPin D2
#define dryPin D3
#define pumpToBoilPin D4
#define pumpToJuicePin D5
#define rotateBoiler D6
#define dryingStart D7


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
bool powerState = false;
bool extractState = false;
bool boilState = false;
bool startTransferingState = false;
bool dryState = false;
bool startExtractionState = false;
int boilSize = 0;
int transferSize = 0;


unsigned long sendDataPrevMillis = 0;

void setup() {
  servo.attach(D0);
  servo.write(0);
  pinMode(powerPin, OUTPUT);
  pinMode(extractPin, OUTPUT);
  pinMode(boilPin, OUTPUT);
  pinMode(dryPin, OUTPUT);
  pinMode(pumpToBoilPin, OUTPUT);
  pinMode(pumpToJuicePin, OUTPUT);
  pinMode(rotateBoiler, OUTPUT);
  pinMode(dryingStart, OUTPUT);
  digitalWrite(powerPin, LOW);
  digitalWrite(extractPin, LOW);
  digitalWrite(dryPin, LOW);
  digitalWrite(pumpToBoilPin, LOW);
  digitalWrite(pumpToJuicePin, LOW);
  digitalWrite(rotateBoiler, LOW);
  digitalWrite(rotateBoiler, LOW);
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

void pumpToBoiler(int boilSizeValue, bool isExtractionStart) {
  if (boilSizeValue != 0 && isExtractionStart){
      int time = 40000 * boilSizeValue;
      int cookingTime = 60000 * boilSizeValue;
      int dryingTime = 30000 * boilSizeValue;
      digitalWrite(pumpToBoilPin, HIGH);
      Serial.println("Pump to Boiler is working...");
      Firebase.RTDB.setInt(&fbdo, "Timer/juiceToBoiler", time);
      delay(time);
      digitalWrite(pumpToBoilPin, LOW);
      Firebase.RTDB.setBool(&fbdo, "Controls/startExtraction", false);
      Serial.println("Pump to Boiler is STOP...");
      Firebase.RTDB.setInt(&fbdo, "Timer/cooking", cookingTime);
      Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", true);
      digitalWrite(rotateBoiler, HIGH);
      Serial.println("Boiler is working...");
      delay(cookingTime);
      Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", false);
      digitalWrite(rotateBoiler, LOW);
      Serial.println("Boiler is stop...");
      Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", true);
      Serial.println("Drying is working...");
      digitalWrite(dryingStart, HIGH);
      Firebase.RTDB.setInt(&fbdo, "Pass/isDrying", dryingTime);
      delay(dryingTime);
       digitalWrite(dryingStart, LOW);
      Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", false);
      Firebase.RTDB.setBool(&fbdo, "Pass/dryingStop", true);
      Serial.println("Drying is stop...");
    }
}

void pumpToJuiceStorage(int transferSizeValue, bool isTransferingStart) {
   if (transferSizeValue != 0 && isTransferingStart){
      digitalWrite(pumpToJuicePin, HIGH);
      Serial.println("Pump to juice storage is working...");
      int time = 40000 * transferSizeValue;
      Firebase.RTDB.setInt(&fbdo, "Timer/juiceToJuiceStorage", time);
      delay(time);
      Serial.println(time);
      digitalWrite(pumpToJuicePin, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Controls/startTransfering", false);
      Serial.println("Pump to Juice Storage is STOP...");
    }
}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int boilSizeValue;
    bool isExtractionStart;
    int transferSizeValue;
    bool isTransferingStart;

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

      if (Firebase.RTDB.getInt(&fbdo, "Sizes/boilSize")) {
          boilSize = fbdo.intData();
          Serial.print("Seccess! Boil: ");
          Serial.println(boilSize);
          boilSizeValue = boilSize;
         
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.getInt(&fbdo, "Sizes/transferSize")) {
          transferSize = fbdo.intData();
          Serial.print("Seccess! Transfer: ");
          Serial.println(transferSize);
          transferSizeValue = transferSize;
         
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.getBool(&fbdo, "Controls/startExtraction")) {
        if (fbdo.dataType() == "boolean"){
          startExtractionState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startExtractionState + "(" + fbdo.dataType() + ")");
          digitalWrite(pumpToBoilPin, startExtractionState);
          isExtractionStart = startExtractionState;
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.getBool(&fbdo, "Controls/startTransfering")) {
        if (fbdo.dataType() == "boolean"){
          startTransferingState = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startTransferingState + "(" + fbdo.dataType() + ")");
          digitalWrite(pumpToJuicePin, startTransferingState);
          isTransferingStart = startTransferingState;
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

      pumpToBoiler(boilSizeValue, isExtractionStart);
      pumpToJuiceStorage(transferSizeValue,isTransferingStart );

 

    Serial.println("_______________________________________");
  }
}




