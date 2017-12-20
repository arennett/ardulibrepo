#ifndef tools_h
#define tools_h
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "Arduino.h"



// uncomment to switch off all messages
// #define MPRINT_OFF
// comment ti switch on debug messages
// #define DPRINT_ON
//#define DPRINT_ON
//#define MPRINT_OFF

#define PRINTFREE  Serial.print(F("free sram : "));Serial.println(freeRam())
#define FS(x) (__FlashStringHelper*)(x)
#define MTIMESTAMP MPRINT(F(" >>> "));MPRINT(millis());MPRINT(F(" >>> "))


    #define XPRINT(x)   	Serial.print((x))
	#define XPRINTHEX(x)   	Serial.print((x),HEX)
	#define XPRINTLN(x) 	Serial.println((x))

	#define XPRINTS(s)   	XPRINT(F(s))
	#define XPRINTSS(s)   	XPRINT(FS(s))

	#define XPRINTLNS(s)    XPRINTLN(F(s))
	#define XPRINTLNSS(s)   XPRINTLN(FS(s))

	#define XPRINTSVAL(s,x)  	XPRINTS(s);XPRINT(x)
    #define XPRINTLNSVAL(s,x)  	XPRINTS(s);XPRINTLN(x)
	#define XPRINTSVALS(s,x,z)  XPRINTS(s);XPRINT(x);XPRINTLNS(z)

	#define XPRINTFREE  XPRINTLNSVAL("free sram : " ,(freeRam()))

//#define MPRINT_OFF
#ifndef MPRINT_OFF

	#define MPRINT(x)   	Serial.print((x))
	#define MPRINTHEX(x)   	Serial.print((x),HEX)
	#define MPRINTLN(x) 	Serial.println((x))

	#define MPRINTS(s)   	MPRINT(F(s))
	#define MPRINTSS(s)   	MPRINT(FS(s))

	#define MPRINTLNS(s)    MPRINTLN(F(s))
	#define MPRINTLNSS(s)   MPRINTLN(FS(s))

	#define MPRINTSVAL(s,x)  	MPRINTS(s);MPRINT(x)
    #define MPRINTLNSVAL(s,x)  	MPRINTS(s);MPRINTLN(x)
	#define MPRINTSVALS(s,x,z)  MPRINTS(s);MPRINT(x);MPRINTLNS(z)

	#define MPRINTFREE  MPRINTLNSVAL("free sram : " ,(freeRam()))


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

	#define MPRINTFREE



#endif

#ifndef DPRINT_ON
    #define DPRINT(x)
	#define DPRINTHEX(x)
	#define DPRINTLN(x)
	#define DPRINTS(s)
	#define DPRINTLNS(s)
	#define DPRINTSVAL(s,x)
    #define DPRINTLNSVAL(s,x)
	#define DPRINTSVALS(s,x,z)
	#define DPRINTFREE
#else


	#define DPRINT(x)		MPRINT(x)
	#define DPRINTHEX(x)	MPRINTHEX(x)
	#define DPRINTLN(x)		MPRINTLN(x)
	#define DPRINTS(s)		MPRINTS(s)
	#define DPRINTLNS(s)	MPRINTLNS(s)
	#define DPRINTSVAL(s,x) MPRINTSVAL(s,x)
    #define DPRINTLNSVAL(s,x) MPRINTLNSVAL(s,x)
	#define DPRINTSVALS(s,x,z) MPRINTSVALS(s,x,z)
	#define DPRINTFREE 		MPRINTFREE

#endif

#define ASSERTP(e,s)	((e) ? (void)0 : ({MPRINTLNS(s);MPRINTLN("[program aborted]");Serial.flush();abort();}));

int freeRam ();

#endif
