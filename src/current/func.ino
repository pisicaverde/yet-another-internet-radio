void func1kHz() {         // functia asta trebuie sa fie cat mai rapida, sa nu intrerupa playbackul

if (PLAYERPAUSE == false) {
    
    if (BUFFER_EMPTY)         { return; } // daca avem acest flag setat, oricum nu mai avem ce cauta aici
    if (!digitalRead(DREQ))   { return; } // daca DREQ==high , atunci NU are nevoie de date pt ca playeaza
    
    if (usedBuffer() >= VS_BUFFER_SIZE) { //daca in bufferul de date s-a strans destul cat sa compunem bufferul audio...
          
              for (int j = 0; j < VS_BUFFER_SIZE ; j++) { vsBuffer[j] = dataBufferRead(); } // compunem bufferul care se transmite la VS
              musicPlayer.playData(vsBuffer, VS_BUFFER_SIZE);   // playem ce s-a scris in vs buffer
              }   
              else { BUFFER_EMPTY = true ; }
} 

byte keyTmp = keyRead();                // citim la 1 kHz dar tratam la 1 Hz
if (keyTmp != 0) { keyLast = keyTmp; }  // daca s-a apasat orice ( 0 = nimic apasat), atunci salvam butonul apasat


}


////////////////////////////////////////////////////////////////////////////////////////////////

void func1Hz() {      // functia contine lucrurile mai putin prioritare

if ( (usedBuffer() < DATA_BUFFER_SIZE / 3) && (PLAYERPAUSE == false) ) { // daca suntem PAUSED nu mai afisam bufferul aiurea pe seriala
                                                                         Serial.print("--> UB="); Serial.print(usedBuffer()); Serial.print(F(" B @ ")) ; 
                                                                         Serial.print(map(usedBuffer(), 1, DATA_BUFFER_SIZE, 1, 100)); Serial.println("%"); }

SCREEN_UPDATE = true;

if ((usedBuffer() < VS_BUFFER_SIZE)  && (PLAYERPAUSE == false) ) { emptyBufferCount++; Serial.print(F("--> empty buffer count = ")); Serial.println(emptyBufferCount); } 
                              else { emptyBufferCount = 0; }
                              
if (keyLast != 0) {Serial.print(F("--> Key=")) ; Serial.print(keyLast); Serial.print(F("   A0=")); Serial.println(keyVal); }

if (keyLast == 1) { Serial.println(F("--> PREV key pressed")); keyLast = 0; switchPrevButton(); /*switchPrev(); */ }
if (keyLast == 3) { Serial.println(F("--> NEXT key pressed")); keyLast = 0; switchNextButton() ; /*switchNext(); */ }

if (keyLast == 2) { Serial.println(F("--> PAUSE key pressed")); 
                    keyLast = 0;
                    
                    unsigned int startTime = millis(); // detectare long press ; dar crapa daca tin apasat
                    while (millis() < startTime + 500) { delay(1); yield; } 
                    if (keyRead() == 2) Serial.println(F("---> LONG KEY PRESS on Pause ; should save song title..."));

                    if (PLAYERPAUSE == false) { PLAYERPAUSE = true; Serial.println(F("--> BEGIN PAUSE")); forceDisconnect(); writePointer = 0; readPointer  = 0; lcd.clear();} // daca inainte era pe play
                                        else  { PLAYERPAUSE = false; Serial.println(F("--> END OF PAUSE")); }  // iesim din pauza
                  }  
}


////////////////////////////////////////////////////////////////////////////////////////////////

void forceDisconnect() {
  mp3client.stop(); delay(500);
  Serial.print(F("--> Disconnecting from stream if previously connected: "));
  while(mp3client.connected()) { while(mp3client.available()) { mp3client.read(); Serial.print("."); } } // singura posibilitate de a goli ce mai era in buffer
  mp3client.stop();  Serial.println(F("\r\n--> Disconnected and buffer flushed.")); delay(500);
}


///////////////////////////////////////////////////////////////////////////////////////////


