//#include <ESPUI.h>
#include <Arduino.h>
#include <ESPUI.h>

uint16_t wUserId, wUserName, wUserNUID, wUserCoffee, wUserSave;
uint16_t wNewNUID, wNewName, wUserAdd;

void select_user_cb(Control* sender, int value);
void setupESPUI();
void save_user(Control* sender, int type);
void add_user(Control* sender, int type);


void textCallback(Control *sender, int type) {
	//This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

void select_user_cb(Control* sender, int value){
    Serial.print("Name selector CB: id=");
    Serial.println(sender->value);
    Serial.print("User size: ");
    Serial.println(j_users.size());
    int id = -1;
    for (int i=0; i<j_users.size(); i++){
      int curId = sender->value.toInt();
      /*Serial.print("  Testing ");
      Serial.print(curId);
      Serial.print("... ");*/
      if (j_users[i]["id"].as<int>() == curId){
        id = curId;
        //Serial.println("Yup.");
      }else{
        //Serial.println("Nope.");
      }
    }
    if (id == -1){
      Serial.println("User not found");
      return;
    }

    const char* name = j_users[id]["name"].as<const char*>();
    //String uid = j_users[id]["id"].as<String>();
    Serial.print(name);
    Serial.print("->");
    Serial.println(j_users[id]["coffee"].as<int>());

    ESPUI.updateControlValue(wUserId, j_users[id]["id"].as<String>());
    ESPUI.updateControlValue(wUserName, j_users[id]["name"].as<String>());
    ESPUI.updateControlValue(wUserNUID, j_users[id]["nfc"].as<String>());
    ESPUI.updateControlValue(wUserCoffee, j_users[id]["coffee"].as<String>());
}

void setupESPUI(){
  uint16_t tab_status = ESPUI.addControl(ControlType::Tab, "Status", "Status");
  uint16_t tab_users = ESPUI.addControl(ControlType::Tab, "Users", "Users");
  uint16_t tab_adduser = ESPUI.addControl(ControlType::Tab, "Add User", "Add User");

  // shown above all tabs
  

  uint16_t select_user = ESPUI.addControl(ControlType::Select, "User:", "", ControlColor::Alizarin, tab_users, &select_user_cb);
  Serial.print("User size: ");
  int jSize = j_users.size();
  Serial.println(jSize);
  for (int i=0; i<jSize; i++){
    const char* name = j_users[i]["name"].as<const char*>();
    String id = j_users[i]["id"].as<String>();
    if (i != id.toInt()){
      Serial.print("ERROR: ID inconsistency: ");
      Serial.print(i);
      Serial.print(" vs ");
      Serial.print(id);
    }
    userLabels[i] = ESPUI.addControl(ControlType::Label, j_users[i]["name"].as<const char*>(), j_users[i]["coffee"].as<String>(), ControlColor::Turquoise, tab_status);
    ESPUI.addControl(ControlType::Option, name, id, ControlColor::Alizarin, select_user);
  }
  wUserId = ESPUI.addControl(ControlType::Label, "ID", "", ControlColor::Alizarin, tab_users);
  wUserName = ESPUI.addControl(ControlType::Text, "Name:", "", ControlColor::Alizarin, tab_users, textCallback);
  wUserNUID = ESPUI.addControl(ControlType::Text, "NUID:", "", ControlColor::Alizarin, tab_users, textCallback);
  wUserCoffee = ESPUI.addControl(ControlType::Number, "Coffee:", "0", ControlColor::Alizarin, tab_users, textCallback);
  wUserSave = ESPUI.addControl(ControlType::Button, "Save User", "Save", ControlColor::Peterriver, tab_users, &save_user);
  //ESPUI.text()

  wNewNUID = ESPUI.addControl(ControlType::Text, "NUID:", "", ControlColor::Alizarin, tab_adduser, textCallback);
  wNewName = ESPUI.addControl(ControlType::Text, "Name:", "", ControlColor::Alizarin, tab_adduser, textCallback);
  wUserAdd = ESPUI.addControl(ControlType::Button, "Add User", "Save", ControlColor::Peterriver, tab_adduser, &add_user);

  /*
    * .begin loads and serves all files from PROGMEM directly.
    * If you want to serve the files from LITTLEFS use ESPUI.beginLITTLEFS
    * (.prepareFileSystem has to be run in an empty sketch before)
    */

  // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
  // ESPUI.sliderContinuous = true;

  /*
    * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
    * SECURE way of limiting access.
    * Anyone who is able to sniff traffic will be able to intercept your password
    * since it is transmitted in cleartext. Just add a string as username and
    * password, for example begin("ESPUI Control", "username", "password")
    */

  ESPUI.begin("CoCo Control");
}

void updateESPUI(){
  //
}

void save_user(Control* sender, int type){
  if (type == B_UP){
    return;
  }  
  int uid = ESPUI.getControl(wUserId)->value.toInt();
  String name = ESPUI.getControl(wUserName)->value;
  String nuid = ESPUI.getControl(wUserNUID)->value;
  int coffee = ESPUI.getControl(wUserCoffee)->value.toInt();
  Serial.print("Retrieved user data: ");
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(nuid);
  Serial.print(" -> ");
  Serial.println(coffee);
  //Serial.println(String("param: ") + String(int(param)));
  Serial.println("Checking user data...");
  if (uid < 0 || uid > 29){
    Serial.print("Error: uid out of bound: ");
    Serial.println(uid);
    while (1){}
  }
  if (coffee < 0 || coffee > 65535){
    Serial.print("Error: coffee out of bound: ");
    Serial.println(coffee);
    while (1){}
  }
  Serial.print("Setting user data: ");
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(nuid);
  Serial.print(" -> ");
  Serial.println(coffee);
  j_users[uid]["name"] = name;
  j_users[uid]["nuid"] = nuid;
  j_users[uid]["coffee"] = coffee;
  Serial.print("Stored user data: ");
  Serial.print(j_users[uid]["name"].as<const char*>());
  Serial.print(" = ");
  Serial.println(j_users[uid]["nuid"].as<const char*>());
  Serial.print(" -> ");
  Serial.println(j_users[uid]["coffee"].as<const char*>());

  Serial.println("Saving user data...");
  if (save_json_data()){
    Serial.println("Data saved successfully.");
  }else{
    Serial.println("Writing failed.");
    while(1){}
  }
  //ESPUI.updateControl(button1);
}

void add_user(Control* sender, int type){
  if (type == B_UP){
    return;
  }
  Serial.print("Adding new User: ");
  Serial.print(ESPUI.getControl(wNewName)->value.c_str());
  Serial.print(" = ");
  Serial.println(ESPUI.getControl(wNewNUID)->value.c_str());
}
