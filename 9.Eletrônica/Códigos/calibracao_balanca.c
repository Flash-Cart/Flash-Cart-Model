//codigo para definicao do fator de calibracao
#include "HX711.h"
 
#define CELULA_DOUT_PIN  5//definicao dos pino de saída
#define CELULA_SCK_PIN  2//definicao dos pino de clock

#define LED LED_VERMELHO//led vermelho
 
HX711 scale(CELULA_DOUT_PIN, CELULA_SCK_PIN);
 
float fator_calibracao = -7050; //fator de calibraçao inicial para celula de 50kg
 
void setup() {
  // inicializar pino digital como saida
  pinMode(LED, OUTPUT);    
  digitalWrite(LED, HIGH);   //ligar o LED(nível de tensao HIGHT )
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Retire todo o peso da balanca");
  Serial.println("Apos iniciar a leitura, insira peso padronizado conhecido"); 
  Serial.println("Teclas a,s,d,f aumentam fator de calibracao em 10,100,1000,10000 respectivamente");
  Serial.println("Teclas z,x,c,v diminuem fator de calibracao em 10,100,1000,10000 respectivamente");
 
  scale.set_scale();
  scale.tare();  //Resetar balanca para 0
 
  long nivel_zero = scale.read_average(); //Leitura de base
  Serial.print("Nivel zero: "); 
  Serial.println(nivel_zero);
 
}
 
void loop() {
  scale.set_scale(fator_calibracao); //Ajuste para este fator de calibracao
 
  Serial.print("Lendo...: ");
  Serial.print(scale.get_units(), 3);
  Serial.print(" Kg"); 
  Serial.print(" fator de calibracao: ");
  Serial.print(fator_calibracao);
  Serial.println();
 
  if(Serial.available())
  {
    char temp = Serial.read();
 
    if(temp == '+' || temp == 'a')
      fator_calibracao += 10;
    else if(temp == '-' || temp == 'z')
      fator_calibracao -= 10;
    else if(temp == 's')
      fator_calibracao += 100;
    else if(temp == 'x')
      fator_calibracao -= 100;
    else if(temp == 'd')
      fator_calibracao += 1000;
    else if(temp == 'c')
      fator_calibracao -= 1000;
    else if(temp == 'f')
      fator_calibracao += 10000;
    else if(temp == 'v')
      fator_calibracao -= 10000;
    else if(temp == 't')
      scale.tare(); //Resete a balanca para 0
  }  
}