//#include <SD.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>

#define BREAKOUT_CS     D8      // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    D3      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            10       // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ
#define BREAKOUT_RESET  D0      // VS1053 reset pin (output) --> XRET
#define CARDCS          -1      // SD Card command select pin (output)

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  
  pinMode(BREAKOUT_CS, OUTPUT);
  pinMode(BREAKOUT_DCS, OUTPUT);
  pinMode(DREQ, INPUT);
  pinMode(BREAKOUT_RESET, OUTPUT);
 
  SPI.begin();
  Serial.println(F("--> VS1053 GPIO test")); 

}

////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  Serial.println(F("--> VS1053 GPIO test")); 
  if (!musicPlayer.begin()) { Serial.println(F("--> ERROR: VS1053 not found.")) ; }
    else {
        Serial.println(F("--> BEEP")); 
        musicPlayer.softReset(); delay(150);
        musicPlayer.setVolume(0, 0);     // 0 = loudest
        musicPlayer.sineTest(0x44, 70);  // Make a tone to indicate VS1053 is working
    }

delay(1000);
}
