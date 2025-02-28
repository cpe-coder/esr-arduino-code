#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define ONE_WIRE_BUS D0
#define echoPin D1
#define trigPin D2
#define echoPin1 D3
#define trigPin1 D4
#define MAX_DISTANCE_CM 40  
#define MAX_DISTANCE_CM1 70  
#define MIN_DISTANCE_CM 4 

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

#define WIFI_SSID "So Good"
#define WIFI_PASSWORD "helloworld"

#define API_KEY "AIzaSyDIUvTegr1EgYJ9qgw7lqKSV2UoG75HKRk"
#define DATABASE_URL "e-sugar-rush-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
long duration;
int distance; 

unsigned long sendDataPrevMillis = 0;


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  Serial.begin(9600);
  sensors.begin();
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

float getJuiceStorageDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2)                                                                                                     ;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.0343 / 2;  // Convert to cm

  return distance;
}

float getMainStorageDistance() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2)                                                                                                     ;
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  long duration = pulseIn(echoPin1, HIGH);
  float distance = duration * 0.0343 / 2;  // Convert to cm

  return distance;
}


int mapJuiceStorageDistanceToLevel(float juiceStorageDistance) {
  if (juiceStorageDistance < MIN_DISTANCE_CM) juiceStorageDistance = MIN_DISTANCE_CM;
  if (juiceStorageDistance > MAX_DISTANCE_CM) juiceStorageDistance = MAX_DISTANCE_CM;

  int level = map(juiceStorageDistance, MAX_DISTANCE_CM, MIN_DISTANCE_CM, 0, 5);
  return level;
}

int mapMainStorageDistanceToLevel(float mainStorageDistance) {
  if (mainStorageDistance < MIN_DISTANCE_CM) mainStorageDistance = MIN_DISTANCE_CM;
  if (mainStorageDistance > MAX_DISTANCE_CM1) mainStorageDistance = MAX_DISTANCE_CM1;

  int level = map(mainStorageDistance, MAX_DISTANCE_CM1, MIN_DISTANCE_CM, 0, 5);
  return level;
}

void loop() {

    float juiceStorageDistance = getJuiceStorageDistance();
    int juiceStorageLevel = mapJuiceStorageDistanceToLevel(juiceStorageDistance);
    Serial.print("JuiceStorageDistance: ");
    Serial.print(juiceStorageDistance);
    Serial.print(" cm, Water Level: ");
    Serial.println(juiceStorageLevel);

    float mainStorageDistance = getJuiceStorageDistance();
    int mainStorageLevel = mapJuiceStorageDistanceToLevel(mainStorageDistance);
    Serial.print("MainStorageDistance: ");
    Serial.print(mainStorageDistance);
    Serial.print(" cm, Water Level: ");
    Serial.println(mainStorageLevel);

   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
     sensors.requestTemperatures(); 
      float temp = sensors.getTempCByIndex(0);

      Firebase.RTDB.setFloat(&fbdo, "Sensors/temperature", temp);
      Serial.print("Celsius temperature: ");
      Serial.print(temp); 

    }

}

