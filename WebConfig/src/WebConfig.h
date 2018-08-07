/*
 * WebConfig.h
 *
 *  Created on: 29.07.2018
 *      Author: Elissa
 */

#ifndef WEBCONFIG_H_
#define WEBCONFIG_H_

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern "C" {

//extern boolean settingMode;

//void setupMode(IPAddress* apIp, const char* apSSID, ESP8266WebServer* webServer, DNSServer* dnsServer);
String makePage(String title, String contents);
String cleanString(String input);
String urlDecode(String input);
String settingsInput(String key, uint16_t length, String type);
String settingsInputNumber(String key, int min, int max, int step);
String settingsInputIp(String caption, String key);
String settingsInputText(String caption, String key, uint16_t length);
String htmlLink(String caption, String url);

void wifiConfigInit(ESP8266WebServer* server, void (*setApCallback)(String ssid, String pass));

}
#endif /* WEBCONFIG_H_ */
