//////////////////////////////////////////////
//				EEPROM Storage
//	ADDR	LEN		DESCRIPTION

//	0		1		WiFi client auto-connect
String eepromAutoConnect() { return eeprom.ReadStringExact(0, 1); }
void eepromAutoConnect(String val) { eeprom.WriteStringExact(0, val, 1); }
//	1		32+1	Remote SSID
String eepromSsid() { return eeprom.ReadString(1); }
void eepromSsid(String Ssid) { eeprom.WriteString(1, Ssid, 33); }
//	34		64+1	Remote password
String eepromPass() { return eeprom.ReadString(34); }
void eepromPass(String Pass) { eeprom.WriteString(34, Pass, 65); }
//	99		4		Last Remote AP connection
sint32 eepromLastApConnectTime() { return eeprom.ReadInt32(99); }
void eepromLastApConnectTime(sint32 UnixTime) { eeprom.WriteInt32(99, UnixTime); }
//	103		2		Last Remote AP RSSI
sint16 eepromLastApRssi() { return eeprom.ReadInt16(103); }
void eepromLastApRssi(sint16 Rssi) { eeprom.WriteInt16(103, Rssi); }
//	105

// 1020		2		Data sample Read pointer
// 1022		2		Data sample Write pointer
// 1024		3072	384 Data samples; 4 byte timestamp, 4 byte value
//////////////////////////////////////////////

