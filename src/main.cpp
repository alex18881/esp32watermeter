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
#include "SPIFFS.h"
#include "appSettings.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "cJSON.h"

#define LED_BUILTIN 2
#define FORMAT_SPIFFS_IF_FAILED true
#define WIFI_CONNECT_TIMEOUT 20000
#define AP_SSID "WaterMetterAP"
#define AP_PASS "12345678"
#define HTROOT "/httroot"


AsyncWebServer webServer(80);
AppSettings settings;

int ledToggleOffPeriod = 500;
int ledTogglePeriod = 2000;
long ledToggleTime;
uint32_t ledValue;
long restartAt = 0;

void printMac(char *type, uint8_t *mac)
{
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println("");
}

char * getWifiStatusName(wl_status_t status)
{
	char * result;
	switch (status)
	{
		case WL_IDLE_STATUS :
			result = (char *)"idle";
			break;
		case WL_NO_SSID_AVAIL :
			result = (char *)"available";
			break;
		case WL_SCAN_COMPLETED :
			result = (char *)"scan-complete";
			break;
		case WL_CONNECTED :
			result = (char *)"connected";
			break;
		case WL_CONNECT_FAILED :
			result = (char *)"conection-failed";
			break;
		case WL_CONNECTION_LOST :
			result = (char *)"connection-lost";
			break;
		case WL_DISCONNECTED :
			result = (char *)"disconnected";
			break;

		default:
			result = (char *)"unknown";
			break;
	}
	return result;
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
	Serial.println(F("Get Wifi list"));
	Serial.print(F("\tcurrent Wifi SSID"));
	Serial.println(settings.getSSID());

	int count = WiFi.scanNetworks();
	cJSON *result = cJSON_CreateObject();
	cJSON *jsonList = cJSON_AddArrayToObject(result, "listAvailable");

	for (int i = 0; i < count; i++)
	{
		cJSON *listItem;
		if (WiFi.SSID(i).compareTo(settings.getSSID()) == 0)
		{
			listItem = cJSON_AddObjectToObject(result, "connected");
			cJSON_AddStringToObject(listItem, "status", getWifiStatusName(WiFi.status()));
		}
		else
		{
			listItem = cJSON_CreateObject();
			cJSON_AddItemToArray(jsonList, listItem);
		}

		cJSON_AddStringToObject(listItem, "ssid", WiFi.SSID(i).c_str());
		cJSON_AddNumberToObject(listItem, "rssi", WiFi.RSSI(i));
		cJSON_AddBoolToObject(listItem, "open", WiFi.encryptionType(i) == WIFI_AUTH_OPEN);

		Serial.print(F("Found SSID: "));
		Serial.println(WiFi.SSID(i));
		delay(10);
	}

	request->send(200, "application/json", cJSON_Print(result));
}

void post_apiValuesUpdate(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to update values"));
	cJSON *result = cJSON_CreateObject();
	bool isOk = false;

	if (
		request->hasParam("val1", true) &&
		request->hasParam("val2", true) &&
		request->hasParam("dec1", true) &&
		request->hasParam("dec2", true)
	)
	{
		long val1 = request->getParam("val1", true)->value().toInt();
		long val2 = request->getParam("val2", true)->value().toInt();
		int dec1 = request->getParam("dec1", true)->value().toInt();
		int dec2 = request->getParam("dec2", true)->value().toInt();

		settings.setValue1(val1);
		settings.setValue1Decimals(dec1);
		settings.setValue2(val2);
		settings.setValue2Decimals(dec2);

		restartAt = millis() + 5000;
		isOk = true;
	}

	cJSON_AddBoolToObject(result, "success", isOk);

	request->send(200, "application/json", cJSON_Print(result));
}

void post_apiWifiConnect(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to connect to WIFI"));
	cJSON *result = cJSON_CreateObject();
	bool isOk = false;

	if (request->hasParam("ssid", true) && request->hasParam("passkey", true))
	{
		String ssid = request->getParam("ssid", true)->value();

		Serial.print(F("\t SSID: "));
		Serial.println(ssid);
		settings.setSSID(ssid);
		settings.setPasskey(request->getParam("passkey", true)->value());
		
		delay(500);

		restartAt = millis() + 5000;
		isOk = true;
	}

	cJSON_AddBoolToObject(result, "success", isOk);

	request->send(200, "application/json", cJSON_Print(result));
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

	settings.init();
	IPAddress ip(0,0,0,0);

	if (settings.getSSID().length())
	{
		Serial.print(F("Connecting to SSID: "));
		Serial.println(settings.getSSID());

		WiFi.begin(settings.getSSID().c_str(), settings.getPasskey().c_str());
		long connectTimeout = millis() + WIFI_CONNECT_TIMEOUT;

		while (WiFi.status() != WL_CONNECTED && millis() < connectTimeout)
		{
			delay(500);
			Serial.print(F("."));
		}
		Serial.println();
	}

	if (WiFi.status() == WL_CONNECTED) {
		Serial.println(F("WiFi connected"));
		WiFi.setAutoReconnect(true);
		WiFi.setHostname(AP_SSID);
		ip = WiFi.localIP();
		ledTogglePeriod = 5000;

	} else if( WiFi.softAP(AP_SSID, AP_PASS) ) {
		Serial.println(F("WIFI Access point"));
		Serial.print(F("\tMac address: "));
		Serial.println(WiFi.macAddress());
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

	//webServer = AsyncWebServer(80);

	webServer.serveStatic("/", SPIFFS, "/httroot/")
		.setDefaultFile("index.html");

	webServer.on("/api/values-update", HTTP_POST, post_apiValuesUpdate);
	webServer.on("/api/wifi-list", HTTP_GET, get_apiWifiList);
	webServer.on("/api/wifi-connect", HTTP_POST, post_apiWifiConnect);

	webServer.onNotFound(get_404);

	delay(2000);
	webServer.begin();
	Serial.print(F("Web server IP: "));
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

	if (restartAt > 0 && restartAt < millis())
	{
		ESP.restart();
	}
}