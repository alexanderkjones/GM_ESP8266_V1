#include <Wire.h>
#include "AT24C32Helper.h"
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DS3231.h"

void LocalApLoop();		// in apconfig.ino

#define I2C_SDA 2
#define I2C_SCL 4
#define PIN_BTN	13
#define PIN_LED	5
#define SLEEP_SEC 1000000

DS3231 rtc(I2C_SDA, I2C_SCL);
AT24C32Helper eeprom = AT24C32Helper();
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
}

// the loop function runs over and over again until power down or reset
void loop()
{
	if (now() - lLastActivity > 90)
	{
		Serial.println("Sleeping for 5 seconds...");
		digitalWrite(PIN_LED, LOW);
		ESP.deepSleep(SLEEP_SEC * 5);
	}
	digitalWrite(PIN_LED, HIGH);

	LocalApLoop();

	if (digitalRead(PIN_BTN) == LOW)	// The button can reset the 90s timer as well
	{
		lLastActivity = now();
	}

	delay(1);
}
