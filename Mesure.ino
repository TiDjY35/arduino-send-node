// Fonction rÃ©cupÃ©rant la tempÃ©rature depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
boolean getTemperature(float *temp) {
  byte data[9], addr[8];
  // data : DonnÃ©es lues depuis le scratchpad
  // addr : adresse du module 1-Wire dÃ©tectÃ©

  if (!ds.search(addr)) { // Recherche un module 1-Wire
    ds.reset_search();    // RÃ©initialise la recherche de module
    return false;         // Retourne une erreur
  }

  if (OneWire::crc8(addr, 7) != addr[7]) // VÃ©rifie que l'adresse a Ã©tÃ© correctement reÃ§ue
    return false;                        // Si le message est corrompu on retourne une erreur

  if (addr[0] != DS18B20) // VÃ©rifie qu'il s'agit bien d'un DS18B20
    return false;         // Si ce n'est pas le cas on retourne une erreur

  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sÃ©lectionne le DS18B20

  ds.write(0x44, 1);      // On lance une prise de mesure de tempÃ©rature
  delay(800);             // Et on attend la fin de la mesure

  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sÃ©lectionne le DS18B20
  ds.write(0xBE);         // On envoie une demande de lecture du scratchpad

  for (byte i = 0; i < 9; i++) // On lit le scratchpad
    data[i] = ds.read();       // Et on stock les octets reÃ§us

  // Calcul de la tempÃ©rature en degrÃ© Celsius
  *temp = ((data[1] << 8) | data[0]) * 0.0625;

  // Pas d'erreur
  return true;
}

// Fonction récupération du niveau d'eau
boolean getNiveau(float *niv) {
  // Mesure niveau d'eau du bac
  if (analogRead(0) > 1000) {
    digitalWrite(A3, LOW);
  }
  else {
    digitalWrite(A3, HIGH);
  }
  *niv = analogRead(0);
}
