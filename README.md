# yet-another-internet-radio

Another semi-decent internet radio, based on VS1053 decoder and ESP8266 MCU, made from scratch solely for my midnight engineering sessions.

It plays anything that VS1053 may decode.

See it live here:

[![yet-another-internet-radio](http://img.youtube.com/vi/FiWzWgzm_9E/0.jpg)](http://www.youtube.com/watch?v=FiWzWgzm_9E "yet-another-internet-radio")

## Features:
* a streaming media player built with **ESP8266** MCU, **VS1053** for mp3 decoding and **1 Mbit buffer**;
* time update from `pool.ntp.org` (no RTC);
* current time shown with big fonts during stand-by;
* clock, stream title, current song, bitrate, wifi level, buffer level shown during playback
* up to 196kbps bitrate streams;
* configurable via wifi using WiFiManager by Tzapu;
* current radio station saved into EEPROM;
* auto-reconnect to stream if connection is lost
* voice feedback (or any other mp3 played at certain moments)

The station list is stored into a remote hosted JSON file: [irconfig.dat](/irconfig.dat).

Please note there are some Romanian comments left into the source. I'll try to clean it up.

Where it all started: [Arduino WebPlayer by Vassilis Serasidis](https://www.serasidis.gr/circuits/Arduino_WebRadio_player/Arduino_WebRadio_player.htm) done with an Arduino Pro Mini.

## Schematics
Could not be easier:
![Yet Another Internet Radio schematics](https://raw.githubusercontent.com/pisicaverde/yet-another-internet-radio/master/images/schema1.jpg)
Everything works at 3.3V, except the 20x4 lcd (5V power & data) that needs the level shifter.

#### Bill of materials:
- [WeMos NodeMcu with ESP8266](https://www.banggood.com/Wemos-Nodemcu-Wifi-For-Arduino-And-NodeMCU-ESP8266-0_96-Inch-OLED-Board-p-1154759.html): $10 ; you will need access to all ESP8216 pins. The LCD is very small and not really useful.
- [VS1053 breakout](https://www.dx.com/p/sd-card-mp3-music-shield-audio-expansion-board-white-171917): $15 - [it was buggy](https://blog.devmobile.co.nz/2014/04/27/netduino-and-freaduino-mp3-music-shield/);
- [23LC1024 breakout](https://www.mikroe.com/sram-click): $15 - unexplainable expensive since you may find a $2 [SO8 package](https://www.tme.eu/en/details/23lc1024-i_sn/serial-sram-memories-integrated-circ/microchip-technology/)
- [2x3W PAM8403 amplifier](https://www.banggood.com/PAM8403-2-Channel-USB-Power-Audio-Amplifier-Module-Board-3Wx2-Volume-Control-p-1068215.html?cur_warehouse=CN): $1.5 - noisy, not recommended;
- [12V power source](https://www.banggood.com/Mini-100W-Switching-Power-Supply-180-240V-To-12V-8_3A-For-LED-Strip-Light-p-985455.html): $14
- [step down converter](https://www.banggood.com/10Pcs-LM2596-DC-DC-Adjustable-Step-Down-Power-Supply-Module-p-963307.html): $1
- [20 x 4 alpha lcd with i2c expander](https://www.dx.com/p/arduino-iic-i2c-serial-3-2-lcd-2004-module-display-138611) - $10
- level shifters and switches (less than $5)

So practically the player is made-in-China.

Note about the VS1053 breakout: *The mp3 shield was intended to work with Arduino Uno (5V). Wiring it directly to ESP8266 (3.3V) could damage it. The mp3 shield did not work properly via a level shifter, due to faster data transfers, so it had to be modified to work directly at 3V.*

## Software stuff
There are many approaches on internet, but none worked for me, so I had to build it from scratch.
* [minimum working program](/src/minimum): a media player with no interface. The wifi user/pass and station are hardcoded. Just to understand the working principle.
* [full app](/src/current) -- big and not really convenient to understand
* some [tools](/src/tools) you may need for hardware check

### Bitrate
It plays fine up to 196 kbps streams; the screen update is blocking the MCU for a few ms, so it won't play higher bitrate streams. With all lcd and serial commented out, it may play up to 320 kbps streams.

### Config mode:
The web radio uses WifiManager by Tzapu'. In case it cannot connect to the last known hotspot, it enters config mode. You can force entering config mode by holding Button1 during startup.

The station list is held into a JSON file, stored remote: [irconfig.dat](/irconfig.dat). The config file URL is entered in config mode. If you're using cPanel to host the configuration file, it should not be named with .json extension, since cPanel is caching it weirdly.

Do not leave `Enter/New line` chars in your json, or ArduinoJson library may crash.

## Literature:
- **[How to parse MP3 Streams](http://www.smackfu.com/stuff/programming/shoutcast.html)**
- [the VS1053 board I was using](http://www.elecfreaks.com/wiki/index.php?title=Freaduino_MP3_Music_Shield)
- [Adafruit VS1053 library](https://github.com/adafruit/Adafruit_VS1053_Library)
- [VS1053 datasheet](https://www.sparkfun.com/datasheets/Components/SMD/vs1053.pdf)
- [What is the circular buffer](https://en.m.wikipedia.org/wiki/Circular_buffer)
- [working with 23LC1024 from ESP8266](https://github.com/paulenuta/ESP_23LC1024)
- [23LC1024 datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/20005142C.pdf)
- [how to use PROGMEM](http://www.gammon.com.au/progmem)
- [one AnalogIn, multiple buttons](https://www.baldengineer.com/5-voltage-divider-circuits.html)
- [computing multiple voltage dividers](http://www.loosweb.de/calculator/en/muteiler.html)
- [how to make SD class coexist with SPIFFS](https://github.com/esp8266/Arduino/issues/1524#issuecomment-253969821) and (https://github.com/esp8266/Arduino/issues/2281#issuecomment-258706478)
- [custom character generator](https://omerk.github.io/lcdchargen/)
- [online json editor](https://jsoneditoronline.org/)


## To do & issues
- [ ] to fix some ESP8266 crashes after connection to stream
- [ ] add bass/treble control by VS1053
- [ ] add MIDI playback
- [ ] add mp3 playback from LAN
