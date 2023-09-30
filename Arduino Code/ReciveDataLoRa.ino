#include <Arduino.h>
#include <RadioLib.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/18, /* data=*/17, /* reset=*/21);

//SSID network
const char* ssid = "HOLA";
const char* password = "hambre21";


void setup() {
  u8g2.begin();
  Serial.begin(9600);
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);

  // connection of WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Conectando a la red WiFi...");
  }
  Serial.println("¡Conectado a la red WiFi!");

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
}

void loop() {
  Serial.print(F("[SX1262] Waiting for incoming transmission ... "));
  String str;
  int state = radio.receive(str);
  char *tempValue = NULL;
  char *humValue = NULL;
  char *soilValue = NULL;
  Serial.println(F("Datos!"));
  Serial.println(F(RADIOLIB_ERR_NONE));

  if (state == RADIOLIB_ERR_NONE) {
    // packet was successfully received
    Serial.println(F("success!"));

    // print the data of the packet
    Serial.print(F("[SX1262] Data:\t\t"));
    Serial.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.print(F("[SX1262] RSSI:\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Recibiendo");
    char charBuf[128];  // make sure this is large enough for your string
    str.toCharArray(charBuf, 128);
    char tempBuf[32];
    char humBuf[32];
    char soilBuf[32];
    char *token = strtok(charBuf, ",");

    if (token != NULL) {
      tempValue = token;
      sprintf(tempBuf, "Temperatura: %s", token);
      u8g2.drawStr(0, 20, tempBuf);
      token = strtok(NULL, ",");
    }
    if (token != NULL) {
       humValue = token; 
      sprintf(humBuf, "Humedad: %s", token);
      u8g2.drawStr(0, 30, humBuf);
      token = strtok(NULL, ",");
    }
    if (token != NULL) {
       soilValue = token;
      sprintf(soilBuf, "Suelo: %s", token);
      u8g2.drawStr(0, 40, soilBuf);
    }

    // We prepare the date
    String payload = "{\"tempertaturaAmbiente\": \"";
    payload += tempValue;
    payload += "\", \"humedadAmbiente\": \"";
    payload += humValue;
    payload += "\", \"humedadSuelo\": \"";
    payload += soilValue ;
    payload += "\"}";

    if(WiFi.status()== WL_CONNECTED){
       HTTPClient http;   
       http.begin("http://192.168.109.224:5000/api");  // request to API
       http.addHeader("Content-Type", "application/json"); 
       int httpResponseCode = http.POST(payload);  
       Serial.print("payload ");
       Serial.println(payload);

       // Server response
       String response = http.getString();   
       if (httpResponseCode == 200) {
           Serial.println("Datos enviados con éxito!");
        } else {
            Serial.print("Error al enviar datos. Código de respuesta: ");
            Serial.println(httpResponseCode);
            Serial.print("Error: ");
            Serial.println(http.errorToString(httpResponseCode));
            Serial.print("Error: ");
            Serial.println(WiFi.status());            
        } 
       http.end();  // End connection
       Serial.println("Conection WiFi");
    }else{
        Serial.println("Error en WiFi");
    }

    sprintf(charBuf, "RSSI: %d dBm", radio.getRSSI());
    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    Serial.print(F("[SX1262] SNR:\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
    sprintf(charBuf, "SNR: %d dB", radio.getSNR());

    u8g2.sendBuffer();
  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    Serial.println(F("timeout!"));
    u8g2.drawStr(20, 40, "Tiempo agotado!");
    u8g2.sendBuffer();

  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));
    u8g2.drawStr(0, 40, "CRC error!");
  } else {
    // some other error occurred
    u8g2.drawStr(0, 40, "failed, code error!");
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
  delay(2000);
}