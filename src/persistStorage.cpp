#include "Arduino.h"
#include "persistStorage.h"

PersistStorage::PersistStorage() {}

void PersistStorage::init()
{
	Serial.print(F("EEPROM size: "));
	Serial.println((int)sizeof(CounterSettings));
	EEPROM.begin(sizeof(CounterSettings));
	
	loadSettings();
}

void PersistStorage::storeSettings() {
	Serial.println(F("EEPROM storring settings"));
	EEPROM.put(0, settings);
	EEPROM.commit();
}

void PersistStorage::loadSettings()
{
	Serial.println(F("EEPROM loading settings"));
	EEPROM.get(0, settings);
}