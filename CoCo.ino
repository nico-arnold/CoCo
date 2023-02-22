#include <DNSServer.h>
#include <ESPUI.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "CoCo.h"
#include "UI.h"


void makeTestJSON(){
  Serial.println("Making test JSON...");

  StaticJsonDocument<512> doc;
  JsonArray users = doc.createNestedArray("users");
  JsonObject christoph = users.createNestedObject();
  christoph["name"] = "Christoph";
  christoph["id"] = 0;
  christoph["nfc"] = "ac72e58c";
  christoph["coffee"] = 5;
  JsonObject johannes = users.createNestedObject();
  johannes["name"] = "Johannes";
  johannes["id"] = 1;
  johannes["nfc"] = "144f5760";
  johannes["coffee"] = 2;
  JsonObject nico = users.createNestedObject();
  nico["name"] = "Nico";
  nico["id"] = 2;
  nico["nfc"] = "NFCCODE"; 
  nico["coffee"] = 7;

  Serial.println("Writing file");

  File file = SPIFFS.open(COCO_FILE, FILE_WRITE);
  if(!file){
      Serial.println("- failed to open file for writing");
      while(true){}
  }
  if(serializeJson(doc, file) != 0){
      Serial.println("- file written");
  } else {
      Serial.println("- write failed");
      while(true){}
  }
  file.close();  
}

void readJSON(){
    File file = SPIFFS.open(COCO_FILE, "r");
    if(!file){
      Serial.println("Failed to open file for reading");
      return;
    }
    Serial.print("\n\n");

    file = SPIFFS.open(COCO_FILE, "r");
    DeserializationError desErr = deserializeJson(usersDoc, file);
    if (desErr){
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(desErr.f_str());
      return;
    }
    j_users = usersDoc["users"].as<JsonArray>();
    
    file.close();
}

bool save_json_data(){
  StaticJsonDocument<512> doc;
  JsonArray doc_users = doc.createNestedArray("users");
  JsonObject users[MAX_USERS];
  for (int i=0; i<j_users.size(); i++){
    users[i] = doc_users.createNestedObject();
    users[i]["name"] = j_users[i]["name"];
    users[i]["id"] = j_users[i]["id"];
    users[i]["nfc"] = j_users[i]["nfc"];
    users[i]["coffee"] = j_users[i]["coffee"];
  }
  Serial.println("   Writing file:\n");
  Serial.print("   ");
  serializeJson(doc, Serial);
  Serial.println("\n");
  File file = SPIFFS.open(COCO_FILE, FILE_WRITE);
  if(!file){
      Serial.println("- failed to open file for writing");
      return false;
  }
  int serBytes = serializeJson(doc, file);
  if (doc.overflowed()){
    Serial.println("  !document overflowed!");
  }
  if( serBytes != 0){
      Serial.print("- file written: ");
      Serial.print(serBytes);
      Serial.println(" bytes");
  } else {
      Serial.println("- write failed");
      return false;
  }
  file.close();  


/*
  Serial.println("   Opening file...");
  File outfile = SPIFFS.open(COCO_FILE, FILE_WRITE);  
  if (!outfile) {
    Serial.println(F("Failed to create outfile"));
    return false;
  }
  Serial.println("   Serializing...\n\n   ");
  serializeJson(usersDoc, Serial);
  Serial.println("\n\n");
  int serBytes = serializeJson(usersDoc, outfile);
  if (serBytes == 0) {
    Serial.println(F("Failed to write to outfile"));
    return false;
  }
  Serial.print("   Wrote ");
  Serial.print(serBytes);
  Serial.println(" bytes.");
  Serial.println("   Closing file...");
  outfile.close();
  Serial.println("   Returning...");*/
  return true;
}

void nfcRegistered(String nuid){
  int id = -1;
  for (int i=0; i<j_users.size(); i++){
    String curNUID = j_users[i]["nfc"].as<String>();
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
    Serial.println("User not found; adding to new NUID");
    ESPUI.updateControlValue(wNewNUID, nuid);
  }else{
    Serial.print(" -> Identified as "); Serial.println(j_users[id]["name"].as<String>());
  }
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

void setup(void){
    Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    Serial.println("SPIFFS initialized. Starting SPI...");

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
    Serial.println("Done. Connecting to MFRC522...");
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();

    //makeTestJSON();

    readJSON();

    Serial.println("Users: ");
    for (int i=0; i<j_users.size(); i++){
      Serial.print("->");
      Serial.print(j_users[i]["id"].as<int>());
      Serial.print("=");
      Serial.print(j_users[i]["name"].as<String>().c_str());
      Serial.print(" -> ");
      Serial.println(j_users[i]["coffee"].as<int>());
    }

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

    setupESPUI();
}

void loop(void)
{
    dnsServer.processNextRequest();
    readRFID();

    //static long oldTime = 0;
    //static bool switchi = false;

    
}

