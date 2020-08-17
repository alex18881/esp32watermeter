/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#define DEBUG_ESP_CORE

#include "Arduino.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "appSettings.h"
#include "appWebServer.h"

#include <WiFi.h>

#define LED_BUILTIN 2
#define HOT_PIN 33
#define COLD_PIN 32
#define WIFI_CONNECT_TIMEOUT 20000
#define WIFI_HOST_NAME "watter-metter"
#define AP_SSID "WaterMetterAP"
#define AP_PASS "12345678"
#define HTROOT "/httroot"

//extern int lwip_bufferize;
AppSettings settings;
AppWebServer webServer;

const int debounceTime = 300;
int ledToggleOffPeriod = 500;
int ledTogglePeriod = 2000;
long ledToggleTime;
uint32_t ledValue;
long restartAt = 0;

int currVal1 = 0;
int currVal2 = 0;
long val1Debounce = 0;
long val2Debounce = 0;

void printMac(char *type, uint8_t *mac)
{
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println(F(""));
}

void incereaseValue(int valueNum, int val)
{
	if (val != HIGH) return;

	long currVal = settings.getValue(valueNum);
	int tickValue = settings.getTickValue(valueNum);

	settings.setValue(currVal + tickValue, valueNum);
	
	Serial.print(valueNum == APP_WM_VALUE1 ? F("Hot ticked ") : F("Cold ticked "));
	Serial.printf(" %ld liters", currVal);
	Serial.println(F(""));
}

void setup()
{
	// initialize LED digital pin as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(COLD_PIN, INPUT_PULLDOWN);
	pinMode(HOT_PIN, INPUT_PULLDOWN);

	Serial.begin(CONFIG_ESPTOOLPY_BAUD);
	Serial.setDebugOutput(true);
//	lwip_bufferize = 0;

	Serial.printf("Built at '%s' %s\n", __DATE__, __TIME__);
	Serial.println();

	settings.init();
	IPAddress ip(0,0,0,0);

	if (settings.getSSID().length() && !settings.getIgnoreWifi())
	{
		Serial.print(F("\tMac address: "));
		Serial.println(WiFi.macAddress());
		Serial.print(F("Connecting to SSID: "));
		Serial.println(settings.getSSID());

		WiFi.enableAP(false);
		delay(500);
		WiFi.mode(WIFI_STA);
		delay(500);

		WiFi.begin(settings.getSSID().c_str(), settings.getPasskey().c_str());
		long connectTimeout = millis() + WIFI_CONNECT_TIMEOUT;

		while (WiFi.status() != WL_CONNECTED && millis() < connectTimeout)
		{
			delay(500);
			Serial.print(F("."));
		}
		Serial.println(F(""));
		Serial.println(webServer.getWifiStatusName(WiFi.status()));
		if (WiFi.status() != WL_CONNECTED)
		{
			settings.setIgnoreWifi(true);
			ESP.restart();
			return;
		}
	}

	settings.setIgnoreWifi(false);

	if (WiFi.status() == WL_CONNECTED) {
		Serial.println(F("WiFi connected"));
		WiFi.setAutoReconnect(true);
		WiFi.setHostname(WIFI_HOST_NAME);
		ip = WiFi.localIP();
		ledTogglePeriod = 5000;
	} else {
		WiFi.disconnect(false);
		delay(500);
		WiFi.enableAP(true);
		delay(500);
		WiFi.mode(WIFI_AP);
		if (WiFi.softAP(AP_SSID, AP_PASS)) {
			Serial.print(F("WIFI Access point: "));
			Serial.println(webServer.getWifiStatusName(WiFi.status()));
			Serial.print(F("\tMac address: "));
			Serial.println(WiFi.macAddress());
			Serial.print(F("\tSSID: "));
			Serial.println(AP_SSID);
			Serial.print(F("\tPass phrase: "));
			Serial.println(AP_PASS);
			ip = WiFi.softAPIP();
			ledTogglePeriod = 1000;
		}
	}

	ledValue = HIGH;
	digitalWrite(LED_BUILTIN, ledValue);
	ledToggleTime = millis();

	Serial.println(F("Starting web server"));

	//webServer = AsyncWebServer(80);
	webServer.init(&settings);
	webServer.onRestart([]() { restartAt = millis() + 5000; });

	Serial.print(F("Web server IP: "));
	Serial.println(ip);

	currVal1 = digitalRead(HOT_PIN);
	currVal2 = digitalRead(COLD_PIN);
}

void loop()
{
	long currTime = millis();
	int togglePeriod = ledValue == HIGH ? ledToggleOffPeriod : ledTogglePeriod;

	if ((currTime - ledToggleTime) > togglePeriod)
	{
		ledValue = ledValue == HIGH ? LOW : HIGH;
		digitalWrite(LED_BUILTIN, ledValue);
		ledToggleTime = currTime;
	}

	if (restartAt > 0 && restartAt < millis())
	{
		ESP.restart();
	}

	int val1i = digitalRead(HOT_PIN);
	int val2i = digitalRead(COLD_PIN);

	if (val1i != currVal1 && val1Debounce < millis())
	{
		currVal1 = val1i;
		incereaseValue(APP_WM_VALUE1, currVal1);
		val1Debounce = millis() + debounceTime;
	}

	if (val2i != currVal2 && val2Debounce < millis())
	{
		currVal2 = val2i;
		incereaseValue(APP_WM_VALUE2, currVal2);
		val2Debounce = millis() + debounceTime;
	}

}