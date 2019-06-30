#include "Arduino.h"
#include "persistStorage.h"

char marker = 'W';

PersistStorage::PersistStorage() {
	//EEPROM.
}

void PersistStorage::init()
{
	EEPROM.begin(sizeof(char) + sizeof(CounterSettings));
	Serial.print(F("EEPROM size: "));
	Serial.println(EEPROM.length());

	loadSettings();
}

void PersistStorage::storeSettings() {
	Serial.println(F("EEPROM storring settings"));
	EEPROM.put(0, marker);
	EEPROM.put(sizeof(char), settings);
	EEPROM.commit();
}

void PersistStorage::loadSettings()
{
	Serial.println(F("EEPROM loading settings"));
	char test = 'a';
	EEPROM.get(0, test);
	if (test == marker) {
		Serial.print(F("EEPROM load: check passed"));
		Serial.println(test);
		EEPROM.get(sizeof(char), settings);
	}
}