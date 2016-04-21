#include <Adafruit_FRAM_I2C.h>	// https://github.com/adafruit/Adafruit_FRAM_I2C/archive/master.zip
#include "FramHelper.h"
#include "DS3231.h"

#define I2C_SDA 2
#define I2C_SCL 4

DS3231 rtc(I2C_SDA, I2C_SCL);
FramHelper fram;
boolean framSetup = 0;
int boots;

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(9600);


	rtc.begin();
	rtc.clearEOSC();
	//rtc.setDate(2001, 1, 1);	// Jan 01 2001
	//rtc.setTime(12, 34, 56);	// 12:34:56 PM
	rtc.setDateTimeString(__DATE__, __TIME__);	// Set to when the compiler created the binary


	framSetup = fram.begin(I2C_SDA, I2C_SCL);

	boots = fram.ReadInt(0);
	fram.WriteInt(0, ++boots);
}

Time t;
int secsAlive;
// the loop function runs over and over again until power down or reset
void loop() 
{
	delay(5000);

	// 2016-01-28 | 20:32:56
	Serial.printf("%s | %s\n", rtc.getDateStr().c_str(), rtc.getTimeStr().c_str());
	t = rtc.getTime();
	// 01/28/2016 | 20:32:56
	Serial.printf("%#02d/%#02d/%#02d | %#02d:%#02d:%#02d\n", t.mon, t.date, t.year, t.hour, t.min, t.sec);
	// 1454013176
	Serial.printf("%ld\n", rtc.getUnixTime(t));


	Serial.printf("FRAM setup: %d\n", framSetup);
	if (framSetup)
	{
		secsAlive = fram.ReadInt(2);
		secsAlive += 5;
		fram.WriteInt(2, secsAlive);
		Serial.printf("Boots: %d, seconds alive: %d\n\n", boots, secsAlive);
	}
}
