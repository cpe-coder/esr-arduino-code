#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>

#define powerPin D0
#define extractPin D1
#define boilPin D2
#define rotateBoiler D3
#define dryPin D4
#define pumpToBoilPin D5
#define pumpToJuicePin D6
#define dryingStart D7
#define pushButton D8

#define WIFI_SSID "So Good"
#define WIFI_PASSWORD "helloworld"

#define API_KEY "AIzaSyDIUvTegr1EgYJ9qgw7lqKSV2UoG75HKRk"
#define DATABASE_URL "e-sugar-rush-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
int boilSize = 0;
int transferSize = 0;

bool lastButtonState = LOW;
bool currentButtonState;


unsigned long sendDataPrevMillis = 0;

void setup() {
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(powerPin, OUTPUT);
  pinMode(extractPin, OUTPUT);
  pinMode(boilPin, OUTPUT);
  pinMode(dryPin, OUTPUT);
  pinMode(pumpToBoilPin, OUTPUT);
  pinMode(pumpToJuicePin, OUTPUT);
  pinMode(rotateBoiler, OUTPUT);
  pinMode(dryingStart, OUTPUT);
  digitalWrite(powerPin, HIGH);
  digitalWrite(extractPin, HIGH);
  digitalWrite(dryPin, HIGH);
  digitalWrite(pumpToBoilPin, HIGH);
  digitalWrite(pumpToJuicePin, HIGH);
  digitalWrite(rotateBoiler, HIGH);
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

void emergencyButton() {
  currentButtonState = digitalRead(pushButton);

  if (currentButtonState == LOW && lastButtonState == HIGH) {
      digitalWrite(extractPin, HIGH);
      digitalWrite(dryPin, HIGH);
      digitalWrite(pumpToBoilPin, HIGH);
      digitalWrite(pumpToJuicePin, HIGH);
      digitalWrite(rotateBoiler, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Controls/boil", false);
      Firebase.RTDB.setBool(&fbdo, "Controls/dry", false);
      Firebase.RTDB.setBool(&fbdo, "Controls/extract", false);
      Firebase.RTDB.setBool(&fbdo, "Controls/startExtraction", false);
      Firebase.RTDB.setBool(&fbdo, "Controls/startTransfering", false);
      Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", false);
      Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", false);
  }

  lastButtonState = currentButtonState;
}

void pumpToBoiler(int boilSizeValue, bool isExtractionStart) {
  if (boilSizeValue != 0 && isExtractionStart){
      int time = 40000 * boilSizeValue;
      int cookingTime = 60000 * boilSizeValue;
      int dryingTime = 30000 * boilSizeValue;
      digitalWrite(pumpToBoilPin, LOW);
      Serial.println("Pump to Boiler is working...");
      Firebase.RTDB.setInt(&fbdo, "Timer/juiceToBoiler", time);
      delay(time);
      digitalWrite(pumpToBoilPin, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Controls/startExtraction", false);
      Serial.println("Pump to Boiler is STOP...");
      Firebase.RTDB.setInt(&fbdo, "Timer/cooking", cookingTime);
      Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", true);
      digitalWrite(rotateBoiler, LOW);
      Serial.println("Boiler is working...");
      delay(cookingTime);
      Firebase.RTDB.setBool(&fbdo, "Pass/isCooking", false);
      digitalWrite(rotateBoiler, HIGH);
      Serial.println("Boiler is stop...");
      Firebase.RTDB.setBool(&fbdo, "Pass/isTransferringToDrying", true);
      delay(15000);
      Firebase.RTDB.setBool(&fbdo, "Pass/isTransferringToDrying", false);
      Firebase.RTDB.setInt(&fbdo, "Timer/drying", dryingTime);
      Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", true);
      Serial.println("Drying is working...");
      digitalWrite(dryingStart, LOW);
      delay(dryingTime);
      digitalWrite(dryingStart, HIGH);
      Firebase.RTDB.setBool(&fbdo, "Pass/isDrying", false);
      Firebase.RTDB.setBool(&fbdo, "Pass/dryingStop", true);
      Serial.println("Drying is stop...");
    }
}

void pumpToJuiceStorage(int transferSizeValue, bool isTransferingStart) {
   if (transferSizeValue != 0 && isTransferingStart){
      digitalWrite(pumpToJuicePin, LOW);
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
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int boilSizeValue;
    bool isExtractionStart;
    int transferSizeValue;
    bool isTransferingStart;

    if (Firebase.RTDB.getBool(&fbdo, "Controls/power")) {
      if (fbdo.dataType() == "boolean"){
      bool powerStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + powerStateStr + "(" + fbdo.dataType() + ")");
      bool powerState = (powerStateStr == false) ? HIGH : LOW;
      digitalWrite(powerPin, powerState);   
      

      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getBool(&fbdo, "Controls/extract")) {
      if (fbdo.dataType() == "boolean"){
        bool extractStateStr = fbdo.boolData();
        Serial.println("Seccess: " + fbdo.dataPath() + ": " + extractStateStr + "(" + fbdo.dataType() + ")");
        bool extractState = (extractStateStr == false) ? HIGH : LOW;
        digitalWrite(extractPin, extractState);
    }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

      if (Firebase.RTDB.getBool(&fbdo, "Controls/boil")) {
        if (fbdo.dataType() == "boolean"){
          bool boilStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + boilStateStr + "(" + fbdo.dataType() + ")");
          bool boilState = (boilStateStr == false) ? HIGH : LOW;
          digitalWrite(boilPin, boilState);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
      
      if (Firebase.RTDB.getBool(&fbdo, "Controls/dry")) {
        if (fbdo.dataType() == "boolean"){
          bool dryStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + dryStateStr + "(" + fbdo.dataType() + ")");
          bool dryState = (dryStateStr == false) ? HIGH : LOW;
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
          bool startExtractionStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startExtractionStateStr + "(" + fbdo.dataType() + ")");
          bool startExtractionState = (startExtractionStateStr == false) ? HIGH : LOW;
          digitalWrite(pumpToBoilPin, startExtractionState);
          isExtractionStart = startExtractionState;
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.getBool(&fbdo, "Controls/startTransfering")) {
        if (fbdo.dataType() == "boolean"){
          bool startTransferingStateStr = fbdo.boolData();
          Serial.println("Seccess: " + fbdo.dataPath() + ": " + startTransferingStateStr + "(" + fbdo.dataType() + ")");
          bool startTransferingState = (startTransferingStateStr == false) ? HIGH : LOW;
          digitalWrite(pumpToJuicePin, startTransferingState);
          isTransferingStart = startTransferingStateStr;
        }
      } else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }

      pumpToBoiler(boilSizeValue, isExtractionStart);
      pumpToJuiceStorage(transferSizeValue,isTransferingStart );
      emergencyButton();
 

    Serial.println("_______________________________________");
  }
}




