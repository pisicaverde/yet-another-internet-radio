#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
// nu uitati sa conectati miso, mosi, sck!! altfel nu merge.
#define BREAKOUT_CS     D8      // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    D3      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            10      // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ
#define BREAKOUT_RESET  D0      // VS1053 reset pin (output) --> XRET
#define CARDCS          -1      // SD Card command select pin (output)
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
WiFiClient       mp3client ;

#include <Ticker.h>
Ticker ticker1Hz;

//#include "SH1106.h" 
//SH1106 display(0x3c, D1, D2); // lcd
#include <brzo_i2c.h>
#include "SH1106Brzo.h"
SH1106Brzo display(0x3c, D1, D2);

#define DATA_BUFFER_SIZE      20000    // cati bytes stocam in data buffer intern
unsigned short writePointer = 0;
unsigned short readPointer  = 0;     // unsigned short < 65535
byte byteBuffer[DATA_BUFFER_SIZE];   // bufferul circular

#define VS_BUFFER_SIZE        32     // cati bytes ii dam odata
byte vsBuffer[VS_BUFFER_SIZE];       // VS minibuffer -  bufferul in care compunem ce se da la VS

boolean endOfHeaders = false;
byte tmpChr;
unsigned int metaInt = 0;             // volumul de date mp3 intre doua blocuri de metadata
unsigned int metaLength = 0;
unsigned int i = 0;
String strRequest ;
String strAnswer ;

#define HASHSIZE   1024               // la cati bytes transferati de la server se afiseaza un #

#define WIFISSID "ssid"
#define WIFIPASS "pass"

const char * playServer     = "184.154.145.114";
const char * playPath       = "/" ; 
const unsigned int playPort = 8016;

String metaDataTxt_tmp = "Listening...";
String metaDataTxt ;
int j = 0;

////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  pinMode(BREAKOUT_CS, OUTPUT);
  pinMode(BREAKOUT_DCS, OUTPUT);
  pinMode(DREQ, INPUT);
  pinMode(BREAKOUT_RESET, OUTPUT);
  SPI.begin();
  SPI.setFrequency(1000000);
  
  display.init(); display.displayOn(); display.setFont(ArialMT_Plain_10); 

  Serial.println(F("\n\n------BEGIN------\n\n"));
  Serial.println(F("--> Initialising WiFi"));

  display.clear(); display.drawStringMaxWidth(0,0,128, "Connecting to WiFI..."); display.display();
  WiFi.hostname(F("iRadio"));
  WiFi.begin(WIFISSID, WIFIPASS);
  while (WiFi.status() != WL_CONNECTED) { Serial.print(F(".")); delay(150); }
  Serial.print(F("\n--> Connected to AP. IP: ")); Serial.println(WiFi.localIP()) ;
  display.clear(); display.drawStringMaxWidth(0,0,128, "Connected"); display.display();

  Serial.println(F("--> VS1053 GPIO test")); 
  if (!musicPlayer.begin()) { Serial.println(F("--> ERROR: VS1053 not found. Program stopped")); while(1){yield();} }
  Serial.println(F("--> BEEP")); 
  musicPlayer.softReset(); delay(150);
  musicPlayer.setVolume(0, 0);     // 0 = cel mai tare
  musicPlayer.sineTest(0x44, 70);  // Make a tone to indicate VS1053 is working

  ticker1Hz.attach_ms(1000, func1Hz);

  Serial.println(F("--> Connecting to stream..."));
  display.clear(); display.drawStringMaxWidth(0,0,122, String("Connecting to ") + String(playServer) + " : " + String(playPort) + String(playPath) ); display.display();
  int connectResult = mp3client.connect(playServer, playPort);
  if (connectResult)
  {
    Serial.println(F("--> Connected!")) ;
    strRequest = "GET " + String(playPath) + " HTTP/1.1\r\n" + "User-Agent: greetings_from_Romania! cip@pisicaverde.ro\r\n" + "Host: " + playServer + "\r\n" + "Icy-Metadata:1\r\n" + "Connection: keep-alive\r\n\r\n";
    Serial.println(F("--> Request is: ")); Serial.println(strRequest); 
    mp3client.print(strRequest);
    Serial.println(F("--> Headers sent"));
  }
  else {Serial.print(F("--> ERROR ClientConnect: ")); Serial.println(connectResult);  mp3client.stop(); Serial.print("--> Program stopped."); while(1){yield();} }
  
  Serial.println(F("--> Receiving answer headers")); delay(2000); // asteptam un pic sa vina datele

  endOfHeaders = false; strAnswer = "";
  while ( mp3client.available() && (endOfHeaders == false) ) // citim pana intalnim \r\n\r\n , dupa aceea vin datele de mp3
    { 
      tmpChr = mp3client.read();
      Serial.write(tmpChr);
      strAnswer = strAnswer + char(tmpChr);
      if (strAnswer.endsWith("\r\n\r\n")) { endOfHeaders = true; }
    }
  Serial.println(F("\r\n--> All headers received."));

