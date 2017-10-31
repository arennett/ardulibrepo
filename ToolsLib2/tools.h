#ifndef tools_h
#define tools_h

#ifndef MPRINT_OFF

	#define MPRINT(x)   	Serial.print((x))
	#define MPRINTHEX(x)   	Serial.print((x,HEX))
	#define MPRINTLN(x) 	Serial.println((x))

	#define MPRINTS(x)   	MPRINT(F(x))
	#define MPRINTLNS(x)    MPRINTLN(F(x))

	#define MPRINTSVAL(s,x)  	MPRINTS(s);MPRINTLN(x)
	#define MPRINTSVALS(s,x,z)  MPRINTS(s);MPRINT(x);MPRINTLNS(z)

	#define MPRINTFREE  MPRINTSVALS("free sram : " ,freeRam())


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


int freeRam ();

#endif
