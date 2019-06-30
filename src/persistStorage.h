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
			String WifiSSID;
			String WifiPassword;
			long value1;
			int valDecimals;
			long value2;
			int val2Decimals;
		};

		CounterSettings settings;
		
		void storeSettings();

	private:
		void loadSettings();
};

#endif