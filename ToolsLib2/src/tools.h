#ifndef tools_h
#define tools_h

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERTNODE(pNode)  assert(pNode->pCcb->localAddr.sysId==10);assert(pNode->pCcb->localAddr.nodeId==1);
#define ADDRESSOFNODE(pNode) MPRINTLNSVAL("address of node: ",long ((void*) pNode));
#define DUMPNODE(pNode) for (size_t i=0;i < sizeof(SerialNode);i++) {MPRINTSVAL(" node byte i :",i);MPRINTS(" ");MPRINTHEX(((byte*) pNode)[i]);MPRINTLN("");}



// uncomment to switch off all messages
// #define MPRINT_OFF
// comment ti switch on debug messages
// #define DPRINT_ON
#define DPRINT_ON
//#define MPRINT_OFF
#ifndef MPRINT_OFF

	#define MPRINT(x)   	Serial.print((x))
	#define MPRINTHEX(x)   	Serial.print((x),HEX)
	#define MPRINTLN(x) 	Serial.println((x))

	#define MPRINTS(x)   	MPRINT(F(x))
	#define MPRINTLNS(x)    MPRINTLN(F(x))

	#define MPRINTSVAL(s,x)  	MPRINTS(s);MPRINT(x)
    #define MPRINTLNSVAL(s,x)  	MPRINTS(s);MPRINTLN(x)
	#define MPRINTSVALS(s,x,z)  MPRINTS(s);MPRINT(x);MPRINTLNS(z)

	#define MPRINTFREE  MPRINTLNSVAL("free sram : " ,(freeRam()))


#else
    #define MPRINT(x)
	#define MPRINTHEX(x)
	#define MPRINTLN(x)
	#define MPRINTS(x)
	#define MPRINTLNS(x)
    #define MPRINTSVAL(s,x)
    #define MPRINTLNSVAL(s,x)
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
    #define DPRINTLNSVAL(s,x) 	MPRINTLNSVAL(s,x)
	#define DPRINTSVALS(s,x,z) 	MPRINTSVALS(s,x,z)
	#define DPRINTFREE 			MPRINTFREE

#endif



int freeRam ();

#endif
