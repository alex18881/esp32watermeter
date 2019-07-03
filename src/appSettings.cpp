#include "appSettings.h"

AppSettings::AppSettings() {}

void AppSettings::init() {
	Serial.println("Initing preferences");
	prefs.begin("settings", false);

	ssid = prefs.getString("ssid");
	passKey = prefs.getString("key");
	val1 = prefs.getLong("value1");
	val2 = prefs.getLong("value2");
	dec1 = prefs.getInt("dec1");
	dec2 = prefs.getInt("dec2");
	ignoreWifi = prefs.getBool("iWifi");

	Serial.println("Init preferences done:");

	Serial.print("\tSSID: ");
	Serial.println(ssid);
	Serial.print("\tPass key: ");
	Serial.println(passKey);
	Serial.print("\tValue 1: ");
	Serial.print(val1);
	Serial.print(".");
	Serial.println(dec1);
	Serial.print("\tValue 2: ");
	Serial.print(val2);
	Serial.print(".");
	Serial.println(dec2);
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

long  AppSettings::getValue(int valueNum) {
	return (valueNum == APP_WM_VALUE1) ? val1 : val2;
};

int  AppSettings::getValueDecimals(int valueNum) {
	return (valueNum == APP_WM_VALUE1) ? dec1 : dec2;
};

void AppSettings::setSSID(String value){
	ssid = value;
	prefs.putString("ssid", ssid);
};

void AppSettings::setIgnoreWifi(bool value)
{
	ignoreWifi = value;
	prefs.putBool("iWifi", ignoreWifi);
};

void  AppSettings::setPasskey(String value) {
	passKey = value;
	prefs.putString("key", passKey);
};
void  AppSettings::setValue(long value, int valueNum) {
	if (valueNum == APP_WM_VALUE1) {
		val1 = value;
		prefs.putLong("value1", value);
	}
	else
	{
		val2 = value;
		prefs.putLong("value2", value);
	}
};

void  AppSettings::setValueDecimals(int value, int valueNum) {
	if (valueNum == APP_WM_VALUE1)
	{
		dec1 = value;
		prefs.putInt("dec1", value);
	}
	else
	{
		dec2 = value;
		prefs.putInt("dec2", value);
	}
};