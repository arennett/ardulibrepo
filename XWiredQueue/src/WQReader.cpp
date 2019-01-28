/*
 * WQReader.cpp
 *
 *  Created on: 27.01.2019
 *      Author: User
 */
#include "Arduino.h"
#include <Wire.h>
#include <tools.h>
#include "WQDefines.h"
#include "WQReader.h"

WQReader::WQReader(uint8_t pin_newdata,int i2c_master_address){
	m_pin_newdata =pin_newdata;
	m_i2c_master_address =i2c_master_address;

}

WQReader::~WQReader() {
	// TODO Auto-generated destructor stub
}

void WQReader::init() {
	pinMode(m_pin_newdata, INPUT_PULLUP);
	MPRINTLNSVAL("WQReader::init: INPUT_PULLUP on pin: " ,INPUT_PULLUP);
}

bool WQReader::process (tWQMessage& wqMessage){
	byte idx = 0;
	if (digitalRead(m_pin_newdata) == LOW) {
			MPRINTLNSVAL("WQReader::process: DATA available for newData PIN:  ",m_pin_newdata);
			Wire.requestFrom(m_i2c_master_address, WQ_MESSAGE_LENGTH);
			MPRINTLNSVAL("WQReader::process: available bytes: ",Wire.available());
			while (Wire.available() && idx < WQ_MESSAGE_LENGTH) {
				wqMessage.bytes[idx]=Wire.read(); // receive a byte as character
				MPRINTSVAL("WQReader::process : read bytes[",idx);
				MPRINTLNSVAL("] : ",wqMessage.bytes[idx]);
				++idx;
			}
			MPRINTLNSVAL("WQReader::process : CMD: ", wqMessage.cmd);
	}
	return idx==WQ_MESSAGE_LENGTH;
}
