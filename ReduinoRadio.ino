#include <EEPROM.h>
#include <OneWire.h> // Inclusion de la librairie OneWire

#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 49 // Broche utilisÃ©e pour le bus 1-Wire
#define reserve A0// broche analogique 0 = niveau reserve eau osmosÃ© 

OneWire ds(BROCHE_ONEWIRE); // CrÃ©ation de l'objet OneWire ds

int pinLed = 13;
int pinEmetteur = 12;
long codeKit = 1001;  // Code de notre couple arduino + sonde dans notre systeme radio
int BytesTypeTemp[] = {1, 0, 1, 0}; // Code du type de transmission, ici 10 pour Ã©mission d'une temperature, exprimÃ© en binaire
int BytesTypeNiv[] = {1, 0, 1, 1}; // Code du type de transmission, ici 11 pour Ã©mission du niveau, exprimÃ© en binaire
int Bytes[14]; //Message binaire Ã  envoyer
int BytesData[14]; //Temperature
int BytesNiveau[14]; //Niveau

// Servent à transformer des données numériques en tableaux de bits
void itob(unsigned long integer, int length)
{
  for (int i = 0; i < length; i++) {
    if ((integer / power2(length - 1 - i)) == 1) {
      integer -= power2(length - 1 - i);
      Bytes[i] = 1;
    }
    else Bytes[i] = 0;
  }
}
// Sert à transformer des données numériques de la temperature en tableaux de bits
void itobTemperature(unsigned long integer, int length)
{
  for (int i = 0; i < length; i++) {
    if ((integer / power2(length - 1 - i)) == 1) {
      integer -= power2(length - 1 - i);
      BytesData[i] = 1;
    }
    else BytesData[i] = 0;
  }
}

// Sert à transformer des données numériques du niveau en tableaux de bits
void itobNiveau(unsigned long integer, int length)
{
  for (int i = 0; i < length; i++) {
    if ((integer / power2(length - 1 - i)) == 1) {
      integer -= power2(length - 1 - i);
      BytesNiveau[i] = 1;
    }
    else BytesNiveau[i] = 0;
  }
}

unsigned long power2(int power) {   //gives 2 to the (power)
  unsigned long integer = 1;
  for (int i = 0; i < power; i++) {
    integer *= 2;
  }
  return integer;
}

/**
 * CrÃ©e notre signal sous forme binaire
**/
void buildSignal()
{
  Serial.println(codeKit);
  // Converti les codes respectifs pour le signal en binaire
  itob(codeKit, 14);
  for (int j = 0; j < 14; j++) {
    Serial.print(Bytes[j]);
  }
  Serial.println();
}

//Envoie d'une paire de pulsation radio qui definissent 1 bit rÃ©el : 0 =01 et 1 =10
//c'est le codage de manchester qui necessite ce petit bouzin, ceci permet entre autres de dissocier les donnÃ©es des parasites
void sendPair(bool b) {
  if (b)
  {
    sendBit(true);
    sendBit(false);
  }
  else
  {
    sendBit(false);
    sendBit(true);
  }
}

//Envois d'une pulsation (passage de l'etat haut a l'etat bas)
//1 = 310Âµs haut puis 1340Âµs bas
//0 = 310Âµs haut puis 310Âµs bas
void sendBit(bool b) {
  if (b) {
    Serial.print('1');
    digitalWrite(pinEmetteur, HIGH);
    delayMicroseconds(310);   //275 orinally, but tweaked.
    digitalWrite(pinEmetteur, LOW);
    delayMicroseconds(1340);  //1225 orinally, but tweaked.
  }
  else {
    Serial.print(0);
    digitalWrite(pinEmetteur, HIGH);
    delayMicroseconds(310);   //275 orinally, but tweaked.
    digitalWrite(pinEmetteur, LOW);
    delayMicroseconds(310);   //275 orinally, but tweaked.
  }
}

/**
 * Transmit Temperature
 * @param boolean  positive temperature negative ou positive
 * @param int temperature temperature en centiÃ¨mes de degrÃ©s 2015 = 20,15Â°C
 **/
