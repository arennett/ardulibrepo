/*
 * SerialMsg.h
 *
 *
 *  Created on: 31.10.2017
 *      Author: User
 *
 * be free to use different pre- and postamble
 */
#include "Arduino.h"

#ifndef SERIALMSG_H_
#define SERIALMSG_H_

#define PREAMBLE   {14,23,32,41}
#define POSTAMBLE  {96,87,78,69}
#define WAITED_READ_CHECKPERIOD_MSEC 10

byte serPreamble[4]  = PREAMBLE;
byte serPostamble[4] = POSTAMBLE;

#endif /* SERIALMSG_H_ */
