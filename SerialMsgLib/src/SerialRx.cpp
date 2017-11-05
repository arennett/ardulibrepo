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



SerialRx::SerialRx(SerialPort* pSerialPort,size_t maxDataSize) {
	this->pSerialPort =pSerialPort;
	updateCallback= NULL;
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];
	lastByte=0;
}




SerialRx::~SerialRx() {
	delete pRecBuffer;

}

void SerialRx::setUpdateCallback(void (*ptr)(byte* data, size_t data_size)){
	updateCallback=ptr;
}
void SerialRx::begin(long speed){
	pSerialPort->begin(speed);

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
	long restOfTime= timeOut;
	data_size=0;
	pData = pRecBuffer;

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
	return SerialRx::waitOnMessage(pData, data_size, timeOut , WAITED_READ_CHECKPERIOD_MSEC);
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
					DPRINTSVAL("BUFFER OVERFLOW: DATA SIZE >=" ,bufferSize - sizeof serPostamble );
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
