#include <SPI.h>
#define SRAM_CS D4 // active LOW
#define VS_CS   D8 // active LOW

//SRAM opcodes
#define READ    0x03
#define WRITE   0x02

void POKE(uint32_t address, byte data_byte) {
  digitalWrite(SRAM_CS, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(address >> 16) & 0xff);
  SPI.transfer((uint8_t)(address >> 8) & 0xff);
  SPI.transfer((uint8_t)address);
  SPI.transfer(data_byte);
  digitalWrite(SRAM_CS, HIGH);
}

byte PEEK(uint32_t address) {
  byte read_byte;
  digitalWrite(SRAM_CS, LOW);
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16) & 0xff);
  SPI.transfer((uint8_t)(address >> 8) & 0xff);
  SPI.transfer((uint8_t)address);
  read_byte = SPI.transfer(0x00);
  digitalWrite(SRAM_CS, HIGH);
  return read_byte;
}

void setup(void) {

  pinMode(SRAM_CS, OUTPUT); digitalWrite(SRAM_CS, LOW); delay(100); digitalWrite(SRAM_CS, HIGH); delay(100);
  pinMode(VS_CS, OUTPUT);   digitalWrite(VS_CS, HIGH); delay(100) ; //ar trebui sa dezactiveze VS-ul

  Serial.begin(115200); delay(100); 
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(1000000);

//g,v,a

byte j = 0;
  for (byte i = 0; i < 16; i++) {
    POKE(i, i); 
    byte value = PEEK(i);
    Serial.print(i);  Serial.print(" : "); Serial.print(value, DEC); Serial.print(" ( ") ; Serial.print(j) ; Serial.println(" )");
    j++; 
    delay(1);
  }
}

void loop() {}















void print_binary(int v, int num_places)
{
  int mask = 0, n;

  for (n = 1; n <= num_places; n++)
  {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask;  // truncate v to specified number of places

  while (num_places)
  {

    if (v & (0x0001 << num_places - 1))
    {
      Serial.print("1");
    }
    else
    {
      Serial.print("0");
    }

    --num_places;
    if (((num_places % 4) == 0) && (num_places != 0))
    {
      Serial.print("_");
    }
  }
}
