/*
 * tools.cpp

 *
 *  Created on: 24.10.2017
 *      Author: User
 */

#include "XTools.h"

#include "Arduino.h"

unsigned long timeStampLastPick = 0;
unsigned long timeDuration = 0;
unsigned int timeStampId= 0;



int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link.

	 XPRINTS("ASSERT func : ");XPRINTLN(__func);
	 XPRINTS("       file : ");XPRINTLN(__file);
     XPRINTS("       line : ");XPRINTLN(__lineno);
     XPRINTS("       val  : ");XPRINTLN(__sexp);
    Serial.flush();

    // abort program execution.
    abort();

}
