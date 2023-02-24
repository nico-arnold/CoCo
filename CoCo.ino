#include <TickTwo.h>
#include <DNSServer.h>
#include <ESPUI.h>
#include "FS.h"
#if defined(ESP32)
#include "SPIFFS.h"
#endif
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "CoCo.h"
#include "JSON.h"
#include "UI.h"

void stopBuzz(){
  digitalWrite(PIN_BUZZER, LOW);
  buzzState = false;
}

void buzzCycle(){
  buzzState = !buzzState;
  digitalWrite(PIN_BUZZER, buzzState);
  buzzTimes--;
  if (buzzTimes){
    Serial.print("Switching Buzzer to "); Serial.println(buzzState);
    Serial.print("Remaining buzzCycles: "); Serial.println(buzzTimes);
    nextBuzzCycle = millis() + BUZZ_LENGTH;
  }else{
    stopBuzz();
    Serial.println("Stopping Buzzer");
  }
}

void startBuzz(){
  nextBuzzCycle = millis() + BUZZ_LENGTH;
  digitalWrite(PIN_BUZZER, HIGH);
  buzzState = true;
  Serial.println("Starting Buzzer");
}

void buzz(int times){
  buzzTimes = 2*(times-1)+1; 
  startBuzz();
}

void nfcRegistered(String nuid){
  int id = -1;
  for (int i=0; i<usercnt; i++){
    String curNUID = nuids[i];
    Serial.print("  Comparing to ");
    Serial.print(curNUID);
    Serial.print("... ");
    if (curNUID == nuid){
      id = i;
      Serial.println("Yup.");
    }else{
      Serial.println("Nope.");
    }
  }
  if (id == -1){
    buzz(3);
    Serial.println("User not found; adding to new NUID");
    ESPUI.updateControlValue(wNewNUID, nuid);
  }else{
    buzz(2);
    Serial.print(" -> Identified as "); Serial.println(usernames[id]);
    addCoffee(id);
  }
}

void addCoffee(int uid){
  coffees[uid]++;
  updateUI();
}

void readRFID(void ) { /* function readRFID */
  ////Read RFID card

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // Look for new 1 cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (  !rfid.PICC_ReadCardSerial())
    return;

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.print(F("RFID In dec: "));
  //printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.print("<");
  String nuid_str = nuid2str(rfid.uid.uidByte, rfid.uid.size);
  Serial.print(nuid_str);
  Serial.println(">");

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  nfcRegistered(nuid_str);
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

String nuid2str(byte *buffer, byte bufferSize) {
  String ret = "";  
  for (byte i = 0; i < bufferSize; i++) {
    //ret.concat(buffer[i] < 0x10 ? " 0" : " ");
    char myHex[10] = "";
    ltoa(buffer[i],myHex,16);
    ret.concat(myHex);
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //Serial.print(buffer[i], HEX);
  }
  return ret;
}

void setupWiFi(){
  #if defined(ESP32)
    WiFi.setHostname(hostname);
  #else
    WiFi.hostname(hostname);
  #endif

  // try to connect to existing network
  WiFi.begin(ssid, password);
  Serial.print("\n\nTry to connect to existing network");

  {
      uint8_t timeout = 10;

      // Wait for connection, 5s timeout
      do
      {
          delay(500);
          Serial.print(".");
          timeout--;
      } while (timeout && WiFi.status() != WL_CONNECTED);

      // not connected -> create hotspot
      if (WiFi.status() != WL_CONNECTED)
      {
          Serial.print("\n\nCreating hotspot");

          WiFi.mode(WIFI_AP);
          delay(100);
          WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
#if defined(ESP32)
          uint32_t chipid = 0;
          for (int i = 0; i < 17; i = i + 8)
          {
              chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
          }
#else
          uint32_t chipid = ESP.getChipId();
#endif
          char ap_ssid[25];
          snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);
          //snprintf(ap_ssid, 26, "CoCo-WUI");
          WiFi.softAP(ap_ssid);

          timeout = 5;

          do
          {
              delay(500);
              Serial.print(".");
              timeout--;
          } while (timeout);
      }
  }

  dnsServer.start(DNS_PORT, "*", apIP);

  Serial.println("\n\nWiFi parameters:");
  Serial.print("Mode: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  Serial.print("IP address: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}





void setup(void){
    Serial.begin(115200);
    pinMode(PIN_BUZZER, OUTPUT);
    buzz(3);
#ifdef ESP32
    if (!SPIFFS.begin(true)) {
#else
    #warning "Compiling for ESP8266"
    SPI.begin();
    if (!SPIFFS.begin()) {
#endif
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    Serial.println("SPIFFS initialized. Starting SPI...");

#if defined(ESP32)
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
#else
    //SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
#endif
    Serial.println("Done. Connecting to MFRC522...");
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();

    //makeTestJSON();

    readJSON();

    setupWiFi();

    setupESPUI();
}

void loop(void)
{
    dnsServer.processNextRequest();
    readRFID();
    if (buzzTimes > 0 && millis() > nextBuzzCycle){
      buzzCycle();
    }
    //static long oldTime = 0;
    //static bool switchi = false;

    
}

