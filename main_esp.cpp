#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const char* ssid = "Caty";
const char* password = "GDe5-6M2";
const char* serverUrl = "https://172.20.10.3:8443/api/data";


String sensorData = "";

SoftwareSerial mySerial(D1, D2); // RX Tx

std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

void setup() {
    Serial.begin(115200);
    mySerial.begin(115200);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Se conecteaza la Wi-Fi...");
    }

    delay(1000);
    Serial.println("Conectat la Wi-Fi!");
    Serial.print("Adresa IP: ");
    Serial.println(WiFi.localIP());
}

void loop(){
  
  if (WiFi.status() == WL_CONNECTED) {

    client->setInsecure();

    HTTPClient https;
    if (!https.begin(*client, serverUrl)) {
    Serial.println("Eroare la inițializarea conexiunii HTTPS!");
    return;
    }

    https.begin(*client, serverUrl);
    https.addHeader("Content-Type", "application/json");

    
    StaticJsonDocument<200> jsonDoc;
    String payload;

    if (mySerial.available()) {
      String payload = mySerial.readStringUntil('\n');
      payload.trim();

      Serial.print("Payload primit: ");
      Serial.println(payload);  
      
      DeserializationError error = deserializeJson(jsonDoc, payload); 
      if (error) {
          Serial.print("Eroare la parsarea JSON: ");
          Serial.println(error.c_str());
          return;
      } 
      String validPayload;
      serializeJson(jsonDoc, validPayload); 
      Serial.print("JSON valid reconstruit: ");
      Serial.println(validPayload);
      int httpCode = https.POST(validPayload);
    
      if (httpCode > 0) {
        Serial.println("Data sent successfully");
        Serial.print("Răspuns server: ");
        Serial.println(https.getString());
      } else {
        Serial.println("Failed to send data");
        Serial.println(httpCode);
      }
      https.end();
      payload = "";
    }
  } else {
    Serial.println("Eroare: Nu s-a putut conecta la Wi-Fi!");
    delay(1000);
  }  
}

