/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#define DEBUG_ESP_CORE

#include "Arduino.h"
#include "WiFi.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "persistStorage.h"
#include "SPIFFS.h"
//#include "AsyncJson.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "cJSON.h"

#define LED_BUILTIN 2
#define FORMAT_SPIFFS_IF_FAILED true
#define AP_SSID "WaterMetterAP"
#define AP_PASS "12345678"
#define HTROOT "/httroot"

PersistStorage storage;
AsyncWebServer webServer(80);
//extern int lwip_bufferize;
int ledToggleOffPeriod = 500;
int ledTogglePeriod = 2000;
long ledToggleTime;
uint32_t ledValue;

void printMac(char *type, uint8_t *mac)
{
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println("");
}

void get_404(AsyncWebServerRequest *request)
{
	Serial.println(F("Web server: 404"));
	File file = SPIFFS.open("/httroot/err404.html");
	request->send(301, "text/html", file.readString());
	file.close();
}

void get_apiWifiList(AsyncWebServerRequest *request)
{
	int count = WiFi.scanNetworks();
	cJSON *json = cJSON_CreateArray();

	for (int i = 0; i < count; i++)
	{
		cJSON *listItem = cJSON_CreateObject();
		cJSON_AddStringToObject(listItem, "ssid", WiFi.SSID(i).c_str());
		cJSON_AddNumberToObject(listItem, "rssi", WiFi.RSSI(i));
		cJSON_AddBoolToObject(listItem, "open", WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
		cJSON_AddItemToArray(json, listItem);
		Serial.print(F("Found SSID: "));
		Serial.println(WiFi.SSID(i));
		delay(10);
	}

	request->send(200, "application/json", cJSON_Print(json));
}

void setup()
{
	// initialize LED digital pin as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.setDebugOutput(true);
	Serial.begin(CONFIG_ESPTOOLPY_BAUD);
	//lwip_bufferize = 0;

	Serial.printf("Built at '%s' %s\n", __DATE__, __TIME__);
	Serial.println();

	if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
	{
		Serial.println("SPIFFS Mount Failed");
		return;
	}

	IPAddress ip(0,0,0,0);

	storage.init();

	if (sizeof(storage.settings.WifiSSID) > 0)
	{
		Serial.print(F("Connecting to SSID "));
		Serial.println(storage.settings.WifiSSID);

		WiFi.begin(storage.settings.WifiSSID, storage.settings.WifiPassword);

		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			Serial.print(F("."));
		}
		Serial.println(F(""));
		Serial.println(F("WiFi connected"));
		ip = WiFi.localIP();
		ledTogglePeriod = 5000;

	} else if( WiFi.softAP(AP_SSID, AP_PASS) ) {
		Serial.println(F("WIFI Access point"));
		Serial.print(F("\tSSID: "));
		Serial.println(AP_SSID);
		Serial.print(F("\tPass phrase: "));
		Serial.println(AP_PASS);
		ip = WiFi.softAPIP();
		ledTogglePeriod = 1000;
	}

	ledValue = HIGH;
	digitalWrite(LED_BUILTIN, ledValue);
	ledToggleTime = millis();

	Serial.println(F("Starting web server"));

	webServer.serveStatic("/", SPIFFS, "/httroot/")
		.setDefaultFile("index.html");
		
	webServer.on("/api/wifi-list", HTTP_GET, get_apiWifiList);
	webServer.onNotFound(get_404);
	webServer.begin();
	Serial.print(F("Web server IP:"));
	Serial.println(ip);
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
	
}