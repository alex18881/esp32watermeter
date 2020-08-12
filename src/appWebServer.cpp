#include "appWebServer.h"

AppWebServer::AppWebServer() {}

void AppWebServer::init(AppSettings *appSettings) {
	settings = appSettings;

	if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
	{
		Serial.println(F("SPIFFS Mount Failed"));
		return;
	}
	else {
		Serial.println(F("SPIFFS Mounted. FilesList:"));
		listFiles();
	}

	using namespace std::placeholders; // for `_1`

	webServer.serveStatic("/", SPIFFS, "/httroot/").setDefaultFile("index.html");

	webServer.on("/api/voltage", HTTP_GET, std::bind(&AppWebServer::get_apiVoltage, this, _1));
	webServer.on("/api/values", HTTP_GET, std::bind(&AppWebServer::get_apiValues, this, _1));
	webServer.on("/api/values-update", HTTP_POST, std::bind(&AppWebServer::post_apiValuesUpdate, this, _1));
	webServer.on("/api/reset", HTTP_POST, std::bind(&AppWebServer::post_reset, this, _1));
	webServer.on("/api/wifi-list", HTTP_GET, std::bind(&AppWebServer::get_apiWifiList, this, _1));
	webServer.on("/api/wifi-connect", HTTP_POST, std::bind(&AppWebServer::post_apiWifiConnect, this, _1));

	webServer.onNotFound(std::bind(&AppWebServer::get_404, this, _1));

	delay(2000);

	webServer.begin();
}

void AppWebServer::onRestart(std::function<void()> callback) {
	restartHandler = callback;
}

char * AppWebServer::getWifiStatusName(wl_status_t status)
{
	char * result;
	switch (status)
	{
	case WL_IDLE_STATUS:
		result = (char *)"idle";
		break;
	case WL_NO_SSID_AVAIL:
		result = (char *)"available";
		break;
	case WL_SCAN_COMPLETED:
		result = (char *)"scan-complete";
		break;
	case WL_CONNECTED:
		result = (char *)"connected";
		break;
	case WL_CONNECT_FAILED:
		result = (char *)"conection-failed";
		break;
	case WL_CONNECTION_LOST:
		result = (char *)"connection-lost";
		break;
	case WL_DISCONNECTED:
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

void AppWebServer::listFiles()
{
	File root = SPIFFS.open("/");

	File file = root.openNextFile();

	while (file) {
		Serial.printf("\t%s (%d)", file.name(), file.size());
		Serial.println("");
		file.close();
		file = root.openNextFile();
	}
}

void AppWebServer::get_404(AsyncWebServerRequest *request)
{
	Serial.print(F("Web server: 404 "));
	Serial.println(request->url());
	request->send(301, "text/html", "/err404.html");
}

void AppWebServer::get_apiWifiList(AsyncWebServerRequest *request)
{
	Serial.println(F("Get Wifi list"));
	Serial.print(F("\tcurrent Wifi SSID "));
	Serial.println(settings->getSSID());

	int count = WiFi.scanNetworks(false, false, true);
	cJSON *result = cJSON_CreateObject();
	cJSON *jsonList = cJSON_AddArrayToObject(result, "listAvailable");

	Serial.print(F("\tfound networks: "));
	Serial.println(count);

	for (int i = 0; i < count; i++)
	{
		cJSON *listItem;
		if (WiFi.SSID(i).compareTo(settings->getSSID()) == 0)
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

void AppWebServer::get_apiValues(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to read values"));
	cJSON *result = cJSON_CreateObject();

	cJSON_AddNumberToObject(result, "value1", settings->getValue(APP_WM_VALUE1));
	cJSON_AddNumberToObject(result, "value2", settings->getValue(APP_WM_VALUE2));
	cJSON_AddNumberToObject(result, "ticks1", settings->getTickValue(APP_WM_VALUE1));
	cJSON_AddNumberToObject(result, "ticks2", settings->getTickValue(APP_WM_VALUE2));

	request->send(200, "application/json", cJSON_Print(result));
}

void AppWebServer::get_apiVoltage(AsyncWebServerRequest *request)
{
	cJSON *result = cJSON_CreateObject();
	request->send(200, "application/json", cJSON_Print(result));
}

void AppWebServer::post_apiValuesUpdate(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to update values"));
	cJSON *result = cJSON_CreateObject();
	bool isOk = false;

	if (
		request->hasParam("val1", true) &&
		request->hasParam("val2", true) &&
		request->hasParam("ticks1", true) &&
		request->hasParam("ticks2", true)
		)
	{
		long val1 = request->getParam("val1", true)->value().toInt();
		long val2 = request->getParam("val2", true)->value().toInt();
		int ticks1 = request->getParam("ticks1", true)->value().toInt();
		int ticks2 = request->getParam("ticks2", true)->value().toInt();

		Serial.printf("\tvalue1: %ld, ticks1: %d, value2: %ld, ticks2: %d", val1, ticks1, val2, ticks2);
		Serial.println();

		settings->setValue(val1, APP_WM_VALUE1);
		settings->setTickValue(ticks2, APP_WM_VALUE2);
		settings->setValue(val2, APP_WM_VALUE2);
		settings->setTickValue(ticks1, APP_WM_VALUE1);

		restartHandler();
		isOk = true;
	}

	cJSON_AddBoolToObject(result, "success", isOk);

	request->send(200, "application/json", cJSON_Print(result));
}

void AppWebServer::post_apiWifiConnect(AsyncWebServerRequest *request)
{
	Serial.println(F("Got request to connect to WIFI"));
	cJSON *result = cJSON_CreateObject();
	bool isOk = false;

	if (request->hasParam("ssid", true) && request->hasParam("passkey", true))
	{
		String ssid = request->getParam("ssid", true)->value();

		Serial.print(F("\t SSID: "));
		Serial.println(ssid);
		settings->setSSID(ssid);
		settings->setPasskey(request->getParam("passkey", true)->value());

		restartHandler();
		isOk = true;
	}

	cJSON_AddBoolToObject(result, "success", isOk);

	request->send(200, "application/json", cJSON_Print(result));
}

void AppWebServer::post_reset(AsyncWebServerRequest *request) {
	Serial.println(F("Got request to reset values"));

	restartHandler();
	cJSON *result = cJSON_CreateObject();
	cJSON_AddBoolToObject(result, "success", true);
	request->send(200, "application/json", cJSON_Print(result));
}