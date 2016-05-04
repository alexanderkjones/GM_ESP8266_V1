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
		lLastActivity = 0l;
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
}

int scaleSampleWeight()
{
	int w = 0;
	scale.power_up();
	while (!scale.is_ready() && w < 1000)
	{
		delay(1);
		w++;
	}
	Serial.println("Scale is ready: " + String(w));

	int sample = scale.get_units(25);
	//Throw away last bit
	sample = (sample / 10) * 10;
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
	}
	Serial.println("Done.");
}

// the loop function runs over and over again until power down or reset
void loop()
{


	if (now() - lLastActivity > 90)
	{
		WritePacket({ now() + 0, 1 });
		WritePacket({ now() + 1, 2 });
		WritePacket({ now() + 2, 3 });
		WritePacket({ now() + 3, 4 });
		WritePacket({ now() + 4, 5 });
		int unread = GetPacketsUnread();
		if (unread < 200)
		{
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
