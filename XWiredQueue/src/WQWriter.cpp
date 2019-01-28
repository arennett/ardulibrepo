/*
 * WQWriter.cpp
 *
 *  Created on: 27.01.2019
 *      Author: User
 */
#include "Arduino.h"
#include <Wire.h>
#include <tools.h>
#include "WQDefines.h"
#include "WQWriter.h"


WQWriter::WQWriter(){
	// TODO Auto-generated constructor stub

}

WQWriter::~WQWriter() {
	// TODO Auto-generated destructor stub
}

void WQWriter::init(uint8_t pin_newdata,int i2c_master_address ,void (*onRequestHandler)() ){
	m_pin_newdata=pin_newdata;
	MPRINTLNS("WQWriter::init()");
	pinMode(pin_newdata,OUTPUT);
	Wire.begin(i2c_master_address);                // join i2c bus with address #8
	Wire.onRequest(onRequestHandler);
}

void WQWriter::write(tWQMessage wqMessage) {
	m_wqQueue.enqueue(wqMessage);
	digitalWrite(m_pin_newdata,LOW);
	MPRINTLNSVAL("WQWriter::write: cmd: ",wqMessage.cmd);
}

void WQWriter::onRequestEvent() {
	MPRINTLNS("");
	MPRINTLNS("WQWriter::onRequestEvent()");
	if (!m_wqQueue.isEmpty()){
		MPRINTLNS("WQWriter::onRequestEvent:: dequeue message");
		tWQMessage wqMessage = m_wqQueue.dequeue();
		MPRINTLNSVAL("WQWriter::onRequestEvent:: CMD :",wqMessage.cmd);
		MPRINTSVAL("WQWriter::onRequestEvent:: write ",sizeof(wqMessage));MPRINTLN(" bytes");
		Wire.write(wqMessage.bytes, sizeof(wqMessage));
		if (m_wqQueue.isEmpty()) {
			MPRINTLNS("WQWriter::onRequestEvent:: queue is empty");
			digitalWrite(m_pin_newdata,HIGH);
		}
	}else {
		// to be sure
		if (digitalRead(m_pin_newdata)==LOW) {
			MPRINTLNS("WQWriter::onRequestEvent:: queue is unexpected empty");
			digitalWrite(m_pin_newdata,HIGH);
		}

	}
}

