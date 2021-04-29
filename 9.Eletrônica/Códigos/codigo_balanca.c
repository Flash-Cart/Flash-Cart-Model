#include <SPI.h>
#include <OneMsTaskTimer.h>
#include <LCD_SharpBoosterPack_SPI.h>
#include "HX711.h"
 
 
#define fator_calibracao -323400.0                   

#define CELULA_DOUT_PIN  5//definicao dos pino de saída
#define CELULA_SCK_PIN  2//definicao dos pino de clock

#define LED LED_VERMELHO//led vermelho

HX711 scale(CELULA_DOUT_PIN, CELULA_SCK_PIN);
 
// Variables
LCD_SharpBoosterPack_SPI myScreen;
uint8_t myOrientation = 0;
uint16_t myCount = 0;
 
void setup() {
 
  // inicializar o pino digital como saida
  pinMode(LED, OUTPUT);    
  digitalWrite(LED, HIGH);   
  Serial.begin(9600);
  
  Serial.println("Balanca HX711");
 
  scale.set_scale(fator_calibracao); 
  scale.tare();  //assumindo que nao ha peso no icicio, inicie em 0
 
  Serial.println("Leitura:");
}
 
void loop() {
  Serial.print("Leitura: ");
  Serial.print(scale.get_units(), 3); 
  Serial.print(" Kg"); 
  Serial.println();
 
}