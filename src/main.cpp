/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"
#include "WiFi.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "persistStorage.h"
#include "SPIFFS.h"
#include "WebServer.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define FORMAT_SPIFFS_IF_FAILED true
#define AP_SSID "WaterMetterAP"
#define AP_PASS "12345678"
#define HTROOT "/httroot"

PersistStorage storage;
WebServer webServer(80);

void printMac(char *type, uint8_t *mac)
{
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println("");
}

void get_404()
{
	Serial.println(F("Web server: 404"));
	File file = SPIFFS.open("/httroot/err404.html");
	webServer.send(301, "text/html", file.readString());
	file.close();
}

void bindStatics(const char *dirName)
{
	File link = SPIFFS.open(dirName);

	if (!link)
	{
		return;
	}

	if (link.isDirectory()) 
	{
		Serial.print(F("Dir:"));
		Serial.println(link.name());
		File child = link.openNextFile();
		while (child)
		{
			bindStatics(child.name());
			child.close();
			child = link.openNextFile();
		}
	} else {
		Serial.print(F("Binding static file:"));
		Serial.println(link.name());

		String fileName = String(link.name());
		fileName.replace(HTROOT, "");
		webServer.serveStatic(fileName.c_str(), SPIFFS, link.name());
	}
	link.close();
}

void setup()
{
	// initialize LED digital pin as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(CONFIG_ESPTOOLPY_BAUD);

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
	} else if( WiFi.softAP(AP_SSID, AP_PASS) ) {
		Serial.println(F("WIFI Access point"));
		Serial.print(F("\tSSID: "));
		Serial.println(AP_SSID);
		Serial.print(F("\tPass phrase: "));
		Serial.println(AP_PASS);
		ip = WiFi.softAPIP();
	}

	Serial.println(F("Starting web server"));

	webServer.serveStatic("/", SPIFFS, "/httroot/index.html");
	bindStatics(HTROOT);
	webServer.onNotFound(get_404);
	webServer.begin();
	Serial.print(F("Web server IP:"));
	Serial.println(ip);

	// uint8_t mac[6];
	// if( ESP_OK == esp_read_mac(mac, ESP_MAC_WIFI_STA))
	// {
	// 	printMac("STA", mac);
	// }

	// if( ESP_OK == esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP))
	// {
	// 	printMac("SOFT AP", mac);
	// }

	// if (ESP_OK == esp_read_mac(mac, ESP_MAC_BT))
	// {
	// 	printMac("BT", mac);
	// }

	// if (ESP_OK == esp_read_mac(mac, ESP_MAC_ETH))
	// {
	// 	printMac("ETH", mac);
	// }
}

void loop()
{
	webServer.handleClient();
}