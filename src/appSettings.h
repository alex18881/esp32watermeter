#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Preferences.h"

class AppSettings {
	public:
		AppSettings();

		void init();

		String getSSID();
		String getPasskey();
		long getValue1();
		long getValue2();
		int getValue1Decimals();
		int getValue2Decimals();

		void setSSID(String value);
		void setPasskey(String value);
		void setValue1(long  value);
		void setValue2(long  value);
		void setValue1Decimals(int  value);
		void setValue2Decimals(int  value);

	private:
		Preferences prefs;
		String ssid = "";
		String passKey = "";
		long val1 = 0;
		long val2 = 0;
		int dec1 = 0;
		int dec2 = 0;

};

#endif