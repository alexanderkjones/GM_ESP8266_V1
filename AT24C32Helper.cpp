#define EEPROM_LEN	4096
#define EEPROM_ADDR	0x57

#include "AT24C32Helper.h"
#include <Wire.h>


boolean AT24C32Helper::begin()
{
	Wire.begin();
	deviceAddr = EEPROM_ADDR;
}

boolean AT24C32Helper::begin(uint8 pinSDA, uint8 pinSCL)
{
	Wire.begin(pinSDA, pinSCL);
	deviceAddr = EEPROM_ADDR;
}

boolean AT24C32Helper::begin(int eepromAddr)
{
	Wire.begin();
	deviceAddr = eepromAddr;
}

boolean AT24C32Helper::begin(int eepromAddr, uint8 pinSDA, uint8 pinSCL)
{
	Wire.begin(pinSDA, pinSCL);
	deviceAddr = eepromAddr;
}



// Read a string from an address until terminated (NUL)
String AT24C32Helper::ReadString(int addr)
{
	String str = "";
	for (int i = addr; i < EEPROM_LEN; i++)
	{
		byte val = i2c_eeprom_read_byte(deviceAddr, i);
		if (val == 0x00 || val == 0xFF)
			break;
		str += char(val);
	}
	return str;
}
// Read a string from an address with an exact length
String AT24C32Helper::ReadStringExact(int addr, int len) {

	String str = "";

	for (int i = addr; i < addr + len; ++i)
	{
		byte val = i2c_eeprom_read_byte(deviceAddr, i);

		if ((0 < val) && (val < 255)) {
			str += char(val);
		}
	}
	return str;

}
// Write a string to an address, truncating at a maximum length, with optional termination (NUL)
void AT24C32Helper::WriteString(int addr, String val, int maxLen, bool ExcludeTermination) {

	for (int i = 0; (i < val.length()) && (i < ExcludeTermination ? maxLen : maxLen - 1); ++i)
	{
		i2c_eeprom_write_byte(deviceAddr, addr + i, val[i]);
		delay(2);	// Seems like the data can be corrupted if the EEPROM is taking longer than usual for its write cycle
	}
	if (!ExcludeTermination)
	i2c_eeprom_write_byte(deviceAddr, addr + ((val.length() >= maxLen) ? maxLen - 1 : val.length()), 0x00);
	delay(2);
}
// Write a string to an address, truncating at a maximum length(-1) and with a terminator (NUL)
void AT24C32Helper::WriteString(int addr, String val, int maxLen) {
	WriteString(addr, val, maxLen, false);
}
// Write a string to an address, with an exact length.  If the maximum length is not used, garbage will fill the rest.
void AT24C32Helper::WriteStringExact(int addr, String val, int maxLen) {
	WriteString(addr, val, maxLen, true);
}


// Read two bytes at an address as INT16
sint16 AT24C32Helper::ReadInt16(int addr)
{
	byte lowByte = i2c_eeprom_read_byte(deviceAddr, addr);
	byte highByte = i2c_eeprom_read_byte(deviceAddr, addr+1);

	return (sint16)(((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00));
}
// Write two bytes at an address from an INT16
void AT24C32Helper::WriteInt16(int addr, sint16 value)
{
	byte lowByte = ((value >> 0) & 0xFF);
	byte highByte = ((value >> 8) & 0xFF);

	i2c_eeprom_write_byte(deviceAddr, addr, lowByte);
	delay(2);	// Seems like the data can be corrupted if the EEPROM is taking longer than usual for its write cycle
	i2c_eeprom_write_byte(deviceAddr, addr + 1, highByte);
	delay(2);
}

sint32 AT24C32Helper::ReadInt32(int addr)
{
	byte b0 = i2c_eeprom_read_byte(deviceAddr, addr);
	byte b1 = i2c_eeprom_read_byte(deviceAddr, addr + 1);
	byte b2 = i2c_eeprom_read_byte(deviceAddr, addr + 2);
	byte b3 = i2c_eeprom_read_byte(deviceAddr, addr + 3);

	return ((b0 << 0) & 0xFF) + ((b1 << 8) & 0xFF00) + ((b2 << 16) & 0xFF0000) + ((b3 << 24) & 0xFF000000);
}

void AT24C32Helper::WriteInt32(int addr, sint32 value)
{
	byte b0 = ((value >> 0) & 0xFF);
	byte b1 = ((value >> 8) & 0xFF);
	byte b2 = ((value >> 16) & 0xFF);
	byte b3 = ((value >> 24) & 0xFF);

	i2c_eeprom_write_byte(deviceAddr, addr, b0);
	delay(2);	// Seems like the data can be corrupted if the EEPROM is taking longer than usual for its write cycle
	i2c_eeprom_write_byte(deviceAddr, addr + 1, b1);
	delay(2);
	i2c_eeprom_write_byte(deviceAddr, addr + 2, b2);
	delay(2);
	i2c_eeprom_write_byte(deviceAddr, addr + 3, b3);
	delay(2);
}

//* From Sample Code */

void AT24C32Helper::i2c_eeprom_write_byte(int deviceaddress, unsigned int eeaddress, byte data) {
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void AT24C32Helper::i2c_eeprom_write_page(int deviceaddress, unsigned int eeaddresspage, byte* data, byte length) {
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddresspage >> 8)); // MSB
	Wire.write((int)(eeaddresspage & 0xFF)); // LSB
	byte c;
	for (c = 0; c < length; c++)
		Wire.write(data[c]);
	Wire.endTransmission();
}

byte AT24C32Helper::i2c_eeprom_read_byte(int deviceaddress, unsigned int eeaddress) {
	byte rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, 1);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void AT24C32Helper::i2c_eeprom_read_buffer(int deviceaddress, unsigned int eeaddress, byte *buffer, int length) {
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, length);
	int c = 0;
	for (c = 0; c < length; c++)
		if (Wire.available()) buffer[c] = Wire.read();
}