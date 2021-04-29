#include <Wire.h>  
  
// Classe simples para tratar a bússola  
class Bussola {  
  public:  
    typedef enum { INDEFINIDO, HMC, QMC } TIPO;  
    Bussola(TIPO tipo);  
    bool inicia(void);  
    TIPO getTipo(void);  
    void setDeclination (int graus , int mins, char dir);  
    float leDirecao(void);  
    void iniciaCal();  
    void encerraCal();  
  
  private:  
    static const int ender_HMC = 0x1E; // endereço I2C do HMC5883  
    static const int regMODE_HMC = 2;  // registrador de modo  
    static const int regXH_HMC = 3;    // primeiro registrador de dados  
    static const int regST_HMC = 9;    // registrador de status  
      
    static const int ender_QMC = 0x0D; // endereço I2C do QMC5883  
    static const int regCR1_QMC = 9;   // registrador de configuração  
    static const int regSR_QMC = 11;   // registador set/reset  
    static const int regXL_QMC = 0;    // primeiro registrador de dados  
    static const int regST_QMC = 6;    // registrador de status  
  
    // fatores de correção determinados na calibração  
    int16_t xMin, yMin, xMax, yMax;  
    float escX = 1.0;  
    float escY = 1.0;  
    int16_t offX = 0;  
    int16_t offY = 0;  
  
    // Edereço e tipo do chip  
    int ender;  
    TIPO tipo;  
  
    // Diferença entre o Polo Magnético e o Geográfico  
    float declination = 0.0;  
  
    // Rotina para disparar leitura dos dados  
    void pedeDados(int regStatus, int regDados);  
};  
  
Bussola bussola(Bussola::INDEFINIDO);  
  
// Iniciação do programa  
void setup(){  
    
  Wire.begin();  
  Serial.begin(115200);  
  if (!bussola.inicia()) {  
    Serial.println ("Nao encontrou a bussola!");  
    for (;;) {  
      delay(100);  
    }  
  }  
  Serial.print ("Achou bussola ");  
  Serial.println (bussola.getTipo() == Bussola::QMC ?  
      "QMC5883L" : "HMC5883L");  
  
  Serial.println ("Calibrando... rode o sensor em um círculo");  
  bussola.iniciaCal();  
  long tmpFim = millis()+60000L;  
  while (millis() < tmpFim) {  
    bussola.leDirecao();  
    delay(20);  
  }  
  bussola.encerraCal();  
  Serial.println ("Calibrado");  
}  
  
// Laço principal  
void loop(){  
  float direcao = bussola.leDirecao();  
  Serial.println (direcao);  
  delay(1000);  
}  
  
// Construtor  
Bussola::Bussola(TIPO tipo) {  
  this->tipo = tipo;  
}  
  
// Inicia comunicação com a bússola  
bool Bussola::inicia() {  
  if (tipo == INDEFINIDO) {  
    // Tenta identificar o chip  
    Wire.beginTransmission(ender_HMC);  
    if (Wire.endTransmission() == 0) {  
      tipo = HMC;  
      ender = ender_HMC;  
    } else {  
      Wire.beginTransmission(ender_QMC);  
      if (Wire.endTransmission() == 0) {  
        tipo = QMC;  
        ender = ender_QMC;  
      }  
    }  
  }  
  
  // Inicia o chip para modo contínuo  
  if (tipo == HMC) {  
    Wire.beginTransmission(ender);  
    Wire.write(regMODE_HMC);  
    Wire.write(0x00);  
    Wire.endTransmission();  
  } else if (tipo == QMC) {  
    Wire.beginTransmission(ender);  
    Wire.write(regSR_QMC);  
    Wire.write(0x01);  
    Wire.endTransmission();  
    Wire.beginTransmission(ender);  
    Wire.write(regCR1_QMC);  
    Wire.write(0x0D);  
    Wire.endTransmission();  
  }  
  
  return tipo != INDEFINIDO;  
}  
  
