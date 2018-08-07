#include "Arduino.h"

//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
//#include <EEPROM.h>

#include "WebConfig.h"

// Settings
String ssidList;

const IPAddress apIPDefault(192, 168, 1, 1);
const char* apSSIDDefault = "ESP8266_SETUP";

String urlWebPageWiFiSettings = "/wifiSettings";
String urlWebPageSetAp = "/setAp";

void (*callback)(String ssid, String pass);

static ESP8266WebServer* webServer;
/*
boolean settingMode = true;


void startWebServer(void);
*/

String cleanString(String input)
{
	uint32_t i = 0;
	Serial.println(input);

	while (i < input.length())
	{
		volatile char buffer = input[i];
		if ((buffer >= 'a' && buffer <= 'z') ||
		    (buffer >= 'A' && buffer <= 'Z'))
		{
			i++;
		}
		else
		{
			input.remove(i, 1);
		}
	}

	return input;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html>\r\n<head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">\r\n";
  s += "<title>";
  s += title;
  s += "</title></head>\r\n<body>\r\n";
  s += contents;
  s += "</body>\r\n</html>";
  return s;
}

void wifiConfigWebPageWiFiSettings() {
    String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
    s += "<form method=\"get\" action=\"" + urlWebPageSetAp + "\"><label>SSID: </label><select name=\"ssid\">";
    s += ssidList;
    s += "</select><p>Password: <input name=\"pass\" length=64 type=\"password\"></p><p><input type=\"submit\"></p></form>";
    webServer->send(200, "text/html", makePage("Wi-Fi Settings", s));
}

void wifiConfigWebPageSetAp() {
	String ssid = urlDecode(webServer->arg("ssid"));
	String pass = urlDecode(webServer->arg("pass"));

	callback(ssid, pass);

    String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
    s += ssid;
    s += "\" after restart.";
    s += "<p>" + htmlLink("restart device", "/restart") + "</p>";

	webServer->send(200, "text/html", makePage("Access Point Set", s));
}

void wifiConfigRestartDevice()
{
	webServer->send(200, "text/html", makePage("Restart Device", "Restart Device - please wait..."));
	delay(1000);
	ESP.restart();
}

void wifiConfigInit(ESP8266WebServer* server, void (*setApCallback)(String ssid, String pass))
{
	callback = setApCallback;
	webServer = server;

	WiFiMode_t wifiMode = WiFi.getMode();
	if (WiFi.status() == WL_CONNECTED)
	{
		WiFi.disconnect();
	}

	WiFi.mode(WIFI_STA);
	delay(100);
	int n = WiFi.scanNetworks();
	delay(100);
	Serial.println("");

	for (uint8_t i = 0; i < n; ++i) {
		ssidList += "<option value=\"";
		ssidList += WiFi.SSID(i);
		ssidList += "\">";
		ssidList += WiFi.SSID(i);
		ssidList += "</option>";
	}

	WiFi.mode(wifiMode);
	delay(100);

	webServer->on(urlWebPageWiFiSettings, wifiConfigWebPageWiFiSettings);
	webServer->on(urlWebPageSetAp, wifiConfigWebPageSetAp);
	webServer->on("/restart", wifiConfigRestartDevice);
}

/*
void setupMode(IPAddress* apIP, const char* apSSID, ESP8266WebServer* webSrv, DNSServer* dnsServer) {
	webServer = webSrv;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  for (uint8_t i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(*apIP, *apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  Serial.print("Starting DNS: ");
  Serial.println(dnsServer->start(53, "*", *apIP));
  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(apSSID);
  Serial.println("\"");
}

void startWebServer(void) {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());

    webServer->on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer->send(200, "text/html", makePage("Wi-Fi Settings", s));
    });

    webServer->on("/setap", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      String ssid = urlDecode(webServer->arg("ssid"));
      Serial.print("SSID: ");
      Serial.println(ssid);
      String pass = urlDecode(webServer->arg("pass"));
      Serial.print("Password: ");
      Serial.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      for (uint32_t i = 0; i < ssid.length(); ++i) {
        EEPROM.write(i, ssid[i]);
      }
      Serial.println("Writing Password to EEPROM...");
      for (uint32_t i = 0; i < pass.length(); ++i) {
        EEPROM.write(32 + i, pass[i]);
      }
      EEPROM.commit();
      Serial.println("Write EEPROM done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer->send(200, "text/html", makePage("Wi-Fi Settings", s));
      ESP.restart();
    });

    webServer->onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer->send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    webServer->on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer->send(200, "text/html", makePage("STA mode", s));
    });
    webServer->on("/reset", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer->send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
    });
  }
  webServer->begin();
}
*/

String settingsInput(String key, uint16_t length, String type)
{
	String s = "<input name=\"";
	s += key + "\" length=" + length + " type=\"" + type + "\">";

	return s;
}

String settingsInputNumber(String key, int min, int max, int step)
{
	String s = "<input name=\"";
	s += key + "\" min=\"" + min + "\" max=\"" + max + "\" step=\"" + step + "\" type=\"number\">";

	return s;
}

String settingsInputIp(String caption, String key)
{
	String s = caption + ": ";
	s += settingsInputNumber(key + "3", 0, 255, 1) + ".";
	s += settingsInputNumber(key + "2", 0, 255, 1) + ".";
	s += settingsInputNumber(key + "1", 0, 255, 1) + ".";
	s += settingsInputNumber(key + "0", 0, 255, 1);

	return s;
}

String settingsInputText(String caption, String key, uint16_t length)
{
	String s = caption + ": " + settingsInput(key, length, "text");
	return s;
}

String htmlLink(String caption, String url)
{
	return "<a href=\"" + url + "\">" + caption + "</a>";
}
