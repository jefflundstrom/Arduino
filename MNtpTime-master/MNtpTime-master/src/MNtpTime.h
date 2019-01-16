// MNtpTime.h

#ifndef _MNTPTIME_h
#define _MNTPTIME_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class MNtpTime
{
 protected:


 public:
	void init();
	int UpdateCnt();

private:

};

#endif

