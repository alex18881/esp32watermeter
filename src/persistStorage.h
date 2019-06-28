#ifndef PERSIST_STORAGE
#define PERSIST_STORAGE

#include <EEPROM.h>
#define EEPROM_SIZE 1

class PersistStorage {
	public:
		PersistStorage();
		void init();

		struct CounterSettings
		{
			char WifiSSID[];
			char WifiPassword[];
			long value1;
			long value2;
		};

		CounterSettings settings;

	private:
		void loadSettings();
		void storeSettings();
};

#endif