/*
https://www.futurashop.it/image/catalog/data/Download/LiquidCrystal_I2C1602V1.zip 
da scaricare!
*/

// include the libraries
#include <EduIntro.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

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
int const LIM_LUCE = 80;
// config Dist. Garage
float const DISTANZA_NORM = 20 ;
int const DISTANZA_VARIANZ = 25; // % della norm.
int const tempo_garage= 5; // numero cicli

struct {
  int OPEN = 180;
  int CLOSE = 0;
}GARAGE_STATE;

bool garage = false;

float getDist_cm();
bool ApriGarage(float dist);
void openGarage();
void closeGarage();
void manageGarage(float dist_cm);

void setup()
{
  // initialize serial communications at 9600 bps
  Serial.begin(9600);
  pinMode(LED_EXT, OUTPUT);
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  Garage.attach(MOTORE_1);

  // robo x distanza  
  pinMode(TRIG_DIST, OUTPUT);
  pinMode(ECHO_DIST, INPUT);
  garagedelay = tempo_garage;
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



  // enable outside led
  if (fotoresist<LIM_LUCE){
    digitalWrite(LED_EXT, HIGH);   
  }else{
    digitalWrite(LED_EXT, LOW);
  }
  // scrivi a schermo
  snprintf(buffer[0], 16,"C %03i, H:%02i \0", temp_read, umidita_read); 
  snprintf(buffer[1], 16,"Luce : %05i \0", fotoresist); 
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
