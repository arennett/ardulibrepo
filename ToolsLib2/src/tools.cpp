/*
 * tools.cpp

 *
 *  Created on: 24.10.2017
 *      Author: User
 */

#include "Arduino.h"
#include "tools.h"


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link.

	 MPRINTS("ASSERT func : ");MPRINTLN(__func);
	 MPRINTS("       file : ");MPRINTLN(__file);
     MPRINTS("       line : ");MPRINTLN(__lineno);
     MPRINTS("       val  : ");MPRINTLN(__sexp);
    Serial.flush();

    // abort program execution.
    abort();

}
