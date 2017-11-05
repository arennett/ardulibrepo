#ifndef tools_h
#define tools_h




// uncomment to switch off all messages
// #define MPRINT_OFF
// comment ti switch on debug messages
// #define DPRINT_ON


#ifndef MPRINT_OFF

	#define MPRINT(x)   	Serial.print((x))
	#define MPRINTHEX(x)   	Serial.print((x,HEX))
	#define MPRINTLN(x) 	Serial.println((x))

	#define MPRINTS(x)   	MPRINT(F(x))
	#define MPRINTLNS(x)    MPRINTLN(F(x))

	#define MPRINTSVAL(s,x)  	MPRINTS(s);MPRINTLN(x)
	#define MPRINTSVALS(s,x,z)  MPRINTS(s);MPRINT(x);MPRINTLNS(z)

	#define MPRINTFREE  MPRINTSVAL("free sram : " ,(freeRam()))


#else
    #define MPRINT(x)
	#define MPRINTHEX(x)
	#define MPRINTLN(x)
	#define MPRINTS(x)
	#define MPRINTLNS(x)

	#define MPRINTSVAL(s,x)
	#define MPRINTSVALS(s,x,z)

	#define MPRINTFREE



#endif

#ifndef DPRINT_ON
    #define DPRINT(x)
	#define DPRINTHEX(x)
	#define DPRINTLN(x)
	#define DPRINTS(x)
	#define DPRINTLNS(x)
	#define DPRINTSVAL(s,x)
	#define DPRINTSVALS(s,x,z)
	#define DPRINTFREE
#else
    #define DPRINT(x) 			MPRINT(x)
	#define DPRINTHEX(x) 		MPRINTHEX(x)
	#define DPRINTLN(x) 		MPRINTLN(x)
	#define DPRINTS(x) 			MPRINTS(x)
	#define DPRINTLNS(x) 		MPRINTLNS(x)
	#define DPRINTSVAL(s,x) 	MPRINTSVAL(s,x)
	#define DPRINTSVALS(s,x,z) 	MPRINTSVALS(s,x,z)
	#define DPRINTFREE 			MPRINTFREE

#endif



int freeRam ();

#endif
