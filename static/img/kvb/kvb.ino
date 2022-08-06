#include <Arduino.h>

// pour les deux afficheurs
#include <TM1637Display.h>

// pour la transmission PC
#include <ArduinoJson.h>

// pour le debouncing
#include <Bounce2.h>

#define CLK_PRINCIPAL PB12
#define DIO_PRINCIPAL PA8

#define CLK_AUXILIAIRE PB12
#define DIO_AUXILIAIRE PB15

#define V_LED PB11
#define FU_LED PB10
#define SOL_LED PB0
#define ENGIN_LED PB1

#define VAL PA15
#define MV PB4
#define FC PB5
#define TEST PB13

// boitier frein
#define FREINS PA4
#define FREIND PA5
#define GRANDDEBIT PB8

#define VAL_LIGHT PA8
#define FC_LIGHT PB9


// paramètres pour l'afficheur principal
// vous devrez sûrement modifier ces paramètres

uint8_t LTV[] = {0b00000000, 0b01110011, 0b00000000, 0b00000000}; // ça existe?

uint8_t eteint[] = {0b00000000, 0b00000000, 0b00000000, 0b00000000};      // 0.0
uint8_t tirets[] = {0b01000000, 0b01000000, 0b01000000, 0b00000000};      // 1.0
uint8_t zerozeroAux[] = {0b00000000, 0b00111111, 0b00111111, 0b00000000};   // 2.0
uint8_t zerozeroPrincipal[] = {0b00000000, 0b00111111, 0b00111111, 0b00000000}; // 3.0
uint8_t triplezeroAux[] = {0b00111111, 0b00111111, 0b00111111, 0b00000000};   // 4.0
uint8_t preannonce[] = {0b00000000, 0b01110011, 0b00000000, 0b00000000};    // 5.0
uint8_t preannonceAux[] = {0b00000000, 0b01011110, 0b00000000, 0b00000000};   // 6.0
uint8_t vitesseover160[] = {0b00000000, 0b01111100, 0b00000000, 0b00000000};  // 7.0
uint8_t autotest[] = {0b01111111, 0b01111111, 0b01111111, 0b00000000};      // 8.0
uint8_t faute[] = {0b00000000, 0b11110001, 0b00000000, 0b00000000};       // 9.0

// autotest
uint8_t PA400Aux[] = {0b00111111, 0b00111111, 0b01100110, 0b00000000};       // 1.0
uint8_t PA400[] = {0b01100110, 0b00111111, 0b00111111, 0b00000000};       // 1.0

uint8_t UC512Aux[] = {0b00000000, 0b00111110, 0b00111001, 0b00000000};       // 2.0
uint8_t UC512[] = {0b00000000, 0b01101101, 0b01011011, 0b00000110};       // 2.0


//typedef unsigned char uint8_t;
String inData;
StaticJsonDocument<256> doc;

String oldOutput;
StaticJsonDocument<128> docOutput;


Bounce VALbtn = Bounce();
Bounce MVbtn = Bounce();
Bounce FCbtn = Bounce();
Bounce TESTbtn = Bounce();
Bounce FREIN_S = Bounce();
Bounce FREIN_D = Bounce();
Bounce BPGD = Bounce();

TM1637Display visuPrincipal(CLK_PRINCIPAL, DIO_PRINCIPAL);
TM1637Display visuAux(CLK_AUXILIAIRE, DIO_AUXILIAIRE);

void separate()
{
  Serial.print(",");
}

int readButton(const int number)
{
  return 1 - digitalRead(number);
}

void configureButtons() {
  VALbtn.attach(VAL, INPUT_PULLUP);
  MVbtn.attach(MV, INPUT_PULLUP);
  FCbtn.attach(FC, INPUT_PULLUP);
  TESTbtn.attach(TEST, INPUT_PULLUP);
  FREIN_S.attach(FREINS, INPUT_PULLUP);
  FREIN_D.attach(FREIND, INPUT_PULLUP);
  BPGD.attach(GRANDDEBIT, INPUT_PULLUP);
}

void readAllButtons() {
  VALbtn.update();
  MVbtn.update();
  FCbtn.update();
  TESTbtn.update();
  FREIN_S.update();
  FREIN_D.update();
  BPGD.update();
}

void visuAuxBoot()
{
  visuPrincipal.setSegments(autotest);
  delay(500);
  visuPrincipal.setSegments(eteint);
  delay(500);
  visuPrincipal.setSegments(preannonce);
  delay(500);
  visuPrincipal.setSegments(vitesseover160);
  delay(500);
  visuPrincipal.setSegments(tirets);
  delay(1500);
  visuPrincipal.setSegments(tirets);
}

void visuPrincipalBoot()
{
  visuAux.setSegments(autotest);
  delay(500);
  visuAux.setSegments(eteint);
  delay(500);
  visuAux.setSegments(preannonce);
  delay(500);
  visuAux.setSegments(vitesseover160);
  delay(500);
  visuAux.setSegments(tirets);
  delay(1500);
}

void testLEDs()
{
      setLED(1, V_LED);
      delay(200);
      setLED(1, FU_LED);
      delay(200);
      setLED(1, SOL_LED);
      delay(200);
      setLED(1, ENGIN_LED);
      delay(200);
      setLED(0, V_LED);
      delay(200);
      setLED(0, FU_LED);
      delay(200);
      setLED(0, SOL_LED);
      delay(200);
      setLED(0, ENGIN_LED);
      delay(200);
      
}

void setLED(const int input, const int led) {
  if (input == 0) {
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
  }
}

