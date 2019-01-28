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

void WQWriter::write(tWQMessage message) {
	m_wqQueue.enqueue(message);
}

void WQWriter::onRequestEvent() {

	if (!m_wqQueue.isEmpty()){
		MPRINTLNS("WQWriter::onRequestEvent:: dequeue message");
		tWQMessage wqMessage = m_wqQueue.dequeue();
		Wire.write(wqMessage.bytes, WQ_MESSAGE_LENGTH);
		if (m_wqQueue.isEmpty()) {
			MPRINTLNS("WQWriter::onRequestEvent:: queue is empty now");
			digitalWrite(m_pin_newdata,LOW);
		}
	}else {
		// to be sure
		if (digitalRead(m_pin_newdata)==HIGH) {
			MPRINTLNS("WQWriter::onRequestEvent:: queue is empty, but pin_newdata was high");
			digitalWrite(m_pin_newdata,LOW);
		}

	}
}

