#pragma once
// Apparently the Arduino default environment doesn't properly prototype things sometimes
#ifndef _timeandvalue
#define _timeandvalue
struct TimeAndValue
{
	sint32 Time;
	sint32 Value;
};
#endif // !_timeandvalue

TimeAndValue eepromDataSample(int PacketIndex);
void eepromDataSample(int PacketIndex, TimeAndValue Data);