int metaIntStart = strAnswer.indexOf("icy-metaint:") + 12;
int metaIntEnd   = strAnswer.indexOf("\r\n", metaIntStart);
metaInt = strAnswer.substring(metaIntStart, metaIntEnd).toInt();

Serial.print(F("--> MP3 data bytes between metadata blocks: metaInt = ")); Serial.println(metaInt); 
Serial.print(F("--> Printing # for each ")); Serial.print(HASHSIZE); Serial.println(F(" downloaded bytes."));
Serial.print(F("--> Printing . for each ")); Serial.print(VS_BUFFER_SIZE); Serial.println(F(" bytes sent to VS1053."));
display.clear(); display.drawStringMaxWidth(0,0,122, metaDataTxt); display.display();
}



////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  // BUFFER WRITE
  if (mp3client.available()) {  
        // daca suntem in zona de MP3 DATA si bufferul nu este full: citim un byte si il scriem in buffer; cand ajunge la capat, writePointer se reseteaza. Altfel ignoram
        if (( i < metaInt ) && (  !((writePointer + 1) % DATA_BUFFER_SIZE == readPointer)    ) ) { 
           byteBuffer[writePointer] = mp3client.read(); 
           if (!(i%HASHSIZE)) { Serial.print(F("#")); }
           writePointer = (writePointer + 1) % DATA_BUFFER_SIZE;  // se incrementeaza pozitia cu 1 sau sare pe pozitia 0 daca era la capat
           if (writePointer == readPointer) { readPointer = (readPointer + 1) % DATA_BUFFER_SIZE; }// daca scriem mai repede decat citim, atunci trebuie mutat cu 1 pozitie si readPointer
        }  // DE VERIFICAT: nu cumva cu acest IF doar pierdem byte-ul ?
        
        // DETECT META MARKER: daca s-au citit metaInt bytes, urmatorul octet * 16 = cati bytes de meta info urmeaza
        if (i == metaInt) { metaLength = mp3client.read() * 16; 
                            if (metaLength != 0) {Serial.print(F("\r\n--> ")); Serial.print(metaLength); Serial.print(F(" Bytes of metadata: ")); } }

        // IN META ZONE: preluam meta info, nu bagam nimic in buffer
        if ( (i > metaInt) && (i <= (metaInt + metaLength ) ) ) { char tmpChr = mp3client.read() ; Serial.write(tmpChr); 
                                                                  metaDataTxt_tmp += tmpChr; j++; 
                                                                  if (j >= metaLength) {  // s-a obtinut un metaData complet...
                                                                          int metaDataStart = metaDataTxt_tmp.indexOf("='") + 2 ; int metaDataEnd = metaDataTxt_tmp.indexOf("';", metaDataStart);
                                                                          metaDataTxt = metaDataTxt_tmp.substring(metaDataStart, metaDataEnd);
                                                                          j = 0 ; metaDataTxt_tmp = "";
                                                                          screenUpdate();
                                                                          }
                                                                 }

        // SFARSIT- s-a terminat un frame, s-a terminat de afisat meta info, bagam un enter
        if ( i == (metaInt + metaLength +1) ) {  i = 0; Serial.println(); } 
            else i++;
  } else Serial.print("/");

  if ((usedBuffer() >= VS_BUFFER_SIZE)) { //daca in bufferul de date s-a strans destul cat sa compunem bufferul audio => buffering complete...

          while (!digitalRead(DREQ)) { yield(); } // daca DREQ==low, e cel mai probabil ca acum playeaza ceva; asteptam cateva ms sa isi goleasca bufferul de vs si sa ceara date

          for (int j = 0; j < VS_BUFFER_SIZE ; j++) { // compunem bufferul care se transmite la VS
              vsBuffer[j] = byteBuffer[readPointer];
              readPointer = (readPointer + 1) % DATA_BUFFER_SIZE;
           }
          Serial.print(".");
          musicPlayer.playData(vsBuffer, VS_BUFFER_SIZE);    // playem ce s-a scris in vs buffer
              
   } 

     // daca intre timp s-a deconectat, se opreste tot
     if (!mp3client.connected()) { Serial.println(F("\r\n\r\n--> DISCONNECTED from stream. Program stopped.")); mp3client.stop(); while(1){yield();} } 
}
