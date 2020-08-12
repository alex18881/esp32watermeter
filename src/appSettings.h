#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Preferences.h"

#define APP_WM_VALUE1 0
#define APP_WM_VALUE2 1

#define APP_SETTINGS_KEY_SSID "ssid"
#define APP_SETTINGS_KEY_KEY "key"
#define APP_SETTINGS_KEY_VAL1 "value1"
#define APP_SETTINGS_KEY_VAL2 "value2"
#define APP_SETTINGS_KEY_TIC1 "tickVal1"
#define APP_SETTINGS_KEY_TIC2 "tickVal2"
#define APP_SETTINGS_KEY_IWIFI "iWifi"
#define DEFAULT_TICK_VALUE 10

class AppSettings {
	public:
		AppSettings();

		void init();

		String getSSID();
		String getPasskey();
		bool getIgnoreWifi();
		long getValue(int valueNum);
		int getTickValue(int valueNum);

		void setSSID(String value);
		void setPasskey(String value);
		void setValue(long  value, int valueNum);
		void setTickValue(int value, int valueNum);
		void setIgnoreWifi(bool value);

		void clear();

	private:
		Preferences prefs;
		String ssid = "";
		String passKey = "";
		long val1 = 0;
		long val2 = 0;
		int tickValue1 = DEFAULT_TICK_VALUE;
		int tickValue2 = DEFAULT_TICK_VALUE;
		bool ignoreWifi = false;
		void startPrefs();
		void endPrefs();
};

#endif