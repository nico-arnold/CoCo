void makeTestJSON();
void readJSON();
bool save_json_data();




void makeTestJSON(){
  // Serial.println("Making test JSON...");

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

  // Serial.println("Writing file");

  File file = SPIFFS.open(COCO_FILE, FILE_WRITE);
  if(!file){
      // Serial.println("- failed to open file for writing");
      while(true){}
  }
  if(serializeJson(doc, file) != 0){
      // Serial.println("- file written");
  } else {
      // Serial.println("- write failed");
      while(true){}
  }
  file.close();  
}

void readJSON(){
  StaticJsonDocument<1024> usersDoc;
  JsonArray j_users;
  File file = SPIFFS.open(COCO_FILE, "r");
  if(!file){
    // Serial.println("Failed to open file for reading; creating empty test file");
    makeTestJSON();
    return;
  }
  // Serial.print("\n\n");

  file = SPIFFS.open(COCO_FILE, "r");
  DeserializationError desErr = deserializeJson(usersDoc, file);
  if (desErr){
    // Serial.print(F("deserializeJson() failed with code "));
    // Serial.println(desErr.f_str());
    return;
  }
  j_users = usersDoc["users"].as<JsonArray>();
  usercnt = j_users.size();

  for (int i=0; i<usercnt; i++){
    usernames[i] = j_users[i]["name"].as<String>();
    nuids[i] = j_users[i]["nfc"].as<String>();
    coffees[i] = j_users[i]["coffee"].as<uint16_t>();
  }

  // Serial.println("Read users:");
  for (int i=0; i<usercnt; i++){
    // Serial.print("-");
    // Serial.print(i);
    // Serial.print(") ");
    // Serial.print(usernames[i].c_str());
    // Serial.print("=");
    // Serial.print(nuids[i].c_str());
    // Serial.print(" -> ");
    // Serial.print(coffees[i]);
    // Serial.print("\n");
  }
  
  file.close();
}

bool save_json_data(){
  StaticJsonDocument<512> doc;
  JsonArray doc_users = doc.createNestedArray("users");
  JsonObject users[MAX_USERS];
  for (int i=0; i<usercnt; i++){
    users[i] = doc_users.createNestedObject();
    users[i]["name"] = usernames[i];
    users[i]["id"] = i;
    users[i]["nfc"] = nuids[i];
    users[i]["coffee"] = coffees[i];
  }
  // Serial.println("   Writing file:\n");
  // Serial.print("   ");
  // SerializeJson(doc, // Serial);
  // Serial.println("\n");
  File file = SPIFFS.open(COCO_FILE, FILE_WRITE);
  if(!file){
      // Serial.println("- failed to open file for writing");
      return false;
  }
  int serBytes = serializeJson(doc, file);
  if (doc.overflowed()){
    // Serial.println("  !document overflowed!");
  }
  if( serBytes != 0){
      // Serial.print("- file written: ");
      // Serial.print(serBytes);
      // Serial.println(" bytes");
  } else {
      // Serial.println("- write failed");
      return false;
  }
  file.close();  


/*
  // Serial.println("   Opening file...");
  File outfile = SPIFFS.open(COCO_FILE, FILE_WRITE);  
  if (!outfile) {
    // Serial.println(F("Failed to create outfile"));
    return false;
  }
  // Serial.println("   // Serializing...\n\n   ");
  // SerializeJson(usersDoc, // Serial);
  // Serial.println("\n\n");
  int serBytes = // SerializeJson(usersDoc, outfile);
  if (serBytes == 0) {
    // Serial.println(F("Failed to write to outfile"));
    return false;
  }
  // Serial.print("   Wrote ");
  // Serial.print(serBytes);
  // Serial.println(" bytes.");
  // Serial.println("   Closing file...");
  outfile.close();
  // Serial.println("   Returning...");*/
  return true;
}