/*
https://www.futurashop.it/image/catalog/data/Download/LiquidCrystal_I2C1602V1.zip 
da scaricare!
*/

// include the libraries
#include <EduIntro.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

DHT11 dht11(D7);  // creating the object sensor on pin 'D7'
LiquidCrystal_I2C lcd(0x27,16,2); // 0x3F = 63 // usa 0x27 se non va

int temp_read;   // temperature C readings are integers
int umidita_read;   // humidity readings are integers
int fotoresist;

int const fotopin = A0;
int const LED_EXT = D4;

// config
int const LIM_LUCE = 80;

void setup()
{
  // initialize serial communications at 9600 bps
  Serial.begin(9600);
  pinMode(LED_EXT, OUTPUT);
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  //lcd.print("si sta accendendo");
}

void loop()
{
  char buffer[2][17] = {"",""};
  
  dht11.update();
  
  temp_read = dht11.readCelsius();     	// Reading the temperature in Celsius degrees and store in the C variable
  umidita_read = dht11.readHumidity();     // Reading the humidity index
  fotoresist = analogRead(fotopin);

  if (fotoresist<LIM_LUCE){
    digitalWrite(LED_EXT, HIGH);   
  }else{
    digitalWrite(LED_EXT, LOW);
  }
  snprintf(buffer[0], 16,"%03i C, H:%02i% \0", temp_read, umidita_read); 
  snprintf(buffer[1], 16,"Luce : %05i \0", fotoresist); 
  lcd.setCursor(0, 0);
  lcd.print( buffer[0] ); 
  lcd.setCursor(0, 1);
  lcd.print( buffer[1] );
  
  // Print the collected data in a row on the Serial Monitor
  Serial.print("luce: ");
  Serial.print(fotoresist);
  
  Serial.print("\n\nH: ");
  Serial.print(umidita_read);
  Serial.print("\tC: ");
  Serial.println(temp_read);

  delay(1000);                // Wait one second before get another temperature reading
}
