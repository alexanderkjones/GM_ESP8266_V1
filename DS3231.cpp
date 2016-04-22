// 
// 
// 

#include "DS3231.h"
#include <Wire\Wire.h>
#include <TimeLib.h>

#define REG_SEC		0x00
#define REG_MIN		0x01
#define REG_HOUR	0x02
#define REG_DOW		0x03
#define REG_DATE	0x04
#define REG_MON		0x05
#define REG_YEAR	0x06
#define REG_CON		0x0e
#define REG_STATUS	0x0f
#define REG_AGING	0x10
#define REG_TEMPM	0x11
#define REG_TEMPL	0x12

#define SEC_1970_TO_2000 946684800

static const uint8_t dim[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

Time::Time()
{
	this->year = 2014;
	this->month = 1;
	this->day = 1;
	this->hour = 0;
	this->min = 0;
	this->sec = 0;
	this->dow = 3;
}

DS3231::DS3231(uint8_t data_pin, uint8_t sclk_pin)
{
	_sda_pin = data_pin;
	_scl_pin = sclk_pin;
}

void DS3231::begin()
{
	Wire.begin(_sda_pin, _scl_pin);
}

// clear /EOSC bit
// Sometimes necessary to ensure that the clock
// keeps running on just battery power. Once set,
// it shouldn't need to be reset but it's a good
// idea to make sure.
void DS3231::clearEOSC()
{
	_writeRegister(REG_CON, 0b00011100);
}

void DS3231::_writeRegister(uint8_t reg, uint8_t value)
{
	Wire.beginTransmission(DS3231_ADDR);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
}

uint8_t DS3231::_readRegister(uint8_t reg)
{
	Wire.beginTransmission(DS3231_ADDR); // 0x68 is DS3231 device address
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(DS3231_ADDR, 1);

	uint8_t ret;
	while (Wire.available())
	{
		ret = Wire.read();
	}
	return ret;
}

Time DS3231::getTime()
{
	Time t = Time();
	// send request to receive data starting at register 0
	Wire.beginTransmission(DS3231_ADDR); // 0x68 is DS3231 device address
	Wire.write((byte)0); // start at register 0
	Wire.endTransmission();
	Wire.requestFrom(DS3231_ADDR, 7); // request three bytes (seconds, minutes, hours)

	if (Wire.available())
	{
		t.sec = _decode(Wire.read());
		t.min = _decode(Wire.read());
		t.hour = _decodeH(Wire.read());
		t.dow = Wire.read();
		t.day = _decode(Wire.read());
		t.month = _decode(Wire.read());
		t.year = _decodeY(Wire.read()) + 2000;
	}
	while (Wire.available())
		Wire.read();

	return t;
}

void DS3231::setTime(uint8_t hour, uint8_t min, uint8_t sec)
{
	if (((hour >= 0) && (hour < 24)) && ((min >= 0) && (min < 60)) && ((sec >= 0) && (sec < 60)))
	{
		_writeRegister(REG_HOUR, _encode(hour));
		_writeRegister(REG_MIN, _encode(min));
		_writeRegister(REG_SEC, _encode(sec));
	}
}

const String months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// sample input: date = "Dec 26 2009", time = "12:34:56"
void DS3231::setDateTimeString(String compDate, String compTime)
{
	int month = 1;
	for (int i = 1; i <= 12; i++)
	{
		if (months[i - 1] == compDate.substring(0, 3))
			month = i;
	}
	int day = compDate.substring(4, 6).toInt();
	int year = compDate.substring(7, 11).toInt();

	int hour = compTime.substring(0, 2).toInt();
	int min = compTime.substring(3, 5).toInt();
	int sec = compTime.substring(6, 8).toInt();

	setDate(year, month, day);
	setTime(hour, min, sec);
}

void DS3231::setDate(uint16_t year, uint8_t month, uint8_t day)
{
	if (((day > 0) && (day <= 31)) && ((month > 0) && (month <= 12)) && ((year >= 2000) && (year < 3000)))
	{
		year -= 2000;
		_writeRegister(REG_YEAR, _encode(year));
		_writeRegister(REG_MON, _encode(month));
		_writeRegister(REG_DATE, _encode(day));
	}
}
// Untested automatic calculation
void DS3231::setDOW()
{
	int dow;
	byte mArr[12] = { 6,2,2,5,0,3,5,1,4,6,2,4 };
	Time _t = getTime();

	dow = (_t.year % 100);
	dow = dow*1.25;
	dow += _t.day;
	dow += mArr[_t.month - 1];
	if (((_t.year % 4) == 0) && (_t.month < 3))
		dow -= 1;
	while (dow > 7)
		dow -= 7;
	_writeRegister(REG_DOW, dow);
}
// Set Day of the Week.  1-7 inclusive.
void DS3231::setDOW(uint8_t dow)
{
	if ((dow > 0) && (dow < 8))
		_writeRegister(REG_DOW, dow);
}

String DS3231::getTimeStr(uint8_t format)
{
	String output = "xxxxxxxx";
	Time t;
	t = getTime();
	if (t.hour < 10)
		output[0] = 48;
	else
		output[0] = char((t.hour / 10) + 48);
	output[1] = char((t.hour % 10) + 48);
	output[2] = 58;
	if (t.min < 10)
		output[3] = 48;
	else
		output[3] = char((t.min / 10) + 48);
	output[4] = char((t.min % 10) + 48);
	output[5] = 58;
	if (format == FORMAT_SHORT)
		output[5] = 0;
	else
	{
		if (t.sec < 10)
			output[6] = 48;
		else
			output[6] = char((t.sec / 10) + 48);
		output[7] = char((t.sec % 10) + 48);
		output[8] = 0;
	}
	return output;
}

String DS3231::getDateStr(uint8_t slformat, uint8_t eformat, char divider)
{
	String output = "xxxxxxxxxx";
	int yr, offset;
	Time t;
	t = getTime();
	switch (eformat)
	{
	case FORMAT_LITTLEENDIAN:
		if (t.day < 10)
			output[0] = 48;
		else
			output[0] = char((t.day / 10) + 48);
		output[1] = char((t.day % 10) + 48);
		output[2] = divider;
		if (t.month < 10)
			output[3] = 48;
		else
			output[3] = char((t.month / 10) + 48);
		output[4] = char((t.month % 10) + 48);
		output[5] = divider;
		if (slformat == FORMAT_SHORT)
		{
			yr = t.year - 2000;
			if (yr < 10)
				output[6] = 48;
			else
				output[6] = char((yr / 10) + 48);
			output[7] = char((yr % 10) + 48);
			output[8] = 0;
		}
		else
		{
			yr = t.year;
			output[6] = char((yr / 1000) + 48);
			output[7] = char(((yr % 1000) / 100) + 48);
			output[8] = char(((yr % 100) / 10) + 48);
			output[9] = char((yr % 10) + 48);
			output[10] = 0;
		}
		break;
	case FORMAT_BIGENDIAN:
		if (slformat == FORMAT_SHORT)
			offset = 0;
		else
			offset = 2;
		if (slformat == FORMAT_SHORT)
		{
			yr = t.year - 2000;
			if (yr < 10)
				output[0] = 48;
			else
				output[0] = char((yr / 10) + 48);
			output[1] = char((yr % 10) + 48);
			output[2] = divider;
		}
		else
		{
			yr = t.year;
			output[0] = char((yr / 1000) + 48);
			output[1] = char(((yr % 1000) / 100) + 48);
			output[2] = char(((yr % 100) / 10) + 48);
			output[3] = char((yr % 10) + 48);
			output[4] = divider;
		}
		if (t.month < 10)
			output[3 + offset] = 48;
		else
			output[3 + offset] = char((t.month / 10) + 48);
		output[4 + offset] = char((t.month % 10) + 48);
		output[5 + offset] = divider;
		if (t.day < 10)
			output[6 + offset] = 48;
		else
			output[6 + offset] = char((t.day / 10) + 48);
		output[7 + offset] = char((t.day % 10) + 48);
		output[8 + offset] = 0;
		break;
	case FORMAT_MIDDLEENDIAN:
		if (t.month < 10)
			output[0] = 48;
		else
			output[0] = char((t.month / 10) + 48);
		output[1] = char((t.month % 10) + 48);
		output[2] = divider;
		if (t.day < 10)
			output[3] = 48;
		else
			output[3] = char((t.day / 10) + 48);
		output[4] = char((t.day % 10) + 48);
		output[5] = divider;
		if (slformat == FORMAT_SHORT)
		{
			yr = t.year - 2000;
			if (yr < 10)
				output[6] = 48;
			else
				output[6] = char((yr / 10) + 48);
			output[7] = char((yr % 10) + 48);
			output[8] = 0;
		}
		else
		{
			yr = t.year;
			output[6] = char((yr / 1000) + 48);
			output[7] = char(((yr % 1000) / 100) + 48);
			output[8] = char(((yr % 100) / 10) + 48);
			output[9] = char((yr % 10) + 48);
			output[10] = 0;
		}
		break;
	}
	return output;
}

String DS3231::getDOWStr(uint8_t format)
{
	String output = "xxxxxxxxxx";
	String daysLong[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
	String daysShort[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	Time t;
	t = getTime();
	if (format == FORMAT_SHORT)
		output = daysShort[t.dow - 1];
	else
		output = daysLong[t.dow - 1];
	return output;
}

String DS3231::getMonthStr(uint8_t format)
{
	String output = "xxxxxxxxx";
	String monthLong[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	String monthShort[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	Time t;
	t = getTime();
	if (format == FORMAT_SHORT)
		output = monthShort[t.month - 1];
	else
		output = monthLong[t.month - 1];
	return output;
}

long DS3231::getUnixTime(Time t)
{
	uint16_t	dc;

	dc = t.day;
	for (uint8_t i = 0; i < (t.month - 1); i++)
		dc += dim[i];
	if ((t.month > 2) && (((t.year - 2000) % 4) == 0))
		++dc;
	dc = dc + (365 * (t.year - 2000)) + (((t.year - 2000) + 3) / 4) - 1;

	return ((((((dc * 24L) + t.hour) * 60) + t.min) * 60) + t.sec) + SEC_1970_TO_2000;

}

Time DS3231::setUnixTime(long unixEpoch)
{
	Time t = Time();
	t.year = year(unixEpoch);
	t.month = month(unixEpoch);
	t.day = day(unixEpoch);
	t.hour = hour(unixEpoch);
	t.min = minute(unixEpoch);
	t.sec = second(unixEpoch);

	return t;
}

uint8_t	DS3231::_decode(uint8_t value)
{
	uint8_t decoded = value & 127;
	decoded = (decoded & 15) + 10 * ((decoded & (15 << 4)) >> 4);
	return decoded;
}

uint8_t DS3231::_decodeH(uint8_t value)
{
	if (value & 128)
		value = (value & 15) + (12 * ((value & 32) >> 5));
	else
		value = (value & 15) + (10 * ((value & 48) >> 4));
	return value;
}

uint8_t	DS3231::_decodeY(uint8_t value)
{
	uint8_t decoded = ((value & 0xF0) >> 4) * 10 + (value & 0x0F);
	return decoded;
}

uint8_t DS3231::_encode(uint8_t value)
{
	uint8_t encoded = ((value / 10) << 4) + (value % 10);
	return encoded;
}
