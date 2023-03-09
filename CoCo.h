#define MAX_USERS 30

#define COCO_FILE "/coco.json"
#define BUZZ_LENGTH 100

#define WIFI_UPTIME 60000*5
#define WIFI_NAME  "CoCo-WUI"
#define WIFI_PW     "naknak"

#define PIN_SOUNDS  3
//#define PIN_LED     5
#define PIN_LED     2
#define PIN_SWITCH  16

#if defined(ESP32)

//ESP32 pin defs
#define PIN_MISO  10
#define PIN_MOSI  3
#define PIN_SCK   2
#define PIN_CS    7
#define PIN_RST   0

#else

//ESP8266 pin defs
#define PIN_MISO  12
#define PIN_MOSI  13
#define PIN_SCK   14
#define PIN_CS    15
#define PIN_RST   4

#endif

#include <MFRC522.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(PIN_CS, PIN_RST);

const char* ssid = "ESPUI";
const char* password = "espui";
const char* hostname = "espui";

bool ui_active = false;
unsigned long closeUITime = 0;

byte nuidPICC[4] = {0, 0, 0, 0};

uint8_t usercnt = 0;
String usernames[MAX_USERS];
String nuids[MAX_USERS];
uint16_t coffees[MAX_USERS];

uint16_t userLabels[MAX_USERS];

String lastNuid = "";
int buzzTimes = 0;
bool buzzState = false;

unsigned long nextBuzzCycle = 0;

void sound(int times);
void readJSON();
void addCoffee(int uid);
bool save_json_data();
void save_user(Control* sender, int type);
void add_user(Control* sender, int type);
void nfcRegistered(String nuid);
void readRFID(void);
void printHex(byte *buffer, byte bufferSize);
String nuid2str(byte *buffer, byte bufferSize);
