/*
 * WQWriter.cpp
 *
 *  Created on: 27.01.2019
 *      Author: User
 */
#include "Arduino.h"
#include <Wire.h>
#define MPRINT_ON
#include <tools.h>
#include <Queue.h>
#include "WQDefines.h"
#include "WQWriter.h"


WQWriter::WQWriter(){
	// TODO Auto-generated constructor stub

}

WQWriter::~WQWriter() {
	// TODO Auto-generated destructor stub
}

void WQWriter::init(uint8_t pin_newdata,int i2c_master_address ,void (*onRequestHandler)() ){
	XPRINTLNS("");
	XPRINTLNS("WQWriter::init()");
	pinMode(pin_newdata,OUTPUT);
	Wire.begin(i2c_master_address);                // join i2c bus with address #8
	Wire.onRequest(onRequestHandler);
	XPRINTFREE;
}

void WQWriter::write(tWQMessage message) {
	m_wqQueue.enqueue(message);
}

void WQWriter::onRequestEvent() {
	XPRINTLNS("WQWriter::onRequestEvent()");
	if (!m_wqQueue.isEmpty()){
		tWQMessage wqMessage = m_wqQueue.dequeue();
		Wire.write(wqMessage.bytes, WQ_MESSAGE_LENGTH);
	}
}

