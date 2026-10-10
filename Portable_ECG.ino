#include <WiFi.h>
#include <ThingSpeak.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const char* ssid = "Aru";
const char* password = "12345678";

unsigned long channel = 3525176;
const char* apiKey = "3NGH1IQBDO1S68E7";

BLECharacteristic *ecg;
WiFiClient client;

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);

  BLEDevice::init("Portable_ECG");
  BLEServer *s = BLEDevice::createServer();
  BLEService *sv = s->createService("180D");

  ecg = sv->createCharacteristic(
    "2A37",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  ecg->addDescriptor(new BLE2902());
  sv->start();
  BLEDevice::getAdvertising()->start();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  ThingSpeak.begin(client);
}

void loop() {
  int value = analogRead(34);
  char data[8];

  sprintf(data, "%d", value);
  ecg->setValue(data);
  ecg->notify();

  Serial.println(value);

  if (millis() - lastSend >= 15000 &&
      WiFi.status() == WL_CONNECTED) {
    lastSend = millis();

    int result = ThingSpeak.writeField(
      channel, 1, value, apiKey
    );

    Serial.println(result == 200 ?
      "ThingSpeak OK" : "ThingSpeak Error");
  }

  delay(20);
}
