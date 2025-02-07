#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>

Servo servo;
#define pumpToJuicePin D6

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
bool startTransferingState = false;
int transferSize = 0;


unsigned long sendDataPrevMillis = 0;




void setup() {
  pinMode(pumpToJuicePin, OUTPUT);
  digitalWrite(pumpToJuicePin, LOW);
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
  // put your main code here, to run repeatedly:
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

     int transferSizeValue;
     bool isTransferingStart;

     if (transferSizeValue != 0 && isTransferingStart){
      digitalWrite(pumpToJuicePin, HIGH);
      Serial.println("Pump to juice storage is working...");
      int time = 40000 * transferSizeValue;
      delay(time);
      Serial.println(time);
      digitalWrite(pumpToJuicePin, LOW);
      if (Firebase.RTDB.setBool(&fbdo, "Controls/startTransfering", false)) {
          Serial.println("Pump to Juice Storage is STOP...");
        }else {
        Serial.println("Failed to read Auto: " + fbdo.errorReason());
      }
    }

   }

}
