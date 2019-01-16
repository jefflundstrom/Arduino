// 
// 
// 
#include <TimeLib.h>
#include <Time.h>
#include <WiFiUdp.h>
#include "MNtpTime.h"

// NTP Servers:
IPAddress nptServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov


const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)

WiFiUDP _udp;
int ntpCnt = 0;
unsigned int localPort = 8888;  // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void _sendNTPpacket(IPAddress &address)
{
	//	strcpy(altBuf, "sendNTPpacket");

	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:                 
	_udp.beginPacket(address, 123); //NTP requests are to port 123
	_udp.write(packetBuffer, NTP_PACKET_SIZE);
	_udp.endPacket();
}

time_t _getNtpTime()
{
	while (_udp.parsePacket() > 0); // discard any previously received packets
	_sendNTPpacket(nptServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500)
	{
		int size = _udp.parsePacket();
		if (size >= NTP_PACKET_SIZE)
		{
			ntpCnt++;
			_udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
		}
	}
	return 0; // return 0 if unable to get the time
}

void MNtpTime::init()
{
	_udp.begin(localPort);
	setSyncProvider(_getNtpTime);
}

int MNtpTime::UpdateCnt() { return ntpCnt; }



