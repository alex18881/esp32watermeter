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
#include "SPIFFS.h"
#include "appSettings.h"
#include "cJSON.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define LED_BUILTIN 2
#define HOT_PIN 33
#define COLD_PIN 25
#define FORMAT_SPIFFS_IF_FAILED true
#define WIFI_CONNECT_TIMEOUT 20000
#define WIFI_HOST_NAME "watter-metter"
#define AP_SSID "WaterMetterAP"
#define AP_PASS "12345678"
#define HTROOT "/httroot"

//extern int lwip_bufferize;

AsyncWebServer webServer(80);
AppSettings settings;

int ledToggleOffPeriod = 500;
int ledTogglePeriod = 2000;
long ledToggleTime;
uint32_t ledValue;
long restartAt = 0;

int currVal1 = 0;
int currVal2 = 0;

void printMac(char *type, uint8_t *mac)
{
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println(F(""));
}

void listFiles() {
	File root = SPIFFS.open("/");

	File file = root.openNextFile();

	while (file) {
		Serial.printf("%s (%d)", file.name(), file.size());
		Serial.println(F(">"));
		Serial.print(file.readString());
		Serial.println(F("--"));
		file.close();
		file = root.openNextFile();
	}
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
		case WL_NO_SHIELD:
			result = (char *)"no shield";
			break;

		default:
			result = (char *)"unknown";
			break;
	}
	return result;
}

void get_404(AsyncWebServerRequest *request)
{
	Serial.print(F("Web server: 404 "));
	Serial.println(request->url());
	request->send(301, "text/html", "/err404.html");
}

void get_apiWifiList(AsyncWebServerRequest *request)
{
	Serial.println(F("Get Wifi list"));
	Serial.print(F("\tcurrent Wifi SSID "));
	Serial.println(settings.getSSID());

	int count = WiFi.scanNetworks(false, false, true);
	cJSON *result = cJSON_CreateObject();
	cJSON *jsonList = cJSON_AddArrayToObject(result, "listAvailable");

	Serial.print(F("\tfound networks: "));
	Serial.println(count);

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
	}

	WiFi.scanDelete();

	request->send(200, "application/json", cJSON_Print(result));
}

void get_apiValues(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to update values"));
	cJSON *result = cJSON_CreateObject();

	cJSON_AddNumberToObject(result, "value1", settings.getValue(APP_WM_VALUE1));
	cJSON_AddNumberToObject(result, "value2", settings.getValue(APP_WM_VALUE2));
	cJSON_AddNumberToObject(result, "decimals1", settings.getValueDecimals(APP_WM_VALUE1));
	cJSON_AddNumberToObject(result, "decimals2", settings.getValueDecimals(APP_WM_VALUE2));

	request->send(200, "application/json", cJSON_Print(result));
}

void get_apiVoltage(AsyncWebServerRequest *request)
{
	cJSON *result = cJSON_CreateObject();
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

		Serial.printf("\tvalue1: %ld, dec1: %d, value2: %ld, dec2: %d", val1, dec1, val2, dec2);
		Serial.println();

		settings.setValue(val1, APP_WM_VALUE1);
		settings.setValueDecimals(dec1, APP_WM_VALUE1);
		settings.setValue(val2, APP_WM_VALUE2);
		settings.setValueDecimals(dec2, APP_WM_VALUE2);

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

		restartAt = millis() + 5000;
		isOk = true;
	}

	cJSON_AddBoolToObject(result, "success", isOk);

	request->send(200, "application/json", cJSON_Print(result));
}

void incereaseValue(int valueNum)
{
	int currDec = settings.getValueDecimals(valueNum);
	long currVal = settings.getValue(valueNum);
	currDec += 1; // Increase by 10 liters
	if (currDec >= 100) // each 1000 liters increasing main value
	{
		currDec = 0;
		currVal += 1;
		settings.setValue(currVal, valueNum);
	}
	settings.setValueDecimals(currDec, valueNum);
	Serial.printf(" %ld cubes, %d0 liters", currVal, currDec);
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

	if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
	{
		Serial.println("SPIFFS Mount Failed");
		return;
	} else {
		Serial.println("SPIFFS Mounted. FilesList:");
		listFiles();
	}

	settings.init();
	IPAddress ip(0,0,0,0);

	if (settings.getSSID().length() && !settings.getIgnoreWifi())
	{
		Serial.print(F("\tMac address: "));
		Serial.println(WiFi.macAddress());
		Serial.print(F("Connecting to SSID: "));
		Serial.println(settings.getSSID());

		WiFi.mode(WIFI_STA);
		WiFi.softAP(WIFI_HOST_NAME);

		WiFi.begin(settings.getSSID().c_str(), settings.getPasskey().c_str());
		long connectTimeout = millis() + WIFI_CONNECT_TIMEOUT;

		while (WiFi.status() != WL_CONNECTED && millis() < connectTimeout)
		{
			delay(500);
			Serial.print(F("."));
		}

		Serial.println(getWifiStatusName(WiFi.status()));
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
		WiFi.setHostname(AP_SSID);
		ip = WiFi.localIP();
		ledTogglePeriod = 5000;
	} else {
		WiFi.disconnect(false);
		delay(500);
		WiFi.mode(WIFI_AP);
		if (WiFi.softAP(AP_SSID, AP_PASS)) {
			Serial.print(F("WIFI Access point: "));
			Serial.println(getWifiStatusName(WiFi.status()));
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

	webServer.serveStatic("/", SPIFFS, "/httroot/").setDefaultFile("index.html");

	webServer.on("/api/voltage", HTTP_GET, get_apiVoltage);
	webServer.on("/api/values", HTTP_GET, get_apiValues);
	webServer.on("/api/values-update", HTTP_POST, post_apiValuesUpdate);
	webServer.on("/api/wifi-list", HTTP_GET, get_apiWifiList);
	webServer.on("/api/wifi-connect", HTTP_POST, post_apiWifiConnect);

	webServer.onNotFound(get_404);

	delay(2000);

	webServer.begin();
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

	if (val1i != currVal1)
	{
		Serial.print(F("Hot ticked "));
		currVal1 = val1i;
		incereaseValue(APP_WM_VALUE1);
		Serial.println();
	}

	if (val2i != currVal2)
	{
		Serial.print(F("Cold ticked "));
		currVal2 = val2i;
		incereaseValue(APP_WM_VALUE2);
		Serial.println();
	}

}