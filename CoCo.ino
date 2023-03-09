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

void sound(int num){
  for(int i=1; i<num; i++){
    digitalWrite(PIN_SOUNDS, HIGH);
    delayMicroseconds(800);
    digitalWrite(PIN_SOUNDS, LOW);
    delayMicroseconds(800);
  }
}

void nfcRegistered(String nuid){
  int id = -1;
  for (int i=0; i<usercnt; i++){
    String curNUID = nuids[i];
    // Serial.print("  Comparing to ");
    // Serial.print(curNUID);
    // Serial.print("... ");
    if (curNUID == nuid){
      id = i;
      // Serial.println("Yup.");
    }else{
      // Serial.println("Nope.");
    }
  }
  if (id == -1){
    // Serial.println("User not found; adding to new NUID");
    ESPUI.updateControlValue(wNewNUID, nuid);
    sound(2);
  }else{
    sound(3);
    addCoffee(id);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    digitalWrite(PIN_LED, HIGH);
    // Serial.print(" -> Identified as "); // Serial.println(usernames[id]);
  }
}

void addCoffee(int uid){
  coffees[uid]++;
  updateUI();
  save_json_data();
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

  // Serial.print(F("RFID In dec: "));
  //printHex(rfid.uid.uidByte, rfid.uid.size);
  // Serial.print("<");
  String nuid_str = nuid2str(rfid.uid.uidByte, rfid.uid.size);
  // Serial.print(nuid_str);
  // Serial.println(">");

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  nfcRegistered(nuid_str);
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    // Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    // Serial.print(buffer[i], HEX);
  }
}

String nuid2str(byte *buffer, byte bufferSize) {
  String ret = "";  
  for (byte i = 0; i < bufferSize; i++) {
    //ret.concat(buffer[i] < 0x10 ? " 0" : " ");
    char myHex[10] = "";
    ltoa(buffer[i],myHex,16);
    ret.concat(myHex);
    //// Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //// Serial.print(buffer[i], HEX);
  }
  return ret;
}

void setupWiFi(){
  #if defined(ESP32)
    WiFi.setHostname(hostname);
  #else
    WiFi.hostname(hostname);
  #endif
  uint8_t timeout = 10;
  // try to connect to existing network
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
  //char *ap_ssid = WIFI_NAME;
  //char *ap_pw   = WIFI_PW;
  //snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);
  snprintf(ap_ssid, 26, "CoCo-WUI");
  WiFi.softAP(ap_ssid);
  //WiFi.softAP(ap_ssid, ap_pw);
  //WiFi.softAP("CoCo-WUI", "");

  timeout = 5;

  do
  {
      delay(500);
      // Serial.print(".");
      timeout--;
  } while (timeout);

  dnsServer.start(DNS_PORT, "*", apIP);

  // Serial.println("\n\nWiFi parameters:");
  // Serial.print("Mode: ");
  // Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}





void setup(void){
    //// Serial.begin(115200);
    pinMode(PIN_SOUNDS, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_SWITCH, INPUT);
    sound(1);
    delay(1000);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    digitalWrite(PIN_LED, HIGH);
    delay(100);
#ifdef ESP32
    if (!SPIFFS.begin(true)) {
#else
    #warning "Compiling for ESP8266"
    SPI.begin();
    if (!SPIFFS.begin()) {
#endif
      // Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    // Serial.println("SPIFFS initialized. Starting SPI...");

#if defined(ESP32)
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
#else
    //SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
#endif
    // Serial.println("Done. Connecting to MFRC522...");
    rfid.PCD_Init();
    // Serial.print(F("Reader :"));
    //rfid.PCD_DumpVersionToSerial();

    //makeTestJSON();

    readJSON();

    //setupWiFi();
    setupESPUI();
    sound(4);
    //setupESPUI();
}

void loop(void)
{
    dnsServer.processNextRequest();
    readRFID();
    //static long oldTime = 0;
    //static bool switchi = false;
    if (ui_active){
      if (millis() > closeUITime){
        ui_active = false;
        ESP.restart();
      }
    }
    if (!ui_active && digitalRead(PIN_SWITCH)){
      ui_active = true;
      sound(5);
      setupWiFi();
      yield();
      beginESPUI();
      closeUITime = millis() + WIFI_UPTIME;
      digitalWrite(PIN_LED, LOW);
    }

    
}

