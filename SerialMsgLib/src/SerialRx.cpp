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



SerialRx::SerialRx() {
	updateCallback= NULL;
	this->bufferSize  = 0;
	pRecBuffer = NULL;
	lastByte=0;
}

SerialRx::SerialRx(size_t maxDataSize) {
	updateCallback= NULL;
	lastByte=0;
	createBuffer(maxDataSize);
}


SerialRx::~SerialRx() {
	delete pRecBuffer;

}

void SerialRx::createBuffer(size_t maxDataSize){
	if(pRecBuffer){
		delete pRecBuffer;
	}
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];
}

bool SerialRx::setPort(SerialPort* pSerialPort) {
	this->pSerialPort =pSerialPort;
	return listen();
}

void SerialRx::setUpdateCallback(void (*ptr)(const byte* data, size_t data_size,SerialPort* pSerialPort)){
	updateCallback=ptr;
}

void SerialRx::setSerialHeaderRx(SerialHeaderRx* pSerialHeaderRx){
	this->pSerialHeaderRx=pSerialHeaderRx;
}


bool SerialRx::listen () {
	return  pSerialPort->listen();
}

SerialPort* SerialRx::getSerialPort(){
	return pSerialPort;
}

bool SerialRx::readNext(){
	return readNext(&lastByte);
}

bool SerialRx::waitOnMessage(byte*&  pData, size_t& data_size, unsigned long timeOut ,unsigned long checkPeriod){
	DPRINTLN("waitOnMessage");

	data_size=0;
	pData = pRecBuffer;
	if (checkPeriod==0){
		checkPeriod = WAITED_READ_CHECKPERIOD_MSEC;
	}
	if (timeOut==0) {
		timeOut=WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	}
	unsigned long restOfTime= timeOut;

	while (restOfTime >= checkPeriod) {
		if (readNext()) {
	 		DPRINTLN("waitOnMessage : message received");
			DPRINTSVAL("restOfTime: " ,restOfTime);
			data_size = prevDataCount;
			return true;
		}
		delay(checkPeriod);
		restOfTime-=checkPeriod;
	}

	DPRINTSVAL("TIMEOUT! restOfTime:" ,restOfTime);
	return false;
}

bool SerialRx::waitOnMessage(byte*&  pData, size_t& data_size, unsigned long timeOut){
	return waitOnMessage(pData, data_size, timeOut , 0);
}

/**
 * returns true if incoming message was completed
 */
bool SerialRx::readNext(byte* pByte){
	bool messReceived = false;

	if (!pSerialPort->isListening()){
		DPRINTLN("NOT LISTEN");
		return messReceived;
	}
	if (pSerialPort->available()> 0) {
			lastByte= pSerialPort->read();
			 pByte[0] =lastByte;
			DPRINTSVAL("byte: " ,lastByte);
			if (dataCollect) {
				if (dataCount < bufferSize) {
					pRecBuffer[dataCount]=lastByte;
					dataCount++;
				}else {
					MPRINTSVAL("BUFFER OVERFLOW: DATA SIZE >=" ,bufferSize - sizeof serPostamble );
					dataCollect=false;
				}

			}

			if ( lastByte == serPreamble[preAmCount] ) {
				//DPRINTSVAL("serPreamble COUNT:",preAmCount);
				if (preAmCount == (sizeof serPreamble) -1 ) {
					DPRINTLN("serPreamble COMPLETE");
					preAmCount=0;
					dataCollect=true;
					dataCount=0;
				}else {
					preAmCount++;
				}
			}else{
				preAmCount=0;
				if (lastByte == serPreamble[preAmCount]) {
					//DPRINTSVAL("serPreamble COUNT:",preAmCount);
					preAmCount++;
				}
			}

			if ( lastByte == serPostamble[postAmCount] ) {
						//DPRINTSVAL("serPostamble COUNT:",postAmCount);
						if (postAmCount == (sizeof serPostamble) -1 ) {
							DPRINTLN("serPostamble COMPLETE");
							prevDataCount = dataCount-sizeof serPostamble;
							DPRINTSVAL("MESSAGE SIZE: "  ,prevDataCount);

							if (updateCallback && dataCollect) {
								updateCallback(pRecBuffer,prevDataCount );

							}

							messReceived = true;
							DPRINTLN("messReceived = true;");
							postAmCount=0;
							dataCollect=false;
							dataCount=0;
						}else {
							postAmCount++;
						}
			}else{
				postAmCount=0;
				if (lastByte == serPostamble[postAmCount]) {
					//DPRINTSVAL("serPostamble COUNT:",preAmCount);
					postAmCount++;
				}


			}
		}

	return messReceived;
}
