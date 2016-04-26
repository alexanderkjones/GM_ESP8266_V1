// 
// 
// 
#define FRAM_LENGTH 262144
#define FRAM_ADDR 0x50
#include "FramHelper.h"
#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>	// https://github.com/adafruit/Adafruit_FRAM_I2C/archive/master.zip

Adafruit_FRAM_I2C _fram = Adafruit_FRAM_I2C();

boolean FramHelper::begin()
{
	Wire.begin();
	_fram.begin(FRAM_ADDR);
}

boolean FramHelper::begin(uint8 pinSDA, uint8 pinSCL)
{
	Wire.begin(pinSDA, pinSCL);
	_fram.begin(FRAM_ADDR);
}


// Read a string from an address until terminated (NUL)
String FramHelper::ReadString(int addr)
{
	String str = "";
	for (int i = addr; i < FRAM_LENGTH; i++)
	{
		byte val = _fram.read8(i);
		if (val == 0x00 || val == 0xFF)
			break;
		str += char(val);
	}
	return str;
}
// Read a string from an address with an exact length
String FramHelper::ReadStringExact(int addr, int buff) {

	String str = "";

	for (int i = addr; i < addr + buff; ++i)
	{
		byte val = _fram.read8(i);

		if ((0 < val) && (val < 255)) {
			str += char(val);
		}
	}
	return str;

}
// Write a string to an address, truncating at a maximum length, with optional termination (NUL)
void FramHelper::WriteString(int addr, String val, int maxLen, bool ExcludeTermination) {

	for (int i = 0; (i < val.length()) && (i < ExcludeTermination ? maxLen : maxLen - 1); ++i)
	{
		_fram.write8(addr + i, val[i]);
	}
	if (!ExcludeTermination)
		_fram.write8(addr + ((val.length() >= maxLen) ? maxLen - 1 : val.length()), 0x00);
}
// Write a string to an address, truncating at a maximum length(-1) and with a terminator (NUL)
void FramHelper::WriteString(int addr, String val, int maxLen) {
	WriteString(addr, val, maxLen, false);
}
// Write a string to an address, with an exact length.  If the maximum length is not used, garbage will fill the rest.
void FramHelper::WriteStringExact(int addr, String val, int maxLen) {
	WriteString(addr, val, maxLen, true);
}


// Read two bytes at an address as INT16
unsigned int FramHelper::ReadInt(int p_address)
{
	byte lowByte = _fram.read8(p_address);
	byte highByte = _fram.read8(p_address + 1);

	return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}
// Write two bytes at an address from an INT16
void FramHelper::WriteInt(int address, int value)
{
	byte lowByte = ((value >> 0) & 0xFF);
	byte highByte = ((value >> 8) & 0xFF);

	_fram.write8(address, lowByte);
	_fram.write8(address + 1, highByte);
}