void transmitTemp(boolean positive, int temperature)
{
  int i;
  itobTemperature(temperature, 14);

  digitalWrite(pinLed, HIGH);
  // Sequence de verrou anoncant le dÃ©part du signal au recepeteur
  digitalWrite(pinEmetteur, HIGH);
  delayMicroseconds(275);     // un bit de bruit avant de commencer pour remettre les delais du recepteur a 0
  digitalWrite(pinEmetteur, LOW);
  delayMicroseconds(9900);     // premier verrou de 9900Âµs
  digitalWrite(pinEmetteur, HIGH);   // high again
  delayMicroseconds(275);      // attente de 275Âµs entre les deux verrous
  digitalWrite(pinEmetteur, LOW);    // second verrou de 2675Âµs
  delayMicroseconds(2500);
  digitalWrite(pinEmetteur, HIGH);  // On reviens en Ã©tat haut pour bien couper les verrous des donnÃ©es


  // Envoie du code du kit en binaire
  for (i = 0; i < 14; i++)
  {
    sendPair(Bytes[i]);
  }

  // Envoie du code de type de transmission
  for (int j = 0; j < 4; j++)
  {
    sendPair(BytesTypeTemp[j]);
  }

  // Envoie du type de la temperature (negative ou positive)
  // Envoie du bit dÃ©finissant si c'est une commande de groupe ou non (26em bit)
  sendPair(positive);

  // Envoie de la temperature
  for (int j = 0; j < 14; j++)
  {
    sendPair(BytesData[j]);
  }


  // Sequence de verrou finale
  digitalWrite(pinEmetteur, HIGH);
  delayMicroseconds(275);     // un bit de bruit avant de commencer pour remettre les delais du recepteur a 0
  digitalWrite(pinEmetteur, LOW);
  digitalWrite(pinLed, LOW);
}
/*
**
 * Transmit Niveau
 * @param boolean  positive temperature negative ou positive ( a corriger)
 * @param int niveau. C'est la valeur analogique du niveau
 **/
void transmitNiv(boolean positive, int niveau)
{
  int i;
  itobNiveau(niveau, 14);

  digitalWrite(pinLed, HIGH);
  // Sequence de verrou anoncant le dÃ©part du signal au recepeteur
  digitalWrite(pinEmetteur, HIGH);
  delayMicroseconds(275);     // un bit de bruit avant de commencer pour remettre les delais du recepteur a 0
  digitalWrite(pinEmetteur, LOW);
  delayMicroseconds(9900);     // premier verrou de 9900Âµs
  digitalWrite(pinEmetteur, HIGH);   // high again
  delayMicroseconds(275);      // attente de 275Âµs entre les deux verrous
  digitalWrite(pinEmetteur, LOW);    // second verrou de 2675Âµs
  delayMicroseconds(2500);
  digitalWrite(pinEmetteur, HIGH);  // On reviens en Ã©tat haut pour bien couper les verrous des donnÃ©es


  // Envoie du code du kit en binaire
  for (i = 0; i < 14; i++)
  {
    sendPair(Bytes[i]);
  }

  // Envoie du code de type de transmission
  for (int j = 0; j < 4; j++)
  {
    sendPair(BytesTypeNiv[j]);
  }

  // Envoie du type de la temperature (negative ou positive)
  // Envoie du bit dÃ©finissant si c'est une commande de groupe ou non (26em bit)
  sendPair(positive);

  // Envoie de la temperature
  for (int j = 0; j < 14; j++)
  {
    sendPair(BytesNiveau[j]);
  }


  // Sequence de verrou finale
  digitalWrite(pinEmetteur, HIGH);
  delayMicroseconds(275);     // un bit de bruit avant de commencer pour remettre les delais du recepteur a 0
  digitalWrite(pinEmetteur, LOW);
  digitalWrite(pinLed, LOW);
}

void setup()
{
  pinMode(pinEmetteur, OUTPUT);
  pinMode(pinLed, OUTPUT);
  Serial.begin(9600);
  Serial.println("setup");
  digitalWrite(pinLed, HIGH);

  buildSignal();
  digitalWrite(pinLed, LOW);
}

void loop()
{
  float temp;
  Serial.println("in loop");

  // Lit la tempÃ©rature ambiante Ã  ~1Hz
  if (getTemperature(&temp)) {

    // Affiche la tempÃ©rature
    Serial.print("Temperature : ");
    Serial.print(temp);
    Serial.write(176); // caractÃ¨re Â°
    Serial.write('C');
    Serial.println();
    temp = temp * 100;
    int tempInt = (int) temp;
    Serial.println(tempInt);
    transmitTemp(true, tempInt);
  }

  delay (6000);

  float niv;

  if (getNiveau(&niv)) {

    // Affiche le niveau
    Serial.print("Niveau eau : ");
    Serial.print(niv);
    Serial.println();
    int nivInt = (int) niv;
    Serial.println(nivInt);
    transmitNiv(true, nivInt);
  }


  delay(6000);
}
