/*
 * tools.cpp

 *
 *  Created on: 24.10.2017
 *      Author: User
 */

#include "src/tools.h"

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

