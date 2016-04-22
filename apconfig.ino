#define LOCALAPSTATUS_INIT		0
#define LOCALAPSTATUS_STARTED	1
#define LOCALAPSTATUS_MAINTAIN	2
int localApStatus = LOCALAPSTATUS_INIT;

String localSsid = "ESP_01560E";
String localPass = "Esp8266_01560E";

ESP8266WebServer server(80);

void handleRoot()
{
	lLastActivity = rtc.getUnixTime(rtc.getTime());
	String content = "<h1>HELLO WORLD</h1>";

	content += "<h2>Local Access Point</h2>";
	content += "<p><b>SSID: </b>" + localSsid + "</p>";
	content += "<p><b>LAN IP: </b>" + WiFi.softAPIP().toString() + "</p>";

	//content += "<h2>Remote Access Point</h2>";
	//if (WiFi.status() == wl_status_t::WL_CONNECTED)
	//{
	//	content += "<p><b>SSID: </b>" + WiFi.SSID() + "</p>";
	//	content += "<p><b>RSSI: </b>" + String(WiFi.RSSI(), DEC) + "</p>";
	//	content += "<p><b>WAN IP: </b>" + WiFi.localIP().toString() + "</p>";
	//}
	//else
	//{
	//	content += "<p>Remote access point is not connected.</p>";
	//}

	//content += "<h2>Configuration</h2>";
	//content += "<p><a href='/test'>Test Remote AP Settings &amp; Disconnect</a></p>";
	//content += "<p><a href='/maintain'>Keep connected to Remote AP</a></p>";

	server.send(200, ((String)"text/html").c_str(), String(content));
}

void LocalApLoop()
{
	switch (localApStatus)
	{
	case LOCALAPSTATUS_INIT:
		Serial.println("Starting local AP: " + localSsid + "...");
		WiFi.softAP(localSsid.c_str(), localPass.c_str());
		Serial.println("Local AP IP: " + WiFi.softAPIP().toString());
		server.on("/", handleRoot);
		//server.on("/test", handleTest);
		//server.on("/maintain", handleMaintain);
		server.begin();
		Serial.println("Web server started...");
		localApStatus = LOCALAPSTATUS_STARTED;
		break;
	case LOCALAPSTATUS_STARTED:
		server.handleClient();
		//if (WiFi.status() == wl_status_t::WL_CONNECTED)
		//	WiFi.disconnect();
		break;
	//case LOCALAPSTATUS_MAINTAIN:
	//	server.handleClient();
	//	MaintainWifi();
	//	break;
	default:
		break;
	}

}