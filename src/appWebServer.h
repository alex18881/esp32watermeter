#ifndef APP_WEB_SERVER
#define APP_WEB_SERVER
#define FORMAT_SPIFFS_IF_FAILED true

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "cJSON.h"
#include "SPIFFS.h"
#include "esp_system.h"
#include "appSettings.h"

class AppWebServer {
	public:
		AppWebServer();
		void init(AppSettings *appSettings);
		char * getWifiStatusName(wl_status_t status);
		void onRestart(std::function<void()> callback);

	private:
		AppSettings *settings;
		AsyncWebServer webServer = AsyncWebServer(80);
		std::function<void()> restartHandler;
		void listFiles();
		void post_reset(AsyncWebServerRequest *request);
		void get_apiVoltage(AsyncWebServerRequest *request);
		void get_404(AsyncWebServerRequest *request);
		void get_apiWifiList(AsyncWebServerRequest *request);
		void get_apiValues(AsyncWebServerRequest *request);
		void post_apiValuesUpdate(AsyncWebServerRequest *request);
		void post_apiWifiConnect(AsyncWebServerRequest *request);
};

#endif