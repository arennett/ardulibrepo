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


bool SerialRx::setPort(SerialPort* pSerialPort) {
	this->pPort =pSerialPort;
	this->pState = &pSerialPort->serialRxState;
	return listen();
}

void SerialRx::setUpdateCallback(void (*ptr)(const byte* data, size_t data_size,SerialPort* pPort)){
	updateCallback=ptr;
}




bool SerialRx::listen () {
	return  pPort->listen();
}

SerialPort* SerialRx::getPort(){
	return pPort;
}

bool SerialRx::readNext(){
	return readNext(&pState->lastByte);
}

void SerialRx::readNextOnAllPorts(){
	SerialPort* pport=SerialPort::pSerialPortList;
	while(pport) {
		setPort(pport);
		readNext();
		pport=(SerialPort*)pport->pNext;
	}
}



bool SerialRx::waitOnMessage(byte*&  pData, size_t& data_size, unsigned long timeOut ,unsigned long checkPeriod){
	DPRINTLN("waitOnMessage");

	data_size=0;
	pData = pState->pBuffer;
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
			data_size = pState->prevDataCount;
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

	if (!pPort->isListening()){
		DPRINTLN("NOT LISTEN");
		return messReceived;
	}
	if (pPort->available()> 0) {
		pState->lastByte= pPort->read();
			 pByte[0] =pState->lastByte;
			DPRINTSVAL("byte: " ,pPort->lastByte);
			if (pState->dataCollect) {
				if (pState->dataCount < pState->bufferSize) {
					pState->pBuffer[pState->dataCount]=pState->lastByte;
					 pState->dataCount++;
				}else {
					MPRINTSVAL("BUFFER OVERFLOW: DATA SIZE >=" , pState->bufferSize - sizeof serPostamble );
					pState->dataCollect=false;
				}
			}

			if ( pState->lastByte == serPreamble[pState->preAmCount] ) {
				//DPRINTSVAL("serPreamble COUNT:",preAmCount);
				if (pState->preAmCount == (sizeof serPreamble) -1 ) {
					DPRINTLN("serPreamble COMPLETE");
					pState->preAmCount=0;
					pState->dataCollect=true;
					pState->dataCount=0;
				}else {
					pState->preAmCount++;
				}
			}else{
				pState->preAmCount=0;
				if (pState->lastByte == serPreamble[pState->preAmCount]) {
					//DPRINTSVAL("serPreamble COUNT:",preAmCount);
					pState->preAmCount++;
				}
			}

			if ( pState->lastByte == serPostamble[pState->postAmCount] ) {
						//DPRINTSVAL("serPostamble COUNT:",postAmCount);
						if (pState->postAmCount == (sizeof serPostamble) -1 ) {
							DPRINTLN("serPostamble COMPLETE");
							pState->prevDataCount = pState->dataCount-sizeof serPostamble;
							DPRINTSVAL("MESSAGE SIZE: "  ,pState->prevDataCount);

							if (updateCallback && pState->dataCollect) {
								updateCallback( pState->pBuffer,pState->prevDataCount ,pPort);

							}

							messReceived = true;
							DPRINTLN("messReceived = true;");
							pState->postAmCount=0;
							pState->dataCollect=false;
							pState->dataCount=0;
						}else {
							pState->postAmCount++;
						}
			}else{
				pState->postAmCount=0;
				if (pState->lastByte == serPostamble[pState->postAmCount]) {
					//DPRINTSVAL("serPostamble COUNT:",preAmCount);
					pState->postAmCount++;
				}


			}
		}

	return messReceived;
}
