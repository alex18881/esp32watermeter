#include "appSettings.h"

AppSettings::AppSettings() {}

void AppSettings::init() {
	Serial.println(F("Initing preferences"));
	startPrefs();

	ssid = prefs.getString(APP_SETTINGS_KEY_SSID);
	passKey = prefs.getString(APP_SETTINGS_KEY_KEY);
	val1 = prefs.getLong(APP_SETTINGS_KEY_VAL1);
	val2 = prefs.getLong(APP_SETTINGS_KEY_VAL2);
	tickValue1 = prefs.getInt(APP_SETTINGS_KEY_TIC1);
	tickValue2 = prefs.getInt(APP_SETTINGS_KEY_TIC2);
	ignoreWifi = prefs.getBool(APP_SETTINGS_KEY_IWIFI);

	Serial.println(F("Init preferences done:"));

	Serial.print(F("\tSSID: "));
	Serial.println(ssid);
	Serial.print(F("\tPass key: "));
	Serial.println(passKey.length() > 0 ? "*****" : "-");
	Serial.printf("\tValue 1: %ld with %d liters per tick", val1, tickValue1);
	Serial.println();
	Serial.printf("\tValue 2: %ld with %d liters per tick", val2, tickValue2);
	Serial.println();
	prefs.end();
}

void AppSettings::startPrefs() {
	prefs.begin("settings", false);
	Serial.printf("Free %d entries", prefs.freeEntries());
	Serial.println(F(""));
}

void AppSettings::clear() {
	startPrefs();
	Serial.println(prefs.clear() ? F("Settings cleared") : F("Failed to clear settings"));
	prefs.end();
}

String AppSettings::getSSID() {
	return ssid;
};

String AppSettings::getPasskey() {
	return passKey;
};

bool AppSettings::getIgnoreWifi()
{
	return ignoreWifi;
}

long AppSettings::getValue(int valueNum) {
	return (valueNum == APP_WM_VALUE1) ? val1 : val2;
};

int AppSettings::getTickValue(int valueNum) {
	int val = (valueNum == APP_WM_VALUE1) ? tickValue1 : tickValue2;
	val = val > 0 ? val : DEFAULT_TICK_VALUE;
	return val;
};

void AppSettings::setSSID(String value){
	Serial.print(F("Update WIFI SSID: "));
	Serial.println(value);

	ssid = value;
	startPrefs();
	prefs.putString(APP_SETTINGS_KEY_SSID, ssid);
	prefs.end();
};

void AppSettings::setIgnoreWifi(bool value)
{
	Serial.print(F("Update ignore WIFI: "));
	Serial.println(value ? "true" : "false");

	ignoreWifi = value;
	startPrefs();
	prefs.putBool(APP_SETTINGS_KEY_IWIFI, ignoreWifi);
	prefs.end();
};

void  AppSettings::setPasskey(String value) {
	Serial.print(F("Update pass key: "));
	Serial.println(value);
	passKey = value;
	startPrefs();
	prefs.putString(APP_SETTINGS_KEY_KEY, passKey);
	prefs.end();
};
void  AppSettings::setValue(long value, int valueNum) {
	Serial.print(F("Update value "));
	Serial.printf("%d: %ld", valueNum, value);
	Serial.println(F(""));
	startPrefs();
	if (valueNum == APP_WM_VALUE1) {
		val1 = value;
		prefs.putLong(APP_SETTINGS_KEY_VAL1, value);
	}
	else
	{
		val2 = value;
		prefs.putLong(APP_SETTINGS_KEY_VAL2, value);
	}
	prefs.end();
};

void  AppSettings::setTickValue(int value, int valueNum) {
	Serial.printf("Update tick value %d: %d liters per tic", valueNum, value);
	Serial.println(F(""));
	startPrefs();
	if (valueNum == APP_WM_VALUE1) {
		tickValue1 = value;
		prefs.putBool(APP_SETTINGS_KEY_TIC1, value);
	}
	else
	{
		tickValue2 = value;
		prefs.putBool(APP_SETTINGS_KEY_TIC2, value);
	}
	prefs.end();
};