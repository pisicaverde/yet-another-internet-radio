const unsigned char chrDef[6][8] PROGMEM = {
{ 0,  0,  0,  0,  3, 15, 15, 31}, // colt 2
{ 0,  0,  0,  0, 24, 30, 30, 31}, // colt 1
{ 0,  0,  0,  0, 31, 31, 31, 31}, // plin jos
{31, 15, 15,  3,  0,  0,  0,  0}, // colt 3
{31, 30, 30, 24,  0,  0,  0,  0}, // colt 4
{31, 31, 31, 31,  0,  0,  0,  0}} // plin sus
;

const PROGMEM byte charSet[12][4][3] = {
   { {0,2,1}, {6,7,6}, {6,7,6}, {3,5,4}}, // 0 - 0
   { {2,1,7}, {7,6,7}, {7,6,7}, {5,5,5}}, // 1 - 1
   { {0,2,1}, {0,2,6}, {6,7,7}, {3,5,5}}, // 2 - 2
   { {0,2,1}, {7,2,6}, {7,7,6}, {3,5,4}}, // 3 - 3
   { {2,7,7}, {6,2,2}, {7,6,7}, {7,5,7}}, // 4 - 4
   { {2,2,2}, {6,2,1}, {7,7,6}, {5,5,4}}, // 5 - 5
   { {0,2,1}, {6,2,1}, {6,7,6}, {3,5,4}}, // 6 - 6
   { {2,2,2}, {7,2,6}, {7,6,7}, {7,5,7}}, // 7 - 7
   { {0,2,1}, {6,2,6}, {6,7,6}, {3,5,4}}, // 8 - 8
   { {0,2,1}, {6,7,6}, {3,5,6}, {7,7,5}}, // 9 - 9
   { {8,8,8}, {8,5,8}, {8,2,8}, {8,8,8}}, // 10 - : (caracterul se incadreaza tot in 3x4, dar 8 inseamna NOT CHANGED)
   { {8,8,8}, {8,7,8}, {8,7,8}, {8,8,8}}  // 11 - stergere :
};

void printAt(byte col, byte chr) {
byte tmpChr;

  for (int y = 0 ; y <= 3 ; y++) { 
   for (int x = 0 ; x <= 2 ; x++) {
       lcd.setCursor(x+col, y);
       tmpChr = PROGMEM_getAnything (&charSet[chr][y][x]); 
       if ((tmpChr != 8) && (tmpChr <= 5)) lcd.write(tmpChr);
         else {
             if (tmpChr == 6) lcd.write(0xFF);
             if (tmpChr == 7) lcd.print(" ");
         }
   }
  }
}


void lcdCreateChars() {

  // creare array semne pentru big clock
unsigned char tmp[8]; 
 for (int i = 0; i < 6; i++) {
  for (int j = 0; j < 8; j++) 
    { 
     tmp[j] = PROGMEM_getAnything (&chrDef[i][j]);
    }
 lcd.createChar(i, tmp);
 }

}


void showClockScr() {

  static char str[12];  
  sprintf(str, "%04ld:%02d:%02d", hour(), minute(), second()); // returneaza str in format HHHH:MM:SS - cu leading zeros
     
  // afisare ore si minute mari
  printAt(0, str[2] - '0'); printAt(4, str[3] - '0'); // artificiul cu -'0' este ca sa converteasca un caracter la un intreg
  printAt(10,str[5] - '0'); printAt(14,str[6] - '0'); 

  // afisare secunde mic
  lcd.setCursor(17,1); lcd.print(":  ");
  lcd.setCursor(18,1); lcd.print(str[8]); lcd.setCursor(19,1); lcd.print(str[9]); 

  // punctul clipitor
  if ( second() %2 ) { printAt(7,10);  } else { printAt(7,11); }

  // id-ul postului
  lcd.setCursor(18,3); lcd.print(stationNow+1); 
  if (stationNow + 1 <= 9) { lcd.setCursor(19,3); lcd.print(" "); } // stergem cifra unitatilor ramasa de la o afisare anterioara de 2 cifre
}
