/*
 * SoftSerialRx.cpp
 *
 *  Created on: 30.10.2017
 *      Author: User
 *
 * implementation for SoftSerialRx
 * comments see SoftSerialRx.h
 */

#include "Arduino.h"
#include <tools.h> //see ToolsLib2
#include <SoftwareSerial.h>
#include "SerialMsg.h"
#include "SerialRx.h"
#include "SerialNode.h"
#include "SerialNodeNet.h"

namespace SerialMsgLib {
SerialRx::SerialRx() {
	//updateCallback = NULL;
	lastByte = 0;
	XPRINTLNSVAL("SerialRx::SerialRx() free ",freeRam());
}

SerialRx::~SerialRx() {

}
SerialRx::SerialRx(SerialPort* pSerialPort, size_t maxDataSize/*,
		void (*ptr)(const byte* data, size_t data_size, SerialPort* pPort)*/) {
	this->pSerialPort = pSerialPort;
	//updateCallback = ptr;
	lastByte = 0;
	createBuffer(maxDataSize);
	XPRINTLNSVAL("SerialRx::SerialRx() free ",freeRam());
}

bool SerialRx::setPort(SerialPort* pSerialPort) {
	this->pSerialPort = pSerialPort;
	return true;
}

byte* SerialRx::getBuffer(){
	return pRecBuffer;
}

size_t SerialRx::getBufferSize(){
	return bufferSize;
}


void SerialRx::createBuffer(size_t maxDataSize) {
	this->bufferSize = maxDataSize + sizeof serPostamble;
	if(pRecBuffer){
		delete pRecBuffer;
	}
	pRecBuffer = new byte[bufferSize];

}

//void SerialRx::setUpdateCallback(
//		void (*ptr)(const byte* data, size_t data_size, SerialPort* pPort)) {
//	updateCallback = ptr;
//
//}

bool SerialRx::listen() {
	return pSerialPort->listen();
}

SerialPort* SerialRx::getPort() {
	return pSerialPort;
}



bool SerialRx::waitOnMessage(byte*& pData, size_t& data_size,
		tStamp timeOut, tStamp checkPeriod) {
	DPRINTLNS("waitOnMessage");

	data_size = 0;
	pData = pRecBuffer;
	if (checkPeriod == 0) {
		checkPeriod = WAITED_READ_CHECKPERIOD_MSEC;
	}
	if (timeOut == 0) {
		timeOut = WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	}
	tStamp restOfTime = timeOut;

	while (restOfTime >= checkPeriod) {
		if (readNext()) {
			DPRINTLNS("waitOnMessage : message received");
			DPRINTSVAL("restOfTime: ", restOfTime);
			data_size = dataSize;
			return true;
		}
		delay(checkPeriod);
		restOfTime -= checkPeriod;
	}

	DPRINTSVAL("TIMEOUT! restOfTime:", restOfTime);
	return false;
}

bool SerialRx::waitOnMessage(byte*& pData, size_t& data_size,
		tStamp timeOut) {
	return waitOnMessage(pData, data_size, timeOut, 0);
}



/**
 * returns true if incoming message was completed
 */

bool SerialRx::readNext() {

	assert(pSerialPort);

	bool messReceived = false;

	if (pSerialPort->available() > 0) {
		assert(postAmCount < sizeof(serPostamble));
		assert(preAmCount < sizeof(serPreamble));
		//assert(dataCount < bufferSize);

		lastByte = pSerialPort->read();
		// pByte[0] =lastByte;



		DPRINTLNSVAL("byte: ", lastByte);
		if (dataCollect) {
			DPRINTLNSVAL("dc: ", dataCount);
			if (dataCount < bufferSize) {
				pRecBuffer[dataCount] = lastByte;
				++dataCount;
			} else {

				XPRINTLNSVAL("BUFFER OVERFLOW: dataSize > ",
						bufferSize - sizeof serPostamble);
				dataCollect = false;
			}

		}

		if (lastByte == serPreamble[preAmCount]) {
			//DPRINTSVAL("serPreamble COUNT:",preAmCount);
			if (preAmCount == (sizeof serPreamble) - 1) {
				XPRINTLNSVAL("serPreamble COMPLETE port : ",pSerialPort->remoteSysId);
				preAmCount = 0;
				dataCollect = true;
				dataCount = 0;
			} else {
				preAmCount++;
			}
		} else {
			preAmCount = 0;
			if (lastByte == serPreamble[preAmCount]) {
				//DPRINTSVAL("serPreamble COUNT:",preAmCount);
				preAmCount++;
			}
		}

		if (lastByte == serPostamble[postAmCount]) {

			//DPRINTSVAL("serPostamble COUNT:",postAmCount);
			if (postAmCount == (sizeof serPostamble) - 1) {
				XPRINTLNSVAL("serPostamble COMPLETE port : ", pSerialPort->remoteSysId);
				dataSize = dataCount - sizeof serPostamble;
				DPRINTLNSVAL("SerialRx::readNext> message size: ",
						dataSize);

				//assert(updateCallback);
				if (/*updateCallback && */dataCollect) {

					DPRINTLNS(" SerialRx::readNext> call updateCallback");
					//updateCallback(pRecBuffer, dataSize, pSerialPort);
					SerialNodeNet::getInstance()->update(pRecBuffer, dataSize, pSerialPort);
				}

				messReceived = true;
				DPRINTLNS("messReceived = true;");
				postAmCount = 0;
				dataCollect = false;
				dataCount = 0;
			} else {
				postAmCount++;
			}
		} else {
			postAmCount = 0;
			if (lastByte == serPostamble[postAmCount]) {
				//DPRINTSVAL("serPostamble COUNT:",preAmCount);
				postAmCount++;
			}

		}
	}

	return messReceived;
}
}
