// includi le librerie
#include <EduIntro.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// come suggerito nella libreria del'LCD, definisce funzione piu comoda per caratteri speciali
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

DHT11 dht11(D7);                    // rea oggetto sensor su pin 'D7'
LiquidCrystal_I2C lcd(0x27,16,2);   // crea oggetto lcd su indirizzo 0x27 del protocollo I2C
Servo Garage;

// costanti con pin usati
int const fotopin = A0;
int const LED_EXT = D4;
int const MOTORE_1 = D3;
int const TASTO_GARAGE = A1;
int const TRIG_DIST = D8;
int const ECHO_DIST = D9;

// dichiarazioni variabili globali usate
int temp_read;   // temperature C, int
int umidita_read;   // humidity, integers, % humidity
int fotoresist;
int garagedelay;
float dist_cm; // distanza non e' intera

// configurazione luce
int const LIM_LUCE = 30;
// config Dist. Garage
float const DISTANZA_NORM = 20 ;
int const DISTANZA_VARIANZ = 25; // % della norm.
int const tempo_garage= 5; // numero cicli d'attesa

// struct per gradi di cui muovere il garage
struct {
  int OPEN = 180;
  int CLOSE = 0;
}GARAGE_STATE;

bool garage = false; // se aperto

// caratteri x icona luce ext
uint8_t luce_asx[8]={0b00000,0b00000,0b00000,0b00000,0b00001,0b00010,0b00100};
uint8_t luce_adx[8]={0b00001,0b01010,0b00100,0b00000,0b10000,0b01000,0b00100};
uint8_t luce_bsx[8]={0b00100,0b00110,0b00010,0b00010,0b00001,0b00011,0b00001};
uint8_t luce_bdx[8]={0b00100,0b01100,0b01000,0b01000,0b10000,0b11000,0b10000};

// prototipi funzioni usate
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
  pinMode(LED_EXT, OUTPUT); // pin luce esterna assegnato
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.home();

  Garage.attach(MOTORE_1);        // assegna a motore il suo pin

  // robo x distanza  
  pinMode(TRIG_DIST, OUTPUT);
  pinMode(ECHO_DIST, INPUT);
  garagedelay = tempo_garage;

  lcd.createChar(0, luce_asx); // crea 4 caratteri per componenti icona luce
  lcd.createChar(1, luce_adx);
  lcd.createChar(2, luce_bsx);
  lcd.createChar(3, luce_bdx);

}

void loop()
{
  // crea due stringe da buffer per lo schermo
  char buffer[2][17] = {"",""};
  
  dht11.update(); // aggiorna sensore
  
  temp_read = dht11.readCelsius();     	// Reading the temperature in Celsius degrees and store in the C variable
  umidita_read = dht11.readHumidity();     // Reading the humidity index
  fotoresist = analogRead(fotopin);     // leggi dati da fotoresistenza

  dist_cm = getDist_cm();               // misura distanza in cm

  // Print the collected data in a row on the Serial Monitor
  Serial.print("luce: ");
  Serial.print(fotoresist);
  
  Serial.print("\n\nH: ");
  Serial.print(umidita_read);
  Serial.print("\tC: ");
  Serial.println(temp_read);

  Serial.println(dist_cm);
  
  Serial.print("button:");
  Serial.println(analogRead(TASTO_GARAGE));


  // prepara buffer
  snprintf(buffer[0], 16,"C %03i, H:%02i \0", temp_read, umidita_read); 
  snprintf(buffer[1], 16,"Luce : %03i \0", fotoresist);
  
  //svuota schermo
  lcd.clear();
  
  // enable outside led and write lamp icon if needed
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
  
  // chiama funzione gestrice del garage
  manageGarage(dist_cm);

  delay(1000);                // Wait 1 second before looping
}

float getDist_cm(){

  // non toccare sti delay per ultrasuoni
  digitalWrite(TRIG_DIST, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_DIST, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_DIST, LOW);  

  return ( pulseIn(ECHO_DIST, HIGH) /2) / 29.1; // converti in cm
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
    Serial.println("open");
}
void closeGarage(){
  for(int i=180;i>GARAGE_STATE.CLOSE;i--){
      Garage.write(i);
      delay(25);         
    }
    garage=false;
    Serial.println("close");
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
      
    if (garage){
      closeGarage();
    }

    return;
  }

  if ( SensoreGarage(dist_cm) ){ 
    if (!garage){
      openGarage();
      garagedelay=0; 
    }
  }else{
    if( garagedelay>=tempo_garage && garage){
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
