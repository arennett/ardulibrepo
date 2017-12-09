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
}



SerialRx::~SerialRx() {


}
SerialRx::SerialRx(SerialPort* pSerialPort,size_t maxDataSize,void (*ptr)(const byte* data, size_t data_size,SerialPort* pPort)) {
	this->pSerialPort =pSerialPort;
	updateCallback= ptr;
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];
	lastByte=0;
}

bool SerialRx::setPort(SerialPort* pSerialPort) {
	this->pSerialPort =pSerialPort;
	return listen();
}

void SerialRx::setUpdateCallback(void (*ptr)(const byte* data, size_t data_size,SerialPort* pPort)){
	updateCallback=ptr;
}




bool SerialRx::listen () {
	return  pSerialPort->listen();
}

SerialPort* SerialRx::getPort(){
	return pSerialPort;
}

bool SerialRx::readNext(){
	byte b;
	return readNext(&b);
}

void SerialRx::readNextOnAllPorts(){
	SerialPort* pport=SerialPort::pSerialPortList;
	while(pport) {

		if (pport->available()) {
			//DPRINTLNS("SerialRx::readNextOnAllPorts> data available");
		}
		if (pport->pSerialRx->readNext()){
			MPRINTLNS("SerialRx::readNextOnAllPorts> receiving a message... ");
		}
		pport=(SerialPort*)pport->pNext;
	}
}



bool SerialRx::waitOnMessage(byte*&  pData, size_t& data_size, unsigned long timeOut ,unsigned long checkPeriod){
	DPRINTLNS("waitOnMessage");

	data_size=0;
	pData = pSerialPort->pBuffer;
	if (checkPeriod==0){
		checkPeriod = WAITED_READ_CHECKPERIOD_MSEC;
	}
	if (timeOut==0) {
		timeOut=WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	}
	unsigned long restOfTime= timeOut;

	while (restOfTime >= checkPeriod) {
		if (readNext()) {
	 		DPRINTLNS("waitOnMessage : message received");
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
			DPRINTLNSVAL("byte: " ,lastByte);
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
							DPRINTLNSVAL("SerialRx::readNext> message size: "  ,prevDataCount);

							if (updateCallback && dataCollect) {
								updateCallback(pRecBuffer,prevDataCount,pSerialPort );

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
