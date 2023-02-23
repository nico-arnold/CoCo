//#include <ESPUI.h>
#include <Arduino.h>
#include <ESPUI.h>

uint16_t tab_status;
uint16_t tab_users;
uint16_t tab_adduser;
uint16_t select_user;
uint16_t wUserId, wUserName, wUserNUID, wUserCoffee, wUserSave;
uint16_t wNewNUID, wNewName, wUserAdd;

void select_user_cb(Control* sender, int value);
void setupESPUI();
void save_user(Control* sender, int type);
void add_user(Control* sender, int type);


void textCallback(Control *sender, int type) {
	//This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

void updateUI(){
  for (int i=0; i<usercnt; i++){
    ESPUI.updateLabel(userLabels[i], String(coffees[i]));
    ESPUI.updateControlLabel(userLabels[i], usernames[i].c_str());
  }
}

void select_user_cb(Control* sender, int value){
    Serial.print("Name selector CB: id=");
    Serial.println(sender->value);
    int id = sender->value.toInt();
    //int id = -1;
    /*for (int i=0; i<usercnt; i++){
      int curId = sender->value.toInt();
      
      if (j_users[i]["id"].as<int>() == curId){
        id = curId;
        //Serial.println("Yup.");
      }else{
        //Serial.println("Nope.");
      }
    }*/
    if (id >= usercnt){
      Serial.println("User not found");
      return;
    }

    const char* name = usernames[id].c_str();
    //String uid = j_users[id]["id"].as<String>();
    Serial.print(name);
    Serial.print("->");
    Serial.println(coffees[id]);

    ESPUI.updateControlValue(wUserId, String(id));
    ESPUI.updateControlValue(wUserName, usernames[id]);
    ESPUI.updateControlValue(wUserNUID, nuids[id]);
    ESPUI.updateControlValue(wUserCoffee, String(coffees[id]));
}

void setupESPUI(){
  tab_status = ESPUI.addControl(ControlType::Tab, "Status", "Status");
  tab_users = ESPUI.addControl(ControlType::Tab, "Users", "Users");
  tab_adduser = ESPUI.addControl(ControlType::Tab, "Add User", "Add User");

  // shown above all tabs
  

  select_user = ESPUI.addControl(ControlType::Select, "User:", "", ControlColor::Turquoise, tab_users, &select_user_cb);
  for (int i=0; i<usercnt; i++){
    const char* name = usernames[i].c_str();
    String id = String(i);
    /*if (i != id.toInt()){
      Serial.print("ERROR: ID inconsistency: ");
      Serial.print(i);
      Serial.print(" vs ");
      Serial.print(id);
    }*/
    userLabels[i] = ESPUI.addControl(ControlType::Label, usernames[i].c_str(), String(coffees[i]), ControlColor::Turquoise, tab_status);
    ESPUI.addControl(ControlType::Option, name, id, ControlColor::Turquoise, select_user);
  }
  wUserId = ESPUI.addControl(ControlType::Label, "ID", "", ControlColor::Turquoise, tab_users);
  wUserName = ESPUI.addControl(ControlType::Text, "Name:", "", ControlColor::Turquoise, tab_users, textCallback);
  wUserNUID = ESPUI.addControl(ControlType::Text, "NUID:", "", ControlColor::Turquoise, tab_users, textCallback);
  wUserCoffee = ESPUI.addControl(ControlType::Number, "Coffee:", "0", ControlColor::Turquoise, tab_users, textCallback);
  wUserSave = ESPUI.addControl(ControlType::Button, "Save User", "Save", ControlColor::Turquoise, tab_users, &save_user);
  //ESPUI.text()

  wNewNUID = ESPUI.addControl(ControlType::Text, "NUID:", "", ControlColor::Turquoise, tab_adduser, textCallback);
  wNewName = ESPUI.addControl(ControlType::Text, "Name:", "", ControlColor::Turquoise, tab_adduser, textCallback);
  wUserAdd = ESPUI.addControl(ControlType::Button, "Add User", "Save", ControlColor::Turquoise, tab_adduser, &add_user);

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
  /*j_users[uid]["name"] = name;
  j_users[uid]["nuid"] = nuid;
  j_users[uid]["coffee"] = coffee;*/
  usernames[uid] = name;
  nuids[uid] = nuid;
  coffees[uid] = coffee;
  Serial.print("Stored user data: ");
  Serial.print(usernames[uid]);
  Serial.print(" = ");
  Serial.println(nuids[uid]);
  Serial.print(" -> ");
  Serial.println(coffees[uid]);

  Serial.println("Saving user data...");
  if (save_json_data()){
    Serial.println("Data saved successfully.");
  }else{
    Serial.println("Writing failed.");
    while(1){}
  }
  updateUI();
}

void add_user(Control* sender, int type){
  if (type == B_UP){
    return;
  }
  const char* name = ESPUI.getControl(wNewName)->value.c_str();
  const char* nuid = ESPUI.getControl(wNewNUID)->value.c_str();
  Serial.print("Adding new User: ");
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(nuid);
  usernames[usercnt] = name;
  nuids[usercnt] = nuid;
  coffees[usercnt] = 0;
  userLabels[usercnt] = ESPUI.addControl(ControlType::Label, name, String("0"), ControlColor::Turquoise, tab_status);
  ESPUI.addControl(ControlType::Option, name, String(usercnt), ControlColor::Turquoise, select_user);
  usercnt++;
  save_json_data();
  ESPUI.updateText(wNewNUID, "");
  updateUI();
}
