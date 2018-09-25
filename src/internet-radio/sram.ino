//Byte transfer functions
uint8_t Spi23LC1024ReadR() {
  uint8_t read_byte;
  //  PORTB &= ~(1<<PORTB2);        //set SPI_SS low
  digitalWrite(SRAM_CS, LOW);
  SPI.transfer(RDSR);
  read_byte = SPI.transfer(0x00);
  //  PORTB |= (1<<PORTB2);         //set SPI_SS high
  digitalWrite(SRAM_CS, HIGH);
  return read_byte;
}

byte ramPeek(unsigned int address) {
  byte read_byte;
  digitalWrite(SRAM_CS, LOW); //set SPI_SS low
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16) & 0xff);
  SPI.transfer((uint8_t)(address >> 8) & 0xff);
  SPI.transfer((uint8_t)address);
  read_byte = SPI.transfer(0x00);
  digitalWrite(SRAM_CS, HIGH); //set SPI_SS high
  return read_byte;
}

void ramPoke(uint32_t address, uint8_t data_byte) {
  //  PORTB &= ~(1<<PORTB2);        //set SPI_SS low
  digitalWrite(SRAM_CS, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(address >> 16) & 0xff);
  SPI.transfer((uint8_t)(address >> 8) & 0xff);
  SPI.transfer((uint8_t)address);
  SPI.transfer(data_byte);
  //  PORTB |= (1<<PORTB2);         //set SPI_SS high
  digitalWrite(SRAM_CS, HIGH);
}





void sram_setup(void) {

  lcd.setCursor(0,2); lcd.print(F("SRAM memory test..."));
  
  uint8_t RDMR;
  digitalWrite(SRAM_CS, HIGH);
  Serial.println("--> SRAM SETUP");
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(1000000);

  RDMR = Spi23LC1024ReadR();
  Serial.print(F("--> Read Mode Register: "));
  print_binary(RDMR, 8);
  Serial.println("");
  switch (RDMR) {
    case 0:      Serial.println(F("  RDMR Byte mode."));      break;
    case 1:      Serial.println(F("  RDMR Sequential mode."));      break;
    case 2:      Serial.println(F("  RDMR Page mode."));      break;
    case 3:      Serial.println(F("  Reserved"));      break;
  }
    Serial.println(F("--> SRAM SETUP ENDED"));

// SRAM TEST incepe aici; e aiurea pentru ca un test complet dureaza enorm
  Serial.println(F("--> START MEMTEST"));
  
   for (unsigned int address = 0; address < DATA_BUFFER_SIZE; address=address+100) {
      if ((address % 1024) == 0) Serial.println(address);
      byte value = random(255);
      ramPoke(address, value); 
      delay(1);
      if (ramPeek(address) != value) { Serial.print("--> MEMORY ERROR AT "); Serial.println(address);} else { Serial.print("--> OK AT "); Serial.println(address);} 
      yield;
   }

}






void print_binary(int v, int num_places)
{
  int mask = 0, n;
  for (n = 1; n <= num_places; n++)   { mask = (mask << 1) | 0x0001;  }
  v = v & mask;  // truncate v to specified number of places

  while (num_places)
  {
    if (v & (0x0001 << num_places - 1)) { Serial.print("1");  }
                                   else { Serial.print("0"); }
    --num_places;
    if (((num_places % 4) == 0) && (num_places != 0)) { Serial.print("_"); }
  }
}
