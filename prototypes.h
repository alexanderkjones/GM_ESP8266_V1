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

#ifndef _VSARDUINO_H_
TimeAndValue eepromDataSample(uint16 PacketIndex);
void eepromDataSample(uint16 PacketIndex, TimeAndValue Data);
void WritePacket(TimeAndValue Data);
TimeAndValue ReadPacket();
#endif
