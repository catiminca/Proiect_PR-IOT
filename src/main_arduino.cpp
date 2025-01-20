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

unsigned long previousMillis_temp = 0;

const unsigned long interval = 10000;

unsigned long currentMillis_temp;

unsigned long currentMillis_mq135;
unsigned long currentMillis_mq2;
const unsigned long interval_sensors = 5000;
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

  pinMode(ledPin_photoresistor,  OUTPUT);
  pinMode(ledPin_mq135, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(A0, INPUT);

  Serial.println("MQ2 & MQ135 are warming up!");
	delay(10000);
  Serial.println("Warming up done!");

}

  // Construct payload
  void sendPayload(float temperature, float humidity, int photoresistor, float mq135, float mq2) {
    StaticJsonDocument<200> jsonDoc;

    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["photoresistor"] = photoresistor;
    jsonDoc["airQuality"] = mq135;
    jsonDoc["gas"] = mq2;

    serializeJson(jsonDoc, espSerial);
    serializeJson(jsonDoc, Serial);
    Serial.println();
}


void loop() {

  //temperature and humidity
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
    
  //MQ135 sensor
  sensorValue_mq135 = analogRead(A1);
  if (sensorValue_mq135 > 400) {
    Serial.print(" | Air quality not good: ");
    Serial.print(sensorValue_mq135, DEC);
    Serial.println(" PPM");
    airQualityFlag = true;
    digitalWrite(ledPin_mq135, HIGH);
    delay(2000);
  } else {
    airQualityFlag = false;
    digitalWrite(ledPin_mq135, LOW);
  }
  
  //MQ2 sensor
  sensorValue_mq2 = analogRead(A3);
  if(sensorValue_mq2 > 500) {
    Serial.print(" | Smoke or gas detected! ");
    Serial.println(sensorValue_mq2, DEC);
    gasFlag = true;
    tone(buzzerPin, 110);

    delay(2000);
  } else {
    noTone(buzzerPin);
    gasFlag = false;
  }

  // Measurements past threshold send each 7 seconds
  if (airQualityFlag == true && currentMillis_mq135 - previousMillis_mq135 >= interval_sensors) {
    previousMillis_mq135 = currentMillis_mq135;
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
  } else if (gasFlag == true && currentMillis_mq2 - previousMillis_mq2 >= interval_sensors) {
    previousMillis_mq2 = currentMillis_mq2;
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
  }

  // Measurements send each 15 seconds
  if (currentMillis_temp - previousMillis_temp >= interval && airQualityFlag == false && gasFlag == false) {
    previousMillis_temp = currentMillis_temp;
    sendPayload(event_t.temperature, event_h.relative_humidity, sensorValue, sensorValue_mq135, sensorValue_mq2);
  }
  
}