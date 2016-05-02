#define REMOTEAPSTATUS_INIT				0
#define REMOTEAPSTATUS_DONTCONNECT		1
#define REMOTEAPSTATUS_CONNECTEDONCE	2
#define REMOTEAPSUCCESS_FAILED		-1
#define REMOTEAPSUCCESS_INIT		0
#define REMOTEAPSUCCESS_GOOD		1
int remoteApStatus = REMOTEAPSTATUS_INIT;
int remoteApSuccess = REMOTEAPSUCCESS_INIT;

#define LOCALAPSTATUS_INIT		0
#define LOCALAPSTATUS_STARTED	1
#define LOCALAPSTATUS_MAINTAIN	2
int localApStatus = LOCALAPSTATUS_INIT;

const String localSsid = "ESP_01560E";
const String localPass = "Esp8266_01560E";

ESP8266WebServer webServer(80);

void handleRoot()
{
	setTime(rtc.getUnixTime(rtc.getTime()));
	lLastActivity = now();

	if (webServer.hasArg("SSID") && webServer.hasArg("PASS"))
	{
		String content = "";
		if (webServer.arg("SSID") == "")
		{
			eepromAutoConnect("0");
			content += "<html><head><meta http-equiv='refresh' content='1'> </head><body>";
			content += "<p>Please wait...</p>";
			content += "</body></html>";

			webServer.send(200, ((String)"text/html").c_str(), String(content));
			return;
		}

		eepromAutoConnect("1");
		eepromSsid(webServer.arg("SSID"));
		eepromPass(webServer.arg("PASS"));
		content += "<html><head><meta http-equiv='refresh' content='30'> </head><body>";
		content += "<p>Connecting to SSID: " + eepromSsid() + ". <br>Please wait...</p>";
		content += "</body></html>";

		webServer.send(200, ((String)"text/html").c_str(), String(content));
		WiFi.disconnect();
		remoteApStatus = REMOTEAPSTATUS_INIT;
		remoteApSuccess = REMOTEAPSUCCESS_INIT;
		return;
	}


	Time t = rtc.getTime(eepromLastApConnectTime());
	short rssi = eepromLastApRssi();
	String rssiQuality = "Poor";
	if (rssi > -80)
		rssiQuality = "Fair";
	if (rssi > -65)
		rssiQuality = "Good";
	if (rssi > -50)
		rssiQuality = "Excellent";

	String content = "<h1>Your Wifi Settings</h1>";

	if (eepromAutoConnect() == "1")
	{
		if (remoteApSuccess == REMOTEAPSUCCESS_GOOD)
		{
			content += "<p>You are currently configured to connect to <b>" + eepromSsid() + "</b></p>";
			content += "<p>Your last successful connection was at " + rtc.getDateStr(t) + " " + rtc.getTimeStr(t) + "</p>";
			content += "<p>Your router's connection strength was " + rssiQuality + " (" + String(rssi) + " dBm)</p>";
		}
		else
		{
			content += "<p>You are currently configured to connect to <b>" + eepromSsid() + "</b></p>";
			content += "<p>Your device has not connected to since <span style='color:#D8000C; font-weight:bold;'> " + rtc.getDateStr(t) + " " + rtc.getTimeStr(t) + "</span></p>";
			content += "<p>Your router's connection strength was " + rssiQuality + " (" + String(rssi) + " dBm)</p>";
		}
	}
	else
	{
		content += "<p>You are not currently configured to connect to a remote WiFi Access Point</p>";
	}

	content += "<h2>To Reconfigure, Enter a new Wifi Router Name and Password<h2>";
	content += "<form action='/' method='POST'>Router Name: <br><input type='text' name='SSID'><br>";
	content += "Password: <br><input type='text' name='PASS'><br>";
	content += "<input type = 'submit' value='Save'></form>";

	webServer.send(200, ((String)"text/html").c_str(), String(content));
}

void LocalApLoop()
{
	switch (localApStatus)
	{
	case LOCALAPSTATUS_INIT:
		Serial.println("Starting local AP: " + localSsid + "...");
		WiFi.softAP(localSsid.c_str(), localPass.c_str());
		Serial.println("Local AP IP: " + WiFi.softAPIP().toString());
		webServer.on("/", handleRoot);
		webServer.begin();
		Serial.println("Web server started...");
		localApStatus = LOCALAPSTATUS_STARTED;
		remoteApSuccess = REMOTEAPSUCCESS_INIT;
		if (eepromAutoConnect() != "1")
			remoteApSuccess = REMOTEAPSUCCESS_FAILED;
		break;
	case LOCALAPSTATUS_STARTED:
		webServer.handleClient();
		if (remoteApSuccess == REMOTEAPSUCCESS_INIT)
			MaintainWifi();
		if (remoteApSuccess > REMOTEAPSUCCESS_INIT && WiFi.status() == wl_status_t::WL_CONNECTED)
		{
			// Presumably do some internet work first
			Serial.println("Disconnecting...");
			WiFi.disconnect();
		}

		break;
		//case LOCALAPSTATUS_MAINTAIN:
		//	webServer.handleClient();
		//	MaintainWifi();
		//	break;
	default:
		break;
	}

}

void MaintainWifi()
{
	wl_status_t wlStatus = WiFi.status();
	String ssid, pass;
	switch (remoteApStatus)
	{
	case REMOTEAPSTATUS_INIT:
		switch (wlStatus)
		{
		case wl_status_t::WL_CONNECTION_LOST:
		case wl_status_t::WL_DISCONNECTED:
		case wl_status_t::WL_IDLE_STATUS:
		case wl_status_t::WL_NO_SSID_AVAIL:
			Serial.println("Connecting to SSID: " + eepromSsid() + "...");
			ssid = eepromSsid();
			pass = eepromPass();
			WiFi.begin(ssid.c_str(), pass.c_str());
			for (int i = 0; i < 10; i++)
			{
				delay(1000);
				webServer.handleClient();
				Serial.println(" -");
				if (WiFi.status() != wl_status_t::WL_DISCONNECTED)
					break;
			}
			break;
		case wl_status_t::WL_CONNECT_FAILED:
			Serial.println("WL CONN FAIL");
			remoteApStatus = REMOTEAPSTATUS_DONTCONNECT;
			remoteApSuccess = REMOTEAPSUCCESS_FAILED;
			break;
		case wl_status_t::WL_CONNECTED:
			Serial.print("Connected; WAN IP: ");
			Serial.println(WiFi.localIP());
			remoteApStatus = REMOTEAPSTATUS_CONNECTEDONCE;
			remoteApSuccess = REMOTEAPSUCCESS_GOOD;
			eepromLastApConnectTime(now());
			eepromLastApRssi(WiFi.RSSI());
			break;
		default:
			break;
		}
		break;
	case REMOTEAPSTATUS_CONNECTEDONCE:
		switch (wlStatus)
		{
		case wl_status_t::WL_CONNECTION_LOST:
		case wl_status_t::WL_DISCONNECTED:
		case wl_status_t::WL_IDLE_STATUS:
		case wl_status_t::WL_NO_SSID_AVAIL:
			Serial.println("Lost connection to AP...");
			remoteApStatus = REMOTEAPSTATUS_INIT;
			break;
		default:
			break;
		}
		break;
	case REMOTEAPSTATUS_DONTCONNECT:
		break;
	}
}