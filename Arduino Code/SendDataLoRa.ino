#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DHTPIN 19      // Pin of DHT11
#define DHTTYPE DHT11  // DHT 11

// Pin used for LoRa
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9
// #define BAND 915E6  // frequency to use (915 MHz our case)

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
// ----------------------------

DHT dht(DHTPIN, DHTTYPE);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/18, /* data=*/17, /* reset=*/21);

#define SOIL_MOISTURE_PIN 20    // Pin of soil moisture sensor
#define SOIL_MOISTURE_DRY 4095  // dry soil data
#define SOIL_MOISTURE_WET 0     // wet soil data

void setup(void) {
  Serial.begin(9600);
  u8g2.begin();
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  // radio.setFrequency(915.0);
}

void loop(void) {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Datos del sensor: ");

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  String dataToSend = "";

  if (isnan(h) || isnan(t)) {
    u8g2.drawStr(10, 20, "Error al leer DHT");
  } else {
    int soilMoisturePercent = map(soilMoisture, SOIL_MOISTURE_WET, SOIL_MOISTURE_DRY, 100, 0);
    char tempStr[10];
    char humStr[10];

    dtostrf(t, 4, 2, tempStr);
    dtostrf(h, 4, 2, humStr);
    char finalTempStr[25];
    char finalHumStr[25];
    char soilMoistureStr[15];

    sprintf(finalTempStr, "Temperatura: %s", tempStr);
    sprintf(finalHumStr, "Humedad: %s", humStr);
    sprintf(soilMoistureStr, "Suelo: %d%%", soilMoisturePercent);

    u8g2.drawStr(10, 30, finalTempStr);
    u8g2.drawStr(10, 45, finalHumStr);
    u8g2.drawStr(94, 45, " %");
    u8g2.drawStr(10, 60, soilMoistureStr); 
    char dataString[50];
    sprintf(dataString, "%s,%s,%d", tempStr, humStr, soilMoisturePercent);
    dataToSend = String(dataString);

    // --- Send datta for LoRa
    Serial.print(F("[SX1262] Transmitting packet ... "));

    int state = radio.transmit(dataToSend);

    if (state == RADIOLIB_ERR_NONE) {
      // the packet was successfully transmitted
      Serial.println(F("success!"));

      // print measured data rate
      Serial.print(F("[SX1262] Datarate:\t"));
      Serial.print(radio.getDataRate());
      Serial.println(F(" bps"));
      // Serial.print("Datos: ");
      // Serial.println(dataString);
    } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
      // the supplied packet was longer than 256 bytes
      Serial.println(F("too long!"));
    } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
      // timeout occured while transmitting packet
      Serial.println(F("timeout!"));
    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
  }
  u8g2.sendBuffer();

  delay(2000);
}
