#include <ESP8266HTTPClient.h>
#include <HX711.h>
#include <Wire.h>
#include "AT24C32Helper.h"
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DS3231.h"
#include "prototypes.h"

#define I2C_SDA 2
#define I2C_SCL 4
#define PIN_BTN	13
#define PIN_LED	5
#define HX_DOUT	14
#define HX_SCK	12
#define SLEEP_SEC 1000000
#define DATA_HOST	"api.pushingbox.com"
#define DATA_DEVID	"v8C0805331DE2F9D";

DS3231 rtc(I2C_SDA, I2C_SCL);
AT24C32Helper eeprom = AT24C32Helper();
HX711 scale(HX_DOUT, HX_SCK);
sint32 lLastActivity;

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(9600);

	rtc.begin();
	//rtc.clearEOSC();
	sint32 tRtc = rtc.getUnixTime(rtc.getTime());
	sint32 tComp = rtc.getUnixTime(rtc.getTime(__DATE__, __TIME__));
	if (tComp > tRtc)
	{
		rtc.setDateTime(__DATE__, __TIME__);	// Set to when the compiler created the binary
	}

	setTime(rtc.getUnixTime(rtc.getTime()));
	if (timeStatus() != timeSet)
		Serial.println("Unable to sync with the RTC");

	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_BTN, INPUT_PULLUP);

	if (digitalRead(PIN_BTN))
	{
		lLastActivity = now() - 91;
	}
	else
	{
		lLastActivity = now();
	}
	Serial.printf("\n%s | %s\n", rtc.getDateStr().c_str(), rtc.getTimeStr().c_str());
	eeprom.begin(I2C_SDA, I2C_SCL);

	if (eepromReadPointer() == -1)
		eepromReadPointer(0);
	if (eepromWritePointer() == -1)
		eepromWritePointer(0);

	scale.set_scale(eepromScaleFactor());
	scale.set_offset(eepromZeroCal());
}

void SetHx711Zero()
{
	scale.set_scale();
	scale.tare();

	long zeroFactor = scale.read_average();
	eepromZeroCal(zeroFactor);
}

int SetHx711Scale(int CalWeight)
{
	int steps = 0;
	float calScale = -10.0f;
	scale.set_scale(calScale);
	eepromScaleFactor(calScale);
	float weightRead = scale.get_units();
	while (weightRead > CalWeight && steps++ < 100)
	{
		calScale -= 10.0f;
		if (calScale > 0.01f || calScale < -0.01f)
			scale.set_scale(calScale);
		weightRead = scale.get_units();
	}
	while (weightRead < CalWeight && steps++ < 100)
	{
		calScale += 1.0f;
		if (calScale > 0.01f || calScale < -0.01f)
			scale.set_scale(calScale);
		weightRead = scale.get_units(2);
	}
	while (weightRead > CalWeight && steps++ < 100)
	{
		calScale -= 0.1f;
		if (calScale > 0.01f || calScale < -0.01f)
			scale.set_scale(calScale);
		weightRead = scale.get_units(4);
	}
	while (weightRead < CalWeight && steps++ < 100)
	{
		calScale += 0.01f;
		if (calScale > 0.01f || calScale < -0.01f)
			scale.set_scale(calScale);
		weightRead = scale.get_units(8);
	}

	if (steps < 100)
	{
		eepromScaleFactor(calScale);
		return 1;
	}
	return -1;
}

float scaleSampleWeight()
{
	int w = 0;
	scale.power_up();
	while (!scale.is_ready() && w < 1000)
	{
		delay(1);
		w++;
	}
	float sample = scale.get_units(25);

	scale.power_down();
	return sample;
}
int GetPacketsUnread()
{
	sint16 rPointer = eepromReadPointer();
	sint16 wPointer = eepromWritePointer();
	if (rPointer > wPointer)
	{
		wPointer += 384;
	}
	return wPointer - rPointer;
}
void WritePacket(TimeAndValue Data)
{
	sint16 wPointer = eepromWritePointer();
	eepromDataSample(wPointer, Data);

	sint16 rPointer = eepromReadPointer();
	wPointer++;
	if (wPointer >= 384)
		wPointer = 0;
	if (wPointer == rPointer)
	{
		if (++rPointer >= 384)
			rPointer = 0;
		eepromReadPointer(rPointer);
	}
	eepromWritePointer(wPointer);
}
TimeAndValue ReadPacket()
{
	sint16 rPointer = eepromReadPointer();
	sint16 wPointer = eepromWritePointer();
	if (rPointer > wPointer)
		wPointer += 384;
	if (wPointer - rPointer > 0)
	{
		TimeAndValue data = eepromDataSample(rPointer);
		if (++rPointer >= 384)
			rPointer = 0;
		eepromReadPointer(rPointer);
		return data;
	}
	return{ 0, 0 };
}

int SendData(TimeAndValue Data)
{
	String url = "/pushingbox";
	url += "?devid=" DATA_DEVID;
	url += "&delta=" + String(Data.Value);
	url += "&scaleid=" + WiFi.macAddress();

	HTTPClient client;
	Serial.print("HTTP begin... ");
	client.begin(DATA_HOST, 80, url);
	Serial.print("get... ");
	int httpStatus = client.GET();
	Serial.println(String(httpStatus) + "... ");

	if (httpStatus > 0)
	{
		String response = client.getString();
		Serial.println("=======================");
		Serial.println(response);
		Serial.println("=======================");
	}
	else
	{
		Serial.printf("httpStatus is negative: %d - %s\n", httpStatus, client.errorToString(httpStatus).c_str());
	}
	return httpStatus;
}

void ExportPackets()
{
	int unread = GetPacketsUnread();
	if (unread == 0)
		return;
	Serial.printf("Exporting %d packets....\n", unread);
	for (int i = 0; i < unread; i++)
	{
		TimeAndValue data = ReadPacket();
		Serial.printf("%s %s = %d\n", rtc.getDateStr(rtc.getTime(data.Time)).c_str(), rtc.getTimeStr(rtc.getTime(data.Time)).c_str(), data.Value);
		SendData(data);
	}
	Serial.println("Done.");
}

void TrackPour(int Delta)
{
	WritePacket({ now(), Delta });
	Serial.print("TrackPour: ");
	Serial.println(Delta);
}

void UnSleepLoop()
{
	int thisRead = (int)scaleSampleWeight() / 10 * 10;	// truncate last digit (10s of grams)
	int setPoint0 = eepromSetPoint0();
	int setPoint1 = eepromSetPoint1();

	if (thisRead != setPoint0)
	{
		if (thisRead == setPoint1)
		{
			int delta = setPoint1 - setPoint0;

			if (abs(delta) > 10)
			{
				TrackPour(delta);
			}
			eepromSetPoint0((sint16)setPoint1);
		}
		else
		{
			eepromSetPoint1((sint16)thisRead);
		}
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{
	if (now() - lLastActivity > 90)
	{
		int unread = GetPacketsUnread();
		if (unread < 1)	// If unread packets exceed X, start up WiFi and export
		{
			// Code that runs every 5 seconds at most
			UnSleepLoop();
			Serial.println("Sleeping for 5 seconds...");
			digitalWrite(PIN_LED, LOW);
			ESP.deepSleep(SLEEP_SEC * 5);
		}
	}
	digitalWrite(PIN_LED, HIGH);

	LocalApLoop();

	if (digitalRead(PIN_BTN) == LOW)	// The button can reset the 90s timer as well
	{
		lLastActivity = now();
	}

	delay(1);
}
