/*
https://www.futurashop.it/image/catalog/data/Download/LiquidCrystal_I2C1602V1.zip 
da scaricare!
*/

// include the libraries
#include <EduIntro.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

DHT11 dht11(D7);                    // creating the object sensor on pin 'D7'
LiquidCrystal_I2C lcd(0x27,16,2);   // 0x3F = 63 // usa 0x27 se non va
Servo Garage;

int const fotopin = A0;
int const LED_EXT = D4;
int const MOTORE_1 = D3;
int const TASTO_GARAGE = A1;
int const TRIG_DIST = D8;
int const ECHO_DIST = D9;

int temp_read;   // temperature C readings are integers
int umidita_read;   // humidity readings are integers
int fotoresist;
int garagedelay;
float dist_cm;

// config luce
int const LIM_LUCE = 30;
// config Dist. Garage
float const DISTANZA_NORM = 20 ;
int const DISTANZA_VARIANZ = 25; // % della norm.
int const tempo_garage= 5; // numero cicli

struct {
  int OPEN = 180;
  int CLOSE = 0;
}GARAGE_STATE;

bool garage = false;
// caratteri x icona luce ext
uint8_t luce_asx[8]={0b00000,0b00000,0b00000,0b00000,0b00001,0b00010,0b00100};
uint8_t luce_adx[8]={0b00001,0b01010,0b00100,0b00000,0b10000,0b01000,0b00100};
uint8_t luce_bsx[8]={0b00100,0b00110,0b00010,0b00010,0b00001,0b00011,0b00001};
uint8_t luce_bdx[8]={0b00100,0b01100,0b01000,0b01000,0b10000,0b11000,0b10000};

uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};

float getDist_cm();
bool ApriGarage(float dist);
void openGarage();
void closeGarage();
void manageGarage(float dist_cm);
void stampa_lapada_lcd();

void setup()
{
  // initialize serial communications at 9600 bps
  Serial.begin(9600);
  pinMode(LED_EXT, OUTPUT);
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  lcd.home();

  Garage.attach(MOTORE_1);

  // robo x distanza  
  pinMode(TRIG_DIST, OUTPUT);
  pinMode(ECHO_DIST, INPUT);
  garagedelay = tempo_garage;

  lcd.createChar(0, luce_asx);
  lcd.createChar(1, luce_adx);
  lcd.createChar(2, luce_bsx);
  lcd.createChar(3, luce_bdx);

  lcd.createChar(4,check);

}

void loop()
{
  char buffer[2][17] = {"",""};
  
  dht11.update();
  
  temp_read = dht11.readCelsius();     	// Reading the temperature in Celsius degrees and store in the C variable
  umidita_read = dht11.readHumidity();     // Reading the humidity index
  fotoresist = analogRead(fotopin);

  dist_cm = getDist_cm();

  // Print the collected data in a row on the Serial Monitor
  Serial.print("luce: ");
  Serial.print(fotoresist);
  
  Serial.print("\n\nH: ");
  Serial.print(umidita_read);
  Serial.print("\tC: ");
  Serial.println(temp_read);

  Serial.println(dist_cm);
  
  //Serial.print("button:");
  //Serial.println(analogRead(TASTO_GARAGE));


  // prepara buffer
  snprintf(buffer[0], 16,"C %03i, H:%02i \0", temp_read, umidita_read); 
  snprintf(buffer[1], 16,"Luce : %05i \0", fotoresist);
  
  //svuota schermo
  lcd.clear();
  
  // enable outside led
  if (fotoresist<LIM_LUCE){
    digitalWrite(LED_EXT, HIGH);
    stampa_lapada_lcd();
    
  }else{
    digitalWrite(LED_EXT, LOW);
    
  }
  // scrivi a schermo
  
  lcd.setCursor(0, 0);
  lcd.print( buffer[0] ); 
  lcd.setCursor(0, 1);
  lcd.print( buffer[1] );
  
  manageGarage(dist_cm);

  delay(1000);                // Wait 1 second before looping
}

float getDist_cm(){

  // non toccare sti delay
  digitalWrite(TRIG_DIST, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_DIST, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_DIST, LOW);  

  return ( pulseIn(ECHO_DIST, HIGH) /2) / 29.1; // in cm
}

bool SensoreGarage(float dist){  
  if (dist < DISTANZA_NORM*(100-DISTANZA_VARIANZ)/100 || dist > DISTANZA_NORM*(100+DISTANZA_VARIANZ)/100 ){         
    return true;
  }
  return false;
}

void openGarage() {
  for(int i=0;i<GARAGE_STATE.OPEN;i++){
      Garage.write(i);
      delay(25);         
    }
    garage=true; 
}
void closeGarage(){
  for(int i=180;i>GARAGE_STATE.CLOSE;i--){
      Garage.write(i);
      delay(20);         
    }
    garage=false;
}
// apri/chiudi garage
void manageGarage(float dist_cm){
  
  if (garagedelay<tempo_garage){
    garagedelay++;
  }
  /*
  if ( analogRead(TASTO_GARAGE) > 300 ) {
      if ( !garage ) //{Garage.write(GARAGE_STATE.OPEN); garage=true;}
      {
        openGarage();
      }
      else{
          closeGarage();   
      }
  } */

  if ( analogRead(TASTO_GARAGE) > 300 ){
    return;
  }

  if ( SensoreGarage(dist_cm) ){ 
    if (!garage){
      openGarage();
      garagedelay=0; 
    }
  }else{
    if( garagedelay==tempo_garage && garage){
        closeGarage();      
    }
  }

}

// icona_luce
void stampa_lapada_lcd(){
  lcd.setCursor(14, 0);
  lcd.printByte(0);

  lcd.setCursor(15, 0);
  lcd.printByte(1);

  lcd.setCursor(14, 1);
  lcd.printByte(2);

  lcd.setCursor(15, 1);
  lcd.printByte(3);
}
