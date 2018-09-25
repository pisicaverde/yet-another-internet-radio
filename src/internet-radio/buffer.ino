void bufferForceFill() { 
  
  if (PLAYERPAUSE == true ) return; 
  
  // functia este BLOCKING si incearca sa faca un refill complet la buffer
  // se ruleaza doar cand de 4 secunde nu s-a primit macar un VS BUFFER complet
   
  Serial.println(F("--> BEGIN Buffer Force Fill..."));
  byteCnt=0;
  streamConnect();        // verificam si daca cumva nu e conectat la stream
  emptyBufferCount = 0;   // resetam contorul 

  unsigned int startTime = millis();              // va fi necesara sa iesim daca intr-un timp rezonabil nu se umple bufferul (complet)
  unsigned int waitMaxTime = 2000 * ((DATA_BUFFER_SIZE * 8 )/ (metaBR.toInt() * 1000) ) ;  // asteptam umplerea completa cel mult dublu fata de timpul de golire normala a bufferului
  Serial.print(F("--> waitMaxTime = ")) ; Serial.print(waitMaxTime); Serial.println(F(" ms"));
  
  while (usedBuffer() < DATA_BUFFER_SIZE-1)       // cat timp mai e loc de bagat in buffer,
    { 
      proc1Byte();                                // citim un byte de la server
      if (second() %2 ) screenUpdate();           // updatam progress bar-ul la fiecare 1 s, dar nu foarte des, pentru redraw-ul ecranului DUREAZA!
      if (emptyBufferCount >= 30) { // nu a venit absolut nimic de 30 de ori, trecem la postul urmator  !!!!!!!!!!!!!!!!!!!!
                                    Serial.println(F("--> Buffer emtpy for 30 tries, switching to next station.")); 
                                    switchNext(); 
                                    return; }

      if (millis() > startTime + waitMaxTime) { // nu s-a reusit umplerea completa a bufferului, e clar ca ceva e aiurea, sarim la postul urmator
                                                Serial.println(F("--> Timeout while waiting for completing bufferForceFill, switching to next station.")); 
                                                switchNext();
                                                return; }
    } 
  
  Serial.print(F("\r\n--> Done. Used buffer = ")); Serial.println(usedBuffer());
  BUFFER_EMPTY = false;
  emptyBufferCount = 0; // daca cumva a mai fost modificata valoarea de o functie din timer
  Serial.println(F("--> END of BufferForceFill"));

}


////////////////////////////////////////////////////////////////////////////////////////////


void bufferFill() { 

  if (PLAYERPAUSE == true ) return; 
  
  // incearca sa umple bufferul, dar nu in mod exclusiv; 
  // functia asta trebuie sa se intample FOARTE repede; 
  // am pus 1000 de bytes empiric, sa nu intrerupa playbackul, chiar daca e un post cu frecventa mare
  
  for (int q = 0 ; q < 1000 ; q ++) { if  (usedBuffer() < DATA_BUFFER_SIZE-1 ) { proc1Byte(); }} // daca mai e loc in buffer, adaugam un byte
  
  if (usedBuffer() >= VS_BUFFER_SIZE) { BUFFER_EMPTY = false; } // daca a apucat sa bage cel putin un calup pentru VS
                                 else { BUFFER_EMPTY = true ; }
}


////////////////////////////////////////////////////////////////////////////////////////////


void proc1Byte() {   
  
  // the heart and soul of radio
  // functia citeste byte cu byte ce vine de la server si in functie de pozitia in stream, face ceva sau altceva cu byte-ul citit
  // facuta dupa informatiile de aici: http://www.smackfu.com/stuff/programming/shoutcast.html

  if (mp3client.available()) {  
        // daca suntem in zona de MP3 DATA si bufferul nu este full: citim un byte si il scriem in buffer; cand ajunge la capat, writePointer se reseteaza. Altfel ignoram
        if (( byteCnt < metaInt ) && (  !((writePointer + 1) % DATA_BUFFER_SIZE == readPointer)    ) ) { dataBufferWrite(mp3client.read());  }  
        
        // DETECT META MARKER: daca s-au citit metaInt bytes, urmatorul octet * 16 = cati bytes de meta info urmeaza
        if (byteCnt == metaInt) { metaLength = mp3client.read() * 16; 
                            if (metaLength != 0) {Serial.print(F("\r\n--> ")); Serial.print(metaLength); Serial.print(F(" Bytes of metadata: ")); } }

        // IN META ZONE: preluam meta info, nu bagam nimic in buffer
        if ( (byteCnt > metaInt) && (byteCnt <= (metaInt + metaLength ) ) ) { 
                                                                  char tmpChr = mp3client.read() ; Serial.write(tmpChr); 
                                                                  metaDataTxt_tmp += tmpChr; 
                                                                  if (metaDataTxt_tmp.length() >= metaLength) {  // s-a obtinut un metaData complet, parsam un pic stringul
                                                                          int metaDataStart = metaDataTxt_tmp.indexOf("='") + 2 ; int metaDataEnd = metaDataTxt_tmp.indexOf("';");
                                                                          metaDataTxt = "      " + metaDataTxt_tmp.substring(metaDataStart, metaDataEnd) + "      ";
                                                                          txtScroll = 0 ; txtDir = 1; metaDataTxt_tmp = ""; // resetam variabilele pt scroll, ca altfel dispare stringul
                                                                          }
                                                                 }

        // SFARSIT- s-a terminat un frame, s-a terminat de afisat meta info, bagam un enter
        if ( byteCnt >= (metaInt + metaLength + 1) ) { byteCnt = 0; } 
                                                else { byteCnt++;   }
  } 
      //else Serial.print("/"); // aici putem pune ce se intampla daca nu primim date
}


////////////////////////////////////////////////////////////////////////////////////////////


void dataBufferWrite(byte x) {     // scrie valoarea x in stiva (bufferul circular)
  ramPoke(writePointer, x);
  writePointer = (writePointer + 1) % DATA_BUFFER_SIZE;  // se incrementeaza pozitia cu 1 sau sare pe pozitia 0 daca era la capat
  if (writePointer == readPointer) { readPointer = (readPointer + 1) % DATA_BUFFER_SIZE; }  // daca scriem mai repede decat citim, atunci trebuie mutat cu 1 pozitie si readPointer
}



byte dataBufferRead() {   // citeste valoarea din stiva
  byte x = ramPeek(readPointer);
  readPointer = (readPointer + 1) % DATA_BUFFER_SIZE;
  return(x);
}



int usedBuffer() {        // returneaza cati bytes sunt folositi in data buffer
  return(( DATA_BUFFER_SIZE + writePointer - readPointer ) % DATA_BUFFER_SIZE);
}


