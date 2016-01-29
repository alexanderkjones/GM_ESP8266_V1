// FramHelper.h

#ifndef _FRAMHELPER_h
#define _FRAMHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class FramHelper
{
public:
	boolean begin();
	boolean begin(uint8 pinSDA, uint8 pinSCL);

	String ReadString(int addr);
	String ReadStringExact(int addr, int buff);
	void WriteString(int addr, String val, int maxLen, bool ExcludeTermination);
	void WriteString(int addr, String val, int maxLen);
	void WriteStringExact(int addr, String val, int maxLen);
	unsigned int ReadInt(int p_address);
	void WriteInt(int address, int value);

};

#endif

