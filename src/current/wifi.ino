
void wifiCustConnect() {

  Serial.println(F("--> WiFi manager custom connect"));
  lcd.setCursor(0,2); lcd.print(F(" Connecting to WiFi "));
  
  // id/name, placeholder/prompt, default, length
  WiFiManagerParameter custom_text("<p>Station list json URL, without http, default: pisicaverde.ro/irconfig.json"); wifiManager.addParameter(&custom_text);
  WiFiManagerParameter p_jsonURL("jsonURL", "enter here", "", 100);  wifiManager.addParameter(&p_jsonURL);
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setDebugOutput(true);
  
  //fetches ssid and pass and tries to connect ; if it does not connect it starts an access point with the specified name, here  "AutoConnectAP", and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("InternetRadioConfig")) {
        Serial.println(F("--> failed to connect and hit timeout; restarting"));
        ESP.restart(); //reset and try again, or maybe put it to deep sleep
    }

  Serial.println(F("--> CONNECTED...yeey :)")); //if you get here you have connected to the WiFi


  if (shouldSaveConfig) {
        SPIFFS.begin();
        Serial.println(F("--> SAVING CONFIG..."));
        strcpy(jsonURL, p_jsonURL.getValue());
  
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        json["jsonURL"] = jsonURL;

        fs::File f = SPIFFS.open("/config.json", "w"); //accessing SPIFFS files, add 'fs::' in front of the file declaration, the rest is done as usual:
        if (!f) { Serial.println(F("--> ERROR: failed to open config file for writing")); }

        json.printTo(Serial);
        json.printTo(f);
        f.close();

        SPIFFS.end();
        Serial.println(F("--> SAVED"));
        shouldSaveConfig = false;
        
    }


  
}






void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(F("---> WiFi Error <---")); 
  lcd.setCursor(0,1); lcd.print(F("Connect to AP")); 
  lcd.setCursor(0,2); lcd.print(myWiFiManager->getConfigPortalSSID()) ;
  lcd.setCursor(0,3); lcd.print("and open " + String(IPAddress(WiFi.softAPIP()[0])) + "." + String(IPAddress(WiFi.softAPIP()[1])) + "." + String(IPAddress(WiFi.softAPIP()[2])) + "." + String(IPAddress(WiFi.softAPIP()[3])) );

}





//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println(F(">>> Should save config"));
  lcd.clear(); lcd.setCursor(0,0); lcd.print(F("Saved.")); 
  shouldSaveConfig = true;
}

