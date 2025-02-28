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
#define MAX_DISTANCE_CM 30  
#define MIN_DISTANCE_CM 2 

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define DATABASE_URL ""

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

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2)                                                                                                     ;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.0343 / 2;  // Convert to cm

  return distance;
}


int mapDistanceToLevel(float distance) {
  if (distance < MIN_DISTANCE_CM) distance = MIN_DISTANCE_CM;
  if (distance > MAX_DISTANCE_CM) distance = MAX_DISTANCE_CM;

  int level = map(distance, MAX_DISTANCE_CM, MIN_DISTANCE_CM, 0, 5);
  return level;
}

void loop() {
    // digitalWrite(trigPin, LOW);
    // delayMicroseconds(2)                                                                                                     ;
    // digitalWrite(trigPin, HIGH);
    // delayMicroseconds(10);
    // digitalWrite(trigPin, LOW);
    // duration = pulseIn(echoPin, HIGH);
    // distance = duration*0.034/2;
    // Serial.print("Distance");
    // Serial.println(distance);

    float distance = getDistance();
    int level = mapDistanceToLevel(distance);
 Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Water Level: ");
  Serial.println(level);

   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
     sensors.requestTemperatures(); 
      float temp = sensors.getTempCByIndex(0);

      int boilSizeValue;
      Firebase.RTDB.setFloat(&fbdo, "Sensors/temperature", temp)) 
      Serial.print("Celsius temperature: ");
      Serial.print(temp); 

    }

   }

