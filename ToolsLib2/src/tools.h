#ifndef tools_h
#define tools_h
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "Arduino.h"


// serial output
//#define DPRINT_ON   ...debugging level
#define MPRINT_ON   ...monitoring level
//#deine  XPRINT_ON   ...application messages / errors


#define PRINTFREE  Serial.print(F("free sram : "));Serial.println(freeRam())
#define FS(x) (__FlashStringHelper*)(x)

#define XPRINT(x)   		Serial.print((x))
#define XPRINTHEX(x)   		Serial.print((x),HEX)
#define XPRINTLN(x) 		Serial.println((x))
#define XPRINTS(s)   		XPRINT(F(s))
#define XPRINTSS(s)   		XPRINT(FS(s))		// print   const progmem char[]
#define XPRINTLNS(s)   	 	XPRINTLN(F(s))
#define XPRINTLNSS(s)   	XPRINTLN(FS(s))  	// println const progmem char[]
#define XPRINTSVAL(s,x)  	XPRINTS(s);XPRINT(x)
#define XPRINTLNSVAL(s,x)  	XPRINTS(s);XPRINTLN(x)
#define XPRINTSVALS(s,x,z)  XPRINTS(s);XPRINT(x);XPRINTS(z)
#define XPRINTLNSVALS(s,x,z) XPRINTSVALS(s,x,z);XPRINTLNS("");
#define XPRINTFREE  XPRINTLNSVAL("free sram : " ,(freeRam()))

#define XTIME_RESET timeStampId = 0;timeStampLastPick = millis()
#define XTIME_PICK  ++timeStampId;timeDuration = millis() - timeStampLastPick;\
	               XPRINT(timeStampId);XPRINTS(" T= ");XPRINTLN(timeDuration);\
				   timeStampLastPick = millis();

#ifdef MPRINT_ON

	#define MPRINT(x)   	XPRINT(x)
	#define MPRINTHEX(x)   	XPRINTHEX(x)
	#define MPRINTLN(x) 	XPRINTLN(x)
	#define MPRINTS(s)   	XPRINTS(s)
	#define MPRINTSS(s)   	XPRINTSS(s)
	#define MPRINTLNS(s)    XPRINTLNS(s)
	#define MPRINTLNSS(s)   XPRINTLNSS(s)
	#define MPRINTSVAL(s,x)    XPRINTSVAL(s,x)
    #define MPRINTLNSVAL(s,x)  XPRINTLNSVAL(s,x)
	#define MPRINTSVALS(s,x,z) XPRINTSVALS(s,x,z)
	#define MPRINTLNSVALS(s,x,z) XPRINTLNSVALS(s,x,z)
	#define MPRINTFREE   	XPRINTFREE
	#define MTIME_RESET  	XTIME_RESET
	#define MTIME_PICK  	XTIME_PICK
#else
    #define MPRINT(x)
	#define MPRINTHEX(x)
	#define MPRINTLN(x)
	#define MPRINTS(s)
	#define MPRINTSS(s)
	#define MPRINTLNS(s)
	#define MPRINTLNSS(s)
    #define MPRINTSVAL(s,x)
    #define MPRINTLNSVAL(s,x)
	#define MPRINTSVALS(s,x,z)
	#define MPRINTLNSVALS(s,x,z)
	#define MPRINTFREE
	#define MTIME_RESET
	#define MTIME_PICK
#endif

#ifdef DPRINT_ON
   #define DPRINT(x)		MPRINT(x)
	#define DPRINTHEX(x)	MPRINTHEX(x)
	#define DPRINTLN(x)		MPRINTLN(x)
	#define DPRINTS(s)		MPRINTS(s)
	#define DPRINTLNS(s)	MPRINTLNS(s)
	#define DPRINTSVAL(s,x) MPRINTSVAL(s,x)
    #define DPRINTLNSVAL(s,x) MPRINTLNSVAL(s,x)
	#define DPRINTSVALS(s,x,z) MPRINTSVALS(s,x,z)
	#define DPRINTLNSVALS(s,x,z) MPRINTLNSVALS(s,x,z)
	#define DPRINTFREE 		MPRINTFREE
	#define DTIME_RESET  	DTIME_RESET
	#define DTIME_PICK  	DTIME_PICK

#else
	#define DPRINT(x)
	#define DPRINTHEX(x)
	#define DPRINTLN(x)
	#define DPRINTS(s)
	#define DPRINTLNS(s)
	#define DPRINTSVAL(s,x)
    #define DPRINTLNSVAL(s,x)
	#define DPRINTSVALS(s,x,z)
	#define DPRINTLNSVALS(s,x,z)
	#define DPRINTFREE
	#define DTIME_RESET
	#define DTIME_PICK
#endif

#define ASSERTP(e,s)	((e) ? (void)0 : ({MPRINTLNS(s);MPRINTLN("[program aborted]");Serial.flush();abort();}));

extern unsigned long timeStampLastPick;
extern unsigned long timeDuration;
extern unsigned int  timeStampId;

int freeRam ();

#endif
