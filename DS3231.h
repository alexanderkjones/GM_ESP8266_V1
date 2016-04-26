// DS3231.h

#ifndef _DS3231_h
#define _DS3231_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define DS3231_ADDR		0x68

#define FORMAT_SHORT	1
#define FORMAT_LONG		2

#define FORMAT_LITTLEENDIAN	1
#define FORMAT_BIGENDIAN	2
#define FORMAT_MIDDLEENDIAN	3

#define MONDAY		1
#define TUESDAY		2
#define WEDNESDAY	3
#define THURSDAY	4
#define FRIDAY		5
#define SATURDAY	6
#define SUNDAY		7


class Time
{
public:
	uint8_t		hour;
	uint8_t		min;
	uint8_t		sec;
	uint8_t		day;
	uint8_t		month;
	uint16_t	year;
	uint8_t		dow;

	Time();
};

class DS3231
{
public:
	DS3231(uint8_t data_pin, uint8_t sclk_pin);
	void	begin();
	void	clearEOSC();
	Time	getTime();
	void	setTime(uint8_t hour, uint8_t min, uint8_t sec);
	void	setDateTime(String compDate, String compTime);
	void	setDate(uint16_t year, uint8_t month, uint8_t day);
	void	setDOW();
	void	setDOW(uint8_t dow);

	String	getTimeStr(uint8_t format = FORMAT_LONG);
	String	getTimeStr(Time t, uint8_t format = FORMAT_LONG);
	String	getDateStr(uint8_t slformat = FORMAT_LONG, uint8_t eformat = FORMAT_BIGENDIAN, char divider = '-');
	String	getDateStr(Time t, uint8_t slformat = FORMAT_LONG, uint8_t eformat = FORMAT_BIGENDIAN, char divider = '-');
	String	getDOWStr(uint8_t format = FORMAT_LONG);
	String	getMonthStr(uint8_t format = FORMAT_LONG);
	long	getUnixTime(Time t);
	Time	getTime(long unixEpoch);
	Time	getTime(String compDate, String compTime);


private:
	uint8_t _scl_pin;
	uint8_t _sda_pin;
	uint8_t	_decode(uint8_t value);
	uint8_t	_decodeH(uint8_t value);
	uint8_t	_decodeY(uint8_t value);
	uint8_t	_encode(uint8_t vaule);
	void 	_writeRegister(uint8_t reg, uint8_t value);
	uint8_t	_readRegister(uint8_t reg);

};
#endif