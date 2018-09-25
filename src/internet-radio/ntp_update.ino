
void updateTime() {
    Serial.println("--> Updating time...");
    lcd.setCursor(0,2); lcd.print(F("  Updating time... "));

                udp.begin(localPort);
                WiFi.hostByName(ntpServerName, timeServerIP);  //get a random server from the pool
                Serial.print(F("Starting UDP on local port: ")); Serial.println(udp.localPort());
                Serial.print(F("NTP from: ")) ;                  Serial.println(ntpServerName);
                sendNTPpacket(timeServerIP);                   // send an NTP packet to a time server
                delay(5000);                                   // wait to see if a reply is available
                
                int cb = udp.parsePacket();
                if (!cb) {
                  Serial.println("No packet received in 5 s; program will continue without time sync");
                }
                else {
                  Serial.print("Packet received, length=");
                  Serial.println(cb);
                  
                  udp.read(packetBuffer, NTP_PACKET_SIZE);    // We've received a packet, read the data from it
                                                              // read the packet into the buffer
                                                              //the timestamp starts at byte 40 of the received packet and is four bytes,
                                                              // or two words, long. First, esxtract the two words:
              
                  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
                  // combine the four bytes (two words) into a long integer; this is NTP time (seconds since Jan 1 1900):
                  unsigned long secsSince1900 = highWord << 16 | lowWord;
                  Serial.print("Seconds since Jan 1 1900 = " );
                  Serial.println(secsSince1900);
              
                  // now convert NTP time into everyday time:
                  Serial.print("Unix time = ");
                  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
                  const unsigned long seventyYears = 2208988800UL;
                  // subtract seventy years:
                  unsigned long epoch = secsSince1900 - seventyYears;
                  // print Unix time:
                  Serial.println(epoch);
              
                  setTime(epoch + 7200); // diferenta GMT - ora bucurestiului
                  
                  // print the hour, minute and second:
                  Serial.print("UTC time: ");       // UTC is the time at Greenwich Meridian (GMT)
                  Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
                  Serial.print(':');
                  if ( ((epoch % 3600) / 60) < 10 ) {
                    // In the first 10 minutes of each hour, we'll want a leading '0'
                    Serial.print('0');
                  }
                  Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
                  Serial.print(':');
                  if ( (epoch % 60) < 10 ) {
                    // In the first 10 seconds of each minute, we'll want a leading '0'
                    Serial.print('0');
                  }
                  Serial.println(epoch % 60); // print the second
                }

                Serial.print("LOCAL time: "); 
                if (hour()   <= 9) Serial.print("0"); Serial.print(hour());   Serial.print(":"); 
                if (minute() <= 9) Serial.print("0"); Serial.print(minute()); Serial.print(":"); 
                if (second() <= 9) Serial.print("0"); Serial.println(second());



// ********************* Calculate offset for Sunday ********************* 
byte DST;
int t_month = month();
int t_dayOfMonth = day();
int t_hour = hour();
int y = year();                          // DS3231 uses two digit year (required here)
int x = (y + y/4 + 2) % 7;               // remainder will identify which day of month is Sunday by subtracting x from the one or two week window.  First two weeks for March and first week for November

// *********** Test DST: BEGINS on 2nd Sunday of March @ 2:00 AM ********* 
if(t_month == 3 && t_dayOfMonth == (14 - x) && t_hour >= 2)
   {                                   
    DST = 1;                           // Daylight Savings Time is TRUE (add one hour)
    Serial.println(F(">>>DST ON"));
   }
if((t_month == 3 && t_dayOfMonth > (14 - x)) || t_month > 3)
   {
    DST = 1;
    Serial.println(F(">>>DST ON"));
   }
// ************* Test DST: ENDS on 1st Sunday of Nov @ 2:00 AM ************       
if(t_month == 11 && t_dayOfMonth == (7 - x) && t_hour >= 2)
   {
    DST = 0;                            // daylight savings time is FALSE (Standard time)
    Serial.println(F(">>>DST OFF"));
   }
if((t_month == 11 && t_dayOfMonth > (7 - x)) || t_month > 11 || t_month < 3)
   {
    DST = 0;
    Serial.println(F(">>>DST OFF"));
   }
if(DST == 1)                        // Test DST and add one hour if = 1 (TRUE) 
   {
    //hour = hour + 1;
    setTime(now() + 3600);
    Serial.println(F(">>>DST ADDED"));
   }





}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

