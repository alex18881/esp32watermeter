#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Preferences.h"

#define APP_WM_VALUE1 0
#define APP_WM_VALUE2 1

class AppSettings {
	public:
		AppSettings();

		void init();

		String getSSID();
		String getPasskey();
		long getValue(int valueNum);
		int getValueDecimals(int valueNum);

		void setSSID(String value);
		void setPasskey(String value);
		void setValue(long  value, int valueNum);
		void setValueDecimals(int value, int valueNum);

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