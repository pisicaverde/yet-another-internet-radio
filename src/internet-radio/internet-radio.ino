#define FS_NO_GLOBALS             //allow spiffs to coexist with SD card, define BEFORE including FS.h , altfel conflicteaza cu libraria SD de Adafruit
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
bool shouldSaveConfig = false;    //flag for saving data
WiFiManager wifiManager;

#include <Arduino.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <LiquidCrystal_I2C.h>

#include <Adafruit_VS1053.h>    // nu uitati sa conectati miso=D6, mosi=D7, sck=D5 !! altfel nu merge.
#define BREAKOUT_CS     D4      // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    D3      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            10      // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ (esp: GPIO10 = SD3
#define BREAKOUT_RESET  D0      // VS1053 reset pin (output) --> XRET
#define CARDCS          -1      // SD Card command select pin (output)
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

#include <ESP8266WiFi.h>
WiFiClient       mp3client ;

#define SRAM_CS 3               // SRAM CHIP SELECT 23LC1024
#define RDSR    0x05            //SRAM opcodes
#define WRSR    0x01
#define READ    0x03
#define WRITE   0x02

#include <Ticker.h>
Ticker ticker1Hz;
Ticker ticker1kHz;

#include <LiquidCrystal_I2C.h>  // lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define DATA_BUFFER_SIZE    125000    // cati bytes stocam in data buffer intern / memCapacity
unsigned int writePointer = 0;       // pentru bufferul circular
unsigned int readPointer  = 0;       // < 125000

#define VS_BUFFER_SIZE      32       // cati bytes se transmit deodata catre VS
byte vsBuffer[VS_BUFFER_SIZE];       // VS minibuffer - bufferul in care compunem ce se da la VS

// lista posturilor
char useragent[20] = "";             // semnatura din GET 
char jsonStations[5000];             // contine datele din fisierul de configurare - lista posturilor [ http://pisicaverde.ro/irconfig.dat ] in format json
char jsonURL[100] = "";              // este scris in SPIFFS de WiFiManager si contine URL-ul de unde se descarca fisierul anterior

struct stations {
    String title, host, path;
    unsigned int port;
};
stations stationList[50] ;  // array de struct-uri in care se salveaza informatiile in RAM dupa parsarea jsonStations

// descarcarea si parsarea stream-ului audio:
boolean endOfHeaders = false;        // flag pentru cand intalnim \r\n\r\n in streamul de date
unsigned int metaInt = 0;            // volumul de date mp3 intre doua blocuri de metadata, depinde de stream
unsigned int metaLength = 0;         // lungimea unui bloc de meta-info (titlul melodiei etc)
unsigned int byteCnt = 0;            // variabila de lucru, contorizeaza la al catelea byte dintr-un ciclu de date suntem
byte tmpChr;                         // byte de lucru
String strRequest ;                  // string de lucru, contine GET requestul
String strAnswer ;                   // string de lucru, contine raspunsul la GET

String metaStationName, metaBR, metaGenre; // stringuri care provin din headerul HTTP de la postul de radio
String metaDataTxt_tmp = "";
String metaDataTxt ;
byte txtScroll = 0 ;        // variabila folosita la scroll-ul textului pe lcd
int  txtDir    = 1;         // directia de scroll (-1 / +1)

// playback stuff
byte stationNow = 0 ;     // id-ul postului playat acum, din stationList
byte prevStationNow = 0;  // folosit ca sa testam daca s-a schimbat postul; folosit numai in showStdStat(); sa nu facem redesenare ecran daca nu s-a schimbat postul
byte stationCnt = 0;      // numarul total de posturi, este calculat dupa parsarea stationList

// in EEPROM salvam id-ul postului curent; daca intre timp se schimba json-ul si se face mai mic, id -> 0
#include <EEPROM.h>
#define EEPROM_ADDR 0     // adresa la care salvam byte-ul cu nr. postului

// diverse flags si counters
boolean BUFFER_EMPTY      = true;  // flag setat cand bufferul devine complet gol
boolean SCREEN_UPDATE     = false; // redraw-ul ecranului dureaza, asa ca setam flagul in functie (sa nu incetinim) si o tratam extern
boolean PLAYERPAUSE       = true;  // status - daca playeaza sau e pauzat
unsigned long lcdStandBy  = 0 ;    // cand schimbam posturile in stand-by, contine timestampul dupa care se revine la ceas
#define SB_IDLE           4000     // cand schimbam posturile in stand-by, contine durata [ms] dupa care se revine la ceas
#define SOFTDEBOUNCE      250      // dupa cat timp [ms] acceptam apasarea de butoane in standby, calculat de la prima apasare
boolean FIRSTCLOCK        = true; // 

