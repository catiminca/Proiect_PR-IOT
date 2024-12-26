#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <MQUnifiedsensor.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#define DHTTYPE DHT11
const int DHTPin = 7; 
SoftwareSerial espSerial(5, 6);
DHT_Unified dht(DHTPin, DHTTYPE);

MQUnifiedsensor MQ2("Arduino", 5, 10, A2, "MQ-2");

int ledPin_photoresistor = 2;
float sensorValue_mq135;
int buzzerPin = 8;
int ledPin_mq135 = 9;

unsigned long previousMillis_temp = 0; // To track the time

const unsigned long interval = 15000; // 15 seconds interval in milliseconds

unsigned long currentMillis_temp;

unsigned long currentMillis_mq135;
unsigned long currentMillis_mq2;
const unsigned long interval_mq135 = 7000;
unsigned long previousMillis_mq135 = 0;
unsigned long previousMillis_mq2 = 0;


int ledPin_mq2 = 4;
float sensorValue_mq2;
int digitalValue_mq2;

bool gasFlag = false;
bool airQualityFlag = false;

void setup() {
  Serial.begin(9600);
  dht.begin();
  espSerial.begin(115200);
  
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  //photoresistor
  pinMode(ledPin_photoresistor,  OUTPUT);
  pinMode(ledPin_mq135, OUTPUT);
  pinMode(A0, INPUT);
  
  Serial.println("MQ2 & MQ135 are warming up!");
	delay(10000); // allow the MQ2 and MQ135 to warm up
  Serial.println("Warming up done!");

  pinMode(buzzerPin, OUTPUT);

  // tone(buzzerPin, 1000, 2000);
}

  // Create JSON payload
  void sendPayload(float temperature, float humidity, int photoresistor, float mq135, float mq2) {
    // Creează un obiect JSON
    StaticJsonDocument<200> jsonDoc;

    // Adaugă valori în obiectul JSON
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["photoresistor"] = photoresistor;
    jsonDoc["airQuality"] = mq135;
    jsonDoc["gas"] = mq2;

    // Serializare JSON și trimitere către ESP
    serializeJson(jsonDoc, espSerial);
    serializeJson(jsonDoc, Serial); // Pentru debugging
    Serial.println(); // Linie nouă pentru claritate
}


void loop() {

  
  currentMillis_temp = millis();
  currentMillis_mq135 = millis();
  currentMillis_mq2 = millis();
  sensors_event_t event_t, event_h;
  dht.temperature().getEvent(&event_t);
  dht.humidity().getEvent(&event_h);

  if (isnan(event_t.temperature) || isnan(event_h.relative_humidity)) {
    Serial.println("Error reading sensor!");
      
  }

  //photoresistor
  int sensorValue = analogRead(A0);
  
  if (sensorValue > 200) {
    digitalWrite(ledPin_photoresistor, HIGH);
  }
  else {
    digitalWrite(ledPin_photoresistor, LOW);
  }
    
  sensorValue_mq135 = analogRead(A1);
  // currentMillis_mq135 = millis();
  if (sensorValue_mq135 > 400) {
    Serial.print(" | Air quality not good: ");
    Serial.print(sensorValue_mq135, DEC); // prints the value read
    Serial.println(" PPM");
    airQualityFlag = true;
    digitalWrite(ledPin_mq135, HIGH);
    delay(2000);
  } else {
    airQualityFlag = false;
    digitalWrite(ledPin_mq135, LOW);
  }
  
  sensorValue_mq2 = analogRead(A3); // read analog input pin 0
  if(sensorValue_mq2 > 500) {
    Serial.print(" | Smoke or gas detected! ");
    // Serial.print("Sensor Value is : ");
    Serial.println(sensorValue_mq2, DEC);
    gasFlag = true;
    tone(buzzerPin, 110); // A4
    // delay(1000);

    delay(2000);
  } else {
    noTone(buzzerPin);  // delay(1000);
    gasFlag = false;
  }

  if (airQualityFlag == true && currentMillis_mq135 - previousMillis_mq135 >= interval_mq135) {
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
    // delay(10000);
  } else if (gasFlag == true && currentMillis_mq2 - previousMillis_mq2 >= interval_mq135) {
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
    // delay(10000);
  }

  if (currentMillis_temp - previousMillis_temp >= interval && airQualityFlag == false && gasFlag == false) {
    previousMillis_temp = currentMillis_temp;
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
  }
  
}