void setButtonPins() {
  pinMode(ENGIN_LED, OUTPUT);
  pinMode(SOL_LED, OUTPUT);
  pinMode(V_LED, OUTPUT);
  pinMode(FU_LED, OUTPUT);

 // pinMode(PB11, OUTPUT);

  pinMode(FC_LIGHT, OUTPUT);
  pinMode(VAL_LIGHT, OUTPUT);
}

void setup()
{
  visuPrincipal.setBrightness(0x0f);
  visuAux.setBrightness(0x0f);
  // reset

  
  setButtonPins();
  configureButtons();

  visuPrincipalBoot();
  visuAuxBoot();

  testLEDs();
  
  Serial.begin(115200);

  while (Serial.available())
  {
    Serial.println("!");
  }
}



void loop()
{
/*  visuPrincipalBoot();
  visuAuxBoot();
  testLEDs();
*/
  readAllButtons();

  if (VALbtn.changed() || MVbtn.changed() || FCbtn.changed() || TESTbtn.changed() ||FREIN_D.changed() || FREIN_S.changed() || BPGD.changed() ) {
    /*int VALValue = VALbtn.read();
      int MVValue = MVbtn.read();
      int FCValue = FCbtn.read();
      int TESTValue = TESTbtn.read();*/
    float FreinAutomatique;
    if (int(FREIN_S.read()) == 0) {
      //Serial.println("ça déserre"); // -1
      FreinAutomatique = 1;
    } else if (int(FREIN_D.read()) == 0) {
      //      Serial.println("ça serre"); // 1
      FreinAutomatique = -1;
    } else {
      //Serial.println("rien"); // 0
      FreinAutomatique = 0;
    }
    docOutput["FA"] = FreinAutomatique ;


    docOutput["VAL"] = !VALbtn.read();
    docOutput["MV"] = false;
    docOutput["FC"] = !MVbtn.read();
    docOutput["TEST"] = !TESTbtn.read();
    docOutput["SF"] = !FCbtn.read();
    docOutput["BPGD"] = !BPGD.read();
    String output;
    serializeJson(docOutput, output);
    Serial.println(output);
  }

  char received = Serial.read();
  if (received != 255)
  {
    inData += received;
  }
  // Process message when new line character is received
  if (received == '\n')
  {
    deserializeJson(doc, inData);
    int visu = doc["visu"];       // 1
    int autotestTS = doc["autotest"]; // 3
    int LSFU = doc["LSFU"];         // 1
    int LSV = doc["LSV"];         // 1
    int panneSol = doc["SOL"];         // 1
    int panneEngin = doc["ENGIN"];         // 1
    int CG = doc["CG"];         // 1
    int FCLight = doc["FC"];         // 1
    int VALLight = doc["VAL"];         // 1


    if (autotestTS == 1 || autotestTS == 2 || autotestTS == 3)
    {
      switch (autotestTS)
      {
        case 1:
          visuPrincipal.showNumberDec(0004);
          visuAux.setSegments(PA400Aux);
          digitalWrite(FU_LED, LOW);
          digitalWrite(V_LED, LOW);
          digitalWrite(SOL_LED, LOW);
          digitalWrite(ENGIN_LED, LOW);
          break;

        case 2:
          visuPrincipal.showNumberDec(2150);
          visuAux.setSegments(UC512Aux);
          digitalWrite(FU_LED, LOW);
          digitalWrite(V_LED, LOW);
          digitalWrite(SOL_LED, HIGH);
          digitalWrite(ENGIN_LED, HIGH);
      }
    }
    else
    {
      switch (visu)
      {
        case 0: // éteint
          visuPrincipal.setSegments(eteint);
          visuAux.setSegments(eteint);
          break;

        case 1: // contrôle de vitesse actif (la vitesse maximale autorisée ne dépasse pas 160 km/h)
          visuPrincipal.setSegments(tirets);
          visuAux.setSegments(tirets);
          break;

        case 2: // contrôle d'annonce d'un signal d'arrêt fermé avec vitesse d'approche 30 km/h
          visuPrincipal.setSegments(eteint);
          visuAux.setSegments(zerozeroAux);
          break;

        case 3: // contrôle de vitesse à 30 km/h
          visuPrincipal.setSegments(zerozeroPrincipal);
          visuAux.setSegments(eteint);
          break;

        case 4: // contrôle d'annonce d'un signal d'arrêt fermé avec vitesse d'approche 10 km/h
          visuPrincipal.setSegments(eteint);
          visuAux.setSegments(triplezeroAux);
          break;

        case 5: // contrôle de vitesse à 160 km/h
          visuPrincipal.setSegments(preannonce);
          visuAux.setSegments(eteint);
          break;

        case 6: // contrôle de préannonce
          visuPrincipal.setSegments(eteint);
          visuAux.setSegments(preannonceAux);
          break;

        case 7: // contrôle de vitesse actif (la vitesse maximale autorisée est supérieure à 160 km/h si rien ne s'y oppose)
          visuAux.setSegments(eteint);
          visuPrincipal.setSegments(vitesseover160);
          break;

        case 8: // autotest ou test en cours*
          visuPrincipal.setSegments(autotest);
          visuAux.setSegments(autotest);
          break;

        case 9: // données incompatibles avec le tableau de cohérence
          visuPrincipal.setSegments(faute);
          visuAux.setSegments(eteint);
          break;

        default:
          break;
      }
      setLED(LSV, V_LED);
      setLED(LSFU, FU_LED);
      setLED(panneSol, SOL_LED);
      setLED(panneEngin, ENGIN_LED);
      setLED(FCLight, FC_LIGHT);
      setLED(VALLight, VAL_LIGHT);
    }


    //Serial.println(visu);
    inData = ""; // Clear received buffer
  }
}