// watchdog flags
byte emptyBufferCount = 0 ; // contor pentru de cate ori s-a gasit succesiv bufferul complet gol in func1Hz()

// keypad - average readings
static const unsigned int key1 = 105 ;    // left
static const unsigned int key2 = 186 ;    // ok
static const unsigned int key3 = 253 ;    // right
byte keyLast          = 0;                // valoarea ultimei taste apasate (chiar daca intre timp s-a luat degetul)
unsigned int keyVal   = 0;                // valoarea citita de analogRead


#include <Time.h>         //https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
#include <WiFiUdp.h>
unsigned int localPort = 2390;      // local port to listen for UDP packets
WiFiUDP udp;                        // A UDP instance to let us send and receive packets over UDP
IPAddress timeServerIP;              // time.nist.gov NTP server address
const char* ntpServerName = "0.pool.ntp.org";
const int NTP_PACKET_SIZE = 48;      // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

#include "PROGMEM_readAnything.h"


////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
   
  Serial.begin(115200); delay(100);
  Serial.print(F("\r\n\r\n-->INTERNET RADIO IS STARTING ; LAST UPDATE: ")); Serial.print(__DATE__); Serial.print(", "); Serial.print(__TIME__); Serial.println("\r\n\r\n");
  
  pinMode(BREAKOUT_CS, OUTPUT); pinMode(BREAKOUT_DCS, OUTPUT); pinMode(DREQ, INPUT); pinMode(BREAKOUT_RESET, OUTPUT); pinMode(SRAM_CS, OUTPUT); // digital in+out
  pinMode(A0, INPUT); // butoanele
  SPI.begin(); SPI.setFrequency(16000000L);  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  lcd.begin(); lcd.backlight(); 
  
  randomSeed(analogRead(A0));

  welcomeMsg(); 
  //sram_setup();            // citeste modul in care este 23LC1024 si face cateva teste scriere / citire RAM ; il comentam pentru ca bazaie in difuzor
  vsInit();                // beep test

  // daca se booteaza cu tasta 1 apasata, atunci stergem info de wifimanager
  if (keyRead() == 1) { Serial.println(F("--> Button1 pressed => WIFI data has been deleted.")); wifiManager.resetSettings();
                        //Serial.print(F("--> Formatting SPIFFS...")); SPIFFS.format(); Serial.print(F(" done."));  // formatarea poate dura si 30s
                      }   

  wifiCustConnect();      // conectare la wifi; daca esueaza, genereaza hotspot unde se introduce si salveaza ssid/pass/ URL json configurare
  lcdCreateChars();       // creaza caracterele grafice pt bigscreen
  //updateTime();           // update ntp
  getStationData();       // 1) citeste din SPIFFS url-ul unde se afla lista de posturi, 2) o descarca de pe net
  jsonParseData();        // parseaza jsonul descarcat de pe net si salveaza datele in struct
  stationRead();          // citim din eeprom id-ul postului cu care se incepe
  emptyBufferCount = 10;  // asta ca sa fortam un forcefill la prima pornire
  
  ticker1Hz.attach_ms(1000, func1Hz); // la fiecare 1 s se ruleaza functia func1Hz
  ticker1kHz.attach_ms(1, func1kHz);  // la fiecare milisecunda se ruleaza asta ; aici se baga chestiile rapide

  lcdStandBy = millis();

  Serial.println(F("--> END OF SETUP."));
  lcd.setCursor(0,2); lcd.print(F("  All done! Enjoy!  ")); delay(2000); lcd.clear(); lcd.noBacklight(); 
}


////////////////////////////////////////////////////////////////////////////////////////////


void loop() {
  // in loop pastram functiile mai lente; playul este in ticker1Hz

if (PLAYERPAUSE == false ) { 

   // daca a picat netul, continuam sa cantam daca mai avem ce, si in paralel conectare.
   if (WiFi.status() != WL_CONNECTED) { Serial.print(F("--> Disconnected from WiFi, trying to reconnect.")); wifiCustConnect(); }

   // de obicei facem bufferFill; doar daca e grava, facem ForceFill
   if (emptyBufferCount >= 4) { forceDisconnect(); bufferForceFill(); }  else { bufferFill(); }  

}

   // daca s-a facut momentul pentru redesenare ecran
   if (SCREEN_UPDATE == true) { screenUpdate(); }

}