// Informa o tipo de bússola  
Bussola::TIPO Bussola::getTipo(void) {   
  return tipo;   
}  
  
// Define a declinação (correção entre o Norte magnético e o Norte geofráfico)  
// ver http://www.magnetic-declination.com/  
void Bussola::setDeclination (int graus , int mins, char dir) {  
  declination = (graus + mins/60.0) * PI / 180.0;  
  if (dir == 'W') {  
    declination = - declination;  
  }  
  Serial.println (declination);  
}  
  
  
// Le a direção da bússola em graus (0 a 360) em relação à marcação do eixo X  
// Assume que a bússola esta na horizontal  
float Bussola::leDirecao(void) {  
  int16_t x, y, z;  
  
  if (tipo == INDEFINIDO) {  
    return 0.0;  
  }  
  
  // Le a intesidade do campo magnético  
  if (tipo == HMC) {  
    pedeDados (regST_HMC, regXH_HMC);  
    x = Wire.read() << 8;     //MSB  x  
    x |= Wire.read();         //LSB  x  
    z = Wire.read() << 8;     //MSB  z  
    z |= Wire.read();         //LSB  z  
    y = Wire.read() << 8;     //MSB  y   
    y |= Wire.read();         //LSB  y  
  } else if (tipo == QMC) {  
    pedeDados(regST_QMC, regXL_QMC);  
    x = Wire.read();          //LSB  x   
    x |= Wire.read() << 8;    //MSB  x  
    y = Wire.read();          //LSB y  
    y |= Wire.read() << 8;    //MSB y  
    z = Wire.read();          //LSB  z  
    z |= Wire.read() << 8;    //MSB z  
  }  
  
  // Registra mínimo e máximo para a calibração  
  if (x < xMin) {  
    xMin = x;  
  }  
  if (xMax < x) {  
    xMax = x;  
  }  
  if (y < yMin) {  
    yMin = y;  
  }  
  if (yMax < y) {  
    yMax = y;  
  }  
  
  // corrige e calcula o angulo em radianos  
  float xC = (x - offX) * escX;  
  float yC = (y - offY) * escY;  
  float angulo = atan2 (yC, xC) + declination;  
  
  // Garante que está entre 0 e 2*PI  
  if (angulo < 0) {  
    angulo += 2.0*PI;  
  } else if (angulo > 2*PI) {  
    angulo -= 2.0*PI;  
  }  
  
  // Converte para graus  
  return (angulo*180.0)/PI;  
}  
  
void Bussola::pedeDados(int regStatus, int regDados) {  
    // Espera ter um dado a ler  
    do {  
      Wire.beginTransmission(ender);  
      Wire.write(regStatus);  
      Wire.endTransmission();  
      Wire.requestFrom(ender, 1);  
    } while ((Wire.read() & 1) == 0);  
  
    Wire.beginTransmission(ender);  
    Wire.write(regDados);  
    Wire.endTransmission();  
    Wire.requestFrom(ender, 6);  
}  
  
  
// Inicia processo de calibração  
void Bussola::iniciaCal() {  
  xMax = yMax = -32768;  
  xMin = yMin = 32767;  
}  
  
// Encerra a calibração  
void Bussola::encerraCal() {  
  Serial.print ("X: ");  
  Serial.print (xMin);  
  Serial.print (" - ");  
  Serial.println (xMax);  
  Serial.print ("Y: ");  
  Serial.print (yMin);  
  Serial.print (" - ");  
  Serial.println (yMax);  
    
  // Offset para centralizar leituras em zero  
  offX = (xMax + xMin)/2;  
  offY = (yMax + yMin)/2;  
  
  // Escala para ter a mesma variação nos dois eixos  
  int16_t varX = xMax - xMin;  
  int16_t varY = yMax - yMin;  
  if (varY > varX) {  
    escY = 1.0;  
    escX = (float) varY / varX;  
  } else {  
    escX = 1.0;  
    escY = (float) varX / varY;  
  }  
}  
