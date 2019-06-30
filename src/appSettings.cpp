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

long  AppSettings::getValue1() {
	return val1;
};

long  AppSettings::getValue2() {
	return val2;
};

int  AppSettings::getValue1Decimals() {
	return dec1;
};

int  AppSettings::getValue2Decimals() {
	return dec2;
};

void AppSettings::setSSID(String value){
	ssid = value;
	prefs.putString("ssid", ssid);
};

void  AppSettings::setPasskey(String value) {
	passKey = value;
	prefs.putString("key", passKey);
};
void  AppSettings::setValue1(long value) {
	val1 = value;
	prefs.putLong("value1", value);
};

void  AppSettings::setValue2(long value) {
	val2 = value;
	prefs.putLong("value2", value);
};

void  AppSettings::setValue1Decimals(int value) {
	dec1 = value;
	prefs.putInt("dec1", value);
};

void  AppSettings::setValue2Decimals(int value) {
	dec2 = value;
	prefs.putInt("dec2", value);
};