void screenUpdate() {

  if (!SCREEN_UPDATE) return; // daca cumva ajungem aici cand nu trebuie, iesim imediat 

  if (PLAYERPAUSE == true) 
           { 
            if (millis() < lcdStandBy) { lcd.backlight(); showStdStat(); } // afiseaza meniul de selectie post
                                  else { lcd.noBacklight(); 
                                         // if ( second() %2 ) lcd.clear(); 
                                         showClockScr(); }
           } 
           else { showPlayScr(); }

  SCREEN_UPDATE = false;
}

///////////////////////////////////////////////////////////////////////////////////////////

void showStdStat() {

  if (stationNow == prevStationNow) return; // nu s-a schimbat postul? nu avem ce sa redesenam 
  
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(F("- POSTURI SALVATE: -"));
  lcd.setCursor(0,1); lcd.print(" "); 
  if (stationNow > 0) { if (stationNow <= 9) lcd.print("0"); lcd.print(stationNow);  // leading zero
                        lcd.setCursor(3,1); lcd.print(F(".                "));
                        lcd.setCursor(5,1); lcd.print(stationList[stationNow-1].title.substring(0,15)); 
                      } else { lcd.setCursor(0,1); lcd.print(F("BEGIN               ")); }
  
  lcd.setCursor(0,2); lcd.write(B01111110) ;            
  if (stationNow+1 <= 9) lcd.print("0"); lcd.print(stationNow+1); 
  lcd.setCursor(3,2); lcd.print(F(".                "));
  lcd.setCursor(5,2); lcd.print(stationList[stationNow].title.substring(0,15));
  
  lcd.setCursor(0,3); lcd.print(" "); 
  if (stationNow < stationCnt-1 ) { if (stationNow+2 <= 9) lcd.print("0"); lcd.print(stationNow+2); 
                                    lcd.setCursor(3,3); lcd.print(F(".                "));
                                    lcd.setCursor(5,3); lcd.print(stationList[stationNow+1].title.substring(0,15)); 
                                  } else { lcd.setCursor(0,3); lcd.print(F("END                 ")); }

  prevStationNow = stationNow; // la urmatoarea apelare a functiei, nu mai avem nimic nou de afisat si iesim imediat
}

///////////////////////////////////////////////////////////////////////////////////////////

void showPlayScr() {
  
  // cateva calcule pt. valorile care se afiseaza in interfata
  int db = WiFi.RSSI(); 
  int q;
  if (db >= -50) { q = 100; }
  else if (db <= -100) { q = 0 ;}
  else {q = 2 * ( WiFi.RSSI() + 100) ; } // q este RSSI in procente

  lcd.setCursor(0,0); lcd.print(F("Station ")); lcd.print(stationNow+1, DEC); lcd.print("/"); lcd.print(stationCnt, DEC); 
 
  lcd.setCursor(15,0); 
  lcd.print( ( hour() <= 9 ? "0" : "") +  String(hour()) + " " + ( minute() <= 9 ? "0" : "") + String(minute()) ); 
  if (second() % 2) { lcd.setCursor(17,0); lcd.print(":"); } else { lcd.setCursor(17,0); lcd.print(" "); } 
    
  lcd.setCursor(0,1); lcd.print(metaStationName.substring(0,20));
  lcd.setCursor(0,2); lcd.print(metaDataTxt.substring(txtScroll, txtScroll+19)); //Serial.println(metaDataTxt.substring(txtScroll, txtScroll+19));
  lcd.setCursor(0,3); lcd.print(metaBR); lcd.print(F("kbps")); 
  lcd.setCursor(8,3); lcd.print(F("B:  % W:   %"));
  lcd.setCursor(10,3); lcd.print(map(usedBuffer(), 1, DATA_BUFFER_SIZE, 1, 99), DEC);
  lcd.setCursor(16,3); lcd.print(q, DEC);

  if (txtScroll == 0) txtDir = 1;
  if (txtScroll == (metaDataTxt.length() - 19) ) txtDir = -1;
  txtScroll = txtScroll + txtDir;

}

///////////////////////////////////////////////////////////////////////////////////////////

byte keyRead(){ // functia citeste valoarea de pe portul analog si intoarce butonul apasat cu toleranta +/- 10%

keyVal = analogRead(A0);
if ((keyVal >= key1 * 0.9) && (keyVal <= key1 * 1.1)) { return(1); } 
if ((keyVal >= key2 * 0.9) && (keyVal <= key2 * 1.1)) { return(2); } 
if ((keyVal >= key3 * 0.9) && (keyVal <= key3 * 1.1)) { return(3); } 
return(0) ; 
}




void jsonParseData() {

Serial.println(F("--> START PARSING JSON")); 
lcd.setCursor(0,2); lcd.print(F("Parsing remote data "));

DynamicJsonBuffer jsonBuffer;

JsonObject& root = jsonBuffer.parseObject(jsonStations);
//root.printTo(Serial); Serial.println();
if (root.success()) { Serial.println(F("--> SUCCESS")); } else { Serial.println(F("--> ERROR IMPORTING JSON STATION LIST -- fatal error")); 
                                                                 lcd.clear();
                                                                 lcd.setCursor(0,0); lcd.print(F("--> FATAL ERROR <--")); 
                                                                 lcd.setCursor(0,1); lcd.print(F("Error parsing remote")); 
                                                                 lcd.setCursor(0,2); lcd.print(F("json station list.")); 
                                                                 lcd.setCursor(0,3); lcd.print(F("Check file on server")); 
                                                                 delay(5000);
                                                                 }  // aici se moare

strcpy(useragent, root["agent"]) ; Serial.print(F("--> user agent: ")); Serial.println(useragent);

JsonArray& station = root["station"]; 
stationCnt = station.size();
Serial.print(F("--> NUMBER OF STATIONS: ")); Serial.println(stationCnt); 
  
  for (byte tmp = 0; tmp < stationCnt ; tmp++) {
    JsonObject& tmpStation = station[tmp];
    char tmpStationTitle[50];      strcpy(tmpStationTitle,      tmpStation["title"]);
    char tmpStationHost[50];       strcpy(tmpStationHost,       tmpStation["host"]);
    char tmpStationPath[50];       strcpy(tmpStationPath,       tmpStation["path"]);
    char tmpStationPort[50];       strcpy(tmpStationPort,       tmpStation["port"]);

    stationList[tmp].title = tmpStationTitle;
    stationList[tmp].host = tmpStationHost;
    stationList[tmp].path = tmpStationPath;
    stationList[tmp].port = String(tmpStationPort).toInt();

    Serial.print("\t"); Serial.print(tmp); Serial.print(" "); Serial.print(stationList[tmp].title); Serial.print(" ");   Serial.print(stationList[tmp].host); Serial.print(":");  
    Serial.print(stationList[tmp].port) ; Serial.println(stationList[tmp].path); 

  }

}

//////////////////////////////////////////////

void streamConnect() {
  if (PLAYERPAUSE == true ) return;
  if (mp3client.connected()) return; // daca suntem conectati, nu mai continuam
  Serial.print(F("--> Connecting to stream=station No. ")); Serial.print(stationNow); Serial.print(" of ") ; Serial.print(stationCnt);
  
  Serial.print(" ("); Serial.print(stationList[stationNow].title); Serial.print(") ");   Serial.print(stationList[stationNow].host); Serial.print(":"); 
  Serial.print(stationList[stationNow].port) ; Serial.println(stationList[stationNow].path); 

  lcd.clear(); lcd.backlight(); 
  lcd.setCursor(0,0); lcd.print(F("Connecting to: ")); lcd.print(stationNow+1, DEC); lcd.print("/"); lcd.print(stationCnt+1, DEC); 
  lcd.setCursor(0,1); lcd.print(stationList[stationNow].title);
  lcd.setCursor(0,2); lcd.print(stationList[stationNow].host); 
  String tmpStr1 = ":" + String(stationList[stationNow].port) + String(stationList[stationNow].path);
  lcd.setCursor(0,3); lcd.print(tmpStr1); 
   
  int connectResult = mp3client.connect(stationList[stationNow].host, stationList[stationNow].port);
  if (connectResult)
  {
    Serial.println(F("--> Connected!")) ;
    strRequest = "GET " + String(stationList[stationNow].path) + " HTTP/1.1\r\n" + "User-Agent: " + useragent +"\r\nHost: " + stationList[stationNow].host + "\r\n" + "Icy-Metadata:1\r\n" + "Accept: audio/mpeg,audio/aacp\r\nConnection: keep-alive\r\n\r\n";
    Serial.println(F("--> Sending request: ")); Serial.println(strRequest); 
    mp3client.print(strRequest);
    Serial.println(F("--> Headers sent"));
  }
  else {Serial.println(F("--> ERROR connecting to server. Server may be offline. Switching to next station.")); 
        //!!!display.clear(); display.drawStringMaxWidth(0,0,122, String("ERROR while connecting to ") + stationList[stationNow].host + " : " + String(stationList[stationNow].port) + stationList[stationNow].path ); display.display();
        musicPlayer.sineTest(1100, 200); musicPlayer.sineTest(1200, 200); 
        switchNext();
        return;      
        } 
  
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

  // raspunsuri valide: ICY 200 OK     sau    HTTP/1.0 200 OK
  if (strAnswer.indexOf(" 200 OK") == -1 ) { Serial.println(F("---> ERROR: answer does not contain 200 OK (nor ICY or HTTP); skipping & moving to next station.")); switchNext(); return; }                                                   

  metaInt = 0; metaStationName = ""; metaDataTxt = ""; metaBR = ""; metaGenre = ""; // resetam valorile, sa nu ramana cu valori de la posturile anterioare

  int paramStart, paramEnd;
  paramStart = strAnswer.indexOf("icy-metaint:") + 12; paramEnd = strAnswer.indexOf("\r\n", paramStart);  
  metaInt = strAnswer.substring(paramStart, paramEnd).toInt();

  paramStart = strAnswer.indexOf("icy-name:") + 9; paramEnd = strAnswer.indexOf("\r\n", paramStart);
  metaStationName = strAnswer.substring(paramStart, paramEnd);

  paramStart = strAnswer.indexOf("icy-br:") + 7; paramEnd = strAnswer.indexOf("\r\n", paramStart);
  metaBR = strAnswer.substring(paramStart, paramEnd);

  paramStart = strAnswer.indexOf("icy-genre:") + 10; paramEnd = strAnswer.indexOf("\r\n", paramStart);
  metaGenre = strAnswer.substring(paramStart, paramEnd);
  
  Serial.print(F("--> MP3 data bytes between metadata blocks: metaInt = ")); Serial.println(metaInt); 

  lcd.clear();
  screenUpdate();
}







