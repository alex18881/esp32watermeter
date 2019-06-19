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

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void printMac(char* type, uint8_t* mac) {
	char buffer[17];
	Serial.printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.println("");
}

void setup()
{
	// initialize LED digital pin as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(CONFIG_ESPTOOLPY_BAUD);
	//WiFi.mode(WIFI_MODE_STA);
	//Serial.println("WIFI Mac: " + WiFi.macAddress());

	uint8_t mac[6];
	if( ESP_OK == esp_read_mac(mac, ESP_MAC_WIFI_STA))
	{
		printMac("STA", mac);
	}

	if( ESP_OK == esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP))
	{
		printMac("SOFT AP", mac);
	}

	if (ESP_OK == esp_read_mac(mac, ESP_MAC_BT))
	{
		printMac("BT", mac);
	}

	if (ESP_OK == esp_read_mac(mac, ESP_MAC_ETH))
	{
		printMac("ETH", mac);
	}
}

void loop()
{
	// turn the LED on (HIGH is the voltage level)
	digitalWrite(LED_BUILTIN, HIGH);

	// wait for a second
	delay(1000);

	// turn the LED off by making the voltage LOW
	digitalWrite(LED_BUILTIN, LOW);

	// wait for a second
	delay(1000);
}
