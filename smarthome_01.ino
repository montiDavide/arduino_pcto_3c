/*
DHT11

This example reads a DHT11 sensor hooked up on pin D7. Reads both
temperature and humidity and sends it to the Serial port

created in Feb 2019 by D. Cuartielles
based on work by F. Vanzati (2011) and M. Loglio (2013)

This example code is in the public domain.
*/

// include the EduIntro library
#include <EduIntro.h>

DHT11 dht11(D7);  // creating the object sensor on pin 'D7'

int temp_read;   // temperature C readings are integers
int umidita_read;   // humidity readings are integers
int fotoresist;

int const fotopin = A5;
int const LED_EXT = D4;

// config
int const LIM_LUCE = 80;

void setup()
{
  // initialize serial communications at 9600 bps
  Serial.begin(9600);
  pinMode(LED_EXT, OUTPUT);

}

void loop()
{
  dht11.update();
  
  temp_read = dht11.readCelsius();     	// Reading the temperature in Celsius degrees and store in the C variable
  umidita_read = dht11.readHumidity();     // Reading the humidity index
  fotoresist = analogRead(fotopin);

  if (fotoresist<LIM_LUCE){
    digitalWrite(LED_EXT, HIGH);   
  }else{
    digitalWrite(LED_EXT, LOW);
  }

  // Print the collected data in a row on the Serial Monitor
  Serial.print("luce: ");
  Serial.print(fotoresist);
  
  Serial.print("\n\nH: ");
  Serial.print(umidita_read);
  Serial.print("\tC: ");
  Serial.println(temp_read);

  delay(1000);                // Wait one second before get another temperature reading
}