void vsInit() {
  Serial.println(F("--> VS1053 GPIO test")); 
  if (!musicPlayer.begin()) { Serial.println(F("--> ERROR: VS1053 not found. Will continue with no sound and acting dumb.")); } 
  musicPlayer.softReset(); delay(150);
  musicPlayer.setVolume(0, 0);     // 0 = cel mai tare
  //musicPlayer.sineTest(1000, 100); musicPlayer.sineTest(1100, 100); musicPlayer.sineTest(1200, 100); // frecventa in Hz, durata in ms

  Serial.println(F("--> PLAY SOME SOUND FROM PROGMEM")); 
  unsigned int tmpPointer = 0;
  unsigned int endPointer = sizeof(static_mp3); Serial.print("--> Playing "); Serial.print(endPointer) ; Serial.println(" bytes of data.");
  byte tmp[VS_BUFFER_SIZE];
  while (tmpPointer <= endPointer) {    
         for (int i = 0; i < VS_BUFFER_SIZE; i++) { tmp[i] = PROGMEM_getAnything(&static_mp3[i+tmpPointer]) ;  } 
         while (!musicPlayer.readyForData()) { yield(); }  // se sta
         if (digitalRead(DREQ) == HIGH) {  // daca data request...
               musicPlayer.playData(tmp, VS_BUFFER_SIZE);
               tmpPointer = tmpPointer + VS_BUFFER_SIZE;
               yield();
               }
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////


void getStationData() {
    lcd.setCursor(0,2); lcd.print(F("Get local config... "));
    // ne conectam la url-ul din fisierul de configurare si descarcam de acolo json-ul cu lista de posturi
  if (SPIFFS.begin()) {
    Serial.println(F("--> MOUNTED FILESYSTEM"));
    if (SPIFFS.exists("/config.json")) { //file exists, reading and loading
      
      Serial.println(F("--> reading config file"));
      fs::File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(F("--> OPEN config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) { strcpy(jsonURL, json["jsonURL"]); } 
                       else { Serial.println(F("--> JSON PARSE ERROR")); 
                              lcd.clear();
                              lcd.setCursor(0,0); lcd.print(F("--> FATAL ERROR <--")); 
                              lcd.setCursor(0,1); lcd.print(F("Error parsing local")); 
                              lcd.setCursor(0,2); lcd.print(F("config data.")); 
                              lcd.setCursor(0,3); lcd.print(F("FLASHMEM error.")); 
                              delay(5000);
                            }  // aici se moare
        }
    } else { Serial.println(F("--> ERROR: missing config file config.json . Run SETUP to recreate it."));  
             lcd.clear();
             lcd.setCursor(0,0); lcd.print(F("--> FATAL ERROR <--")); 
             lcd.setCursor(0,1); lcd.print(F("Missing local json.")); 
             lcd.setCursor(0,2); lcd.print(F("FLASHMEM error."));
             lcd.setCursor(0,3); lcd.print(F("Run Setup to fix.")); 
             delay(5000);
           } // aici se moare
  } else { Serial.println(F("--> ERROR - failed to mount filesystem")); 
           lcd.clear();
           lcd.setCursor(0,0); lcd.print(F("--> FATAL ERROR <--")); 
           lcd.setCursor(0,1); lcd.print(F("Failed to mount")); 
           lcd.setCursor(0,2); lcd.print(F("local filesystem.")); 
           lcd.setCursor(0,3); lcd.print(F("FLASHMEM error.")); 
           delay(5000);
         } // aici se moare

  lcd.setCursor(0,2); lcd.print(F("Download remote list"));
  Serial.println(F("--> START getting station list json"));
  String tmpConfigHost, tmpConfigPath;
  String jsonURLstr(jsonURL);
  tmpConfigHost = jsonURLstr.substring(0, jsonURLstr.indexOf("/")); Serial.print("---> HOST:"); Serial.println(tmpConfigHost); // am extras domeniul
  tmpConfigPath = jsonURLstr.substring(jsonURLstr.indexOf("/") + 1 , jsonURLstr.length()); Serial.print("---> PATH:"); Serial.println(tmpConfigPath); // am extras ce vine dupa slash  

  int connectResult = mp3client.connect(tmpConfigHost, 80);
  
  if (connectResult)
  {
    Serial.println(F("--> Connected!")) ;
    strRequest = "GET /" + tmpConfigPath + " HTTP/1.1\r\nHost: " + tmpConfigHost + "\r\n\r\n";
    Serial.println(F("--> Sending request: ")); Serial.println(strRequest); 
    mp3client.print(strRequest);
    Serial.println(F("--> Headers sent"));
  }
  else { Serial.println(F("--> ERROR connecting to server - CANNOT GET STATION LIST")); 
         lcd.clear();
         lcd.setCursor(0,0); lcd.print(F("--> FATAL ERROR <--")); 
         lcd.setCursor(0,1); lcd.print(F("Cannot connect to")); 
         lcd.setCursor(0,1); lcd.print(F("remote config server")); 
         lcd.setCursor(0,2); lcd.print(F("Run SETUP again.")); 
       } // aici se moare

  // citim ce intoarce serverul
  Serial.println(F("--> RECEIVING DATA..."));
  endOfHeaders = false; strAnswer = "";
  delay(2000);
  int y = 0;
  while ( mp3client.connected()  && mp3client.available() ) // citim pana intalnim \r\n\r\n , dupa aceea vin datele utile
    { 
      tmpChr = mp3client.read();
      strAnswer = strAnswer + char(tmpChr);
      if (strAnswer.endsWith("\r\n\r\n")) { endOfHeaders = true; }
      if (endOfHeaders) { Serial.write(tmpChr); jsonStations[y] = tmpChr; y++;}
    }

  // folosim shitul asta pentru ca la un moment dat pica conexiunea in timpul transferului sau in orice caz se opreste transferul inainte de termen si el crede ca e complet. Dar nu e.
  delay(2000); Serial.print("--------------------------------------------"); 
  
  while ( mp3client.connected()  && mp3client.available() ) // citim pana intalnim \r\n\r\n , dupa aceea vin datele utile
    { 
      tmpChr = mp3client.read();
      strAnswer = strAnswer + char(tmpChr);
      if (strAnswer.endsWith("\r\n\r\n")) { endOfHeaders = true; }
      if (endOfHeaders) { Serial.write(tmpChr); jsonStations[y] = tmpChr; y++;}
    }

  
  mp3client.stop();
  Serial.println(F("\r\n--> ALL DONE"));
}



void switchNextButton() {
  if ( millis() < lcdStandBy - SB_IDLE + SOFTDEBOUNCE ) { Serial.println(F("--> debouncing")); return; }
  switchNext();  
  if (PLAYERPAUSE == true ) { Serial.println(F("--> Station changed to NEXT while in PAUSE")); lcdStandBy = millis() + SB_IDLE; }
}

void switchPrevButton() {
  if ( millis() < lcdStandBy - SB_IDLE + SOFTDEBOUNCE ) { Serial.println(F("--> debouncing")); return; }
  switchPrev();  
  if (PLAYERPAUSE == true ) { Serial.println(F("--> Station changed to PREV while in PAUSE")); lcdStandBy = millis() + SB_IDLE; }
}


void switchNext() {
  forceDisconnect();
  prevStationNow = stationNow ;
  if (stationNow < stationCnt-1) { stationNow++ ; } else { stationNow = 0 ; } 
  writePointer = 0; readPointer  = 0;
  emptyBufferCount = 10;
  stationSave();
  Serial.print(F("--> Will play: ")); Serial.println(stationList[stationNow].title);  
}



void switchPrev() {
  forceDisconnect();
  prevStationNow = stationNow;
  if (stationNow > 0) {stationNow-- ; } else { stationNow = stationCnt-1 ; } 
  writePointer = 0; readPointer  = 0;
  emptyBufferCount = 10;
  stationSave();
  Serial.print(F("--> Will play: ")); Serial.println(stationList[stationNow].title);
}


void stationSave() {
  EEPROM.begin(1);
  EEPROM.write(EEPROM_ADDR, stationNow);
  EEPROM.commit();
  Serial.print(F("--> Station ID written into EEPROM. It is #")); Serial.println(stationNow);
}


byte stationRead() {
  EEPROM.begin(1);
  stationNow = EEPROM.read(EEPROM_ADDR);
  Serial.print(F("--> Reading station ID from EEPROM. It is #")); Serial.println(stationNow);
  if ( (stationNow + 1) > stationCnt) { stationNow = 0; Serial.println(F("--> Station ID overflew ; setting to 0")); }
  prevStationNow = stationNow;
}


void welcomeMsg() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(F(" Internet Radio 1.0")); 
  lcd.setCursor(0,1); lcd.print(__DATE__); lcd.print(" "); lcd.print(__TIME__); 
  lcd.setCursor(0,3); lcd.print(F("> PLEASE STAND BY <"));
}

