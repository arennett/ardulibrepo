/*
 * SoftSerialRx.cpp
 *
 *  Created on: 30.10.2017
 *      Author: User
 */

#include "SoftSerialRx.h"
#include "tools.h"


SoftSerialRx::SoftSerialRx(byte pinRx,byte pinTx,size_t maxDataSize) {
	// TODO Auto-generated constructor stub
	pSoftSerial = new SoftwareSerial(pinRx,pinTx);
	deleteSoftSerial=true;
	updateCallback=NULL;
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];
	_byte=0;


}


SoftSerialRx::SoftSerialRx(SoftwareSerial* pSoftSerial,size_t maxDataSize){
	this->pSoftSerial = pSoftSerial;
	deleteSoftSerial = false;
	updateCallback=NULL;
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];
	_byte=0;
}



SoftSerialRx::~SoftSerialRx() {
	// TODO Auto-generated destructor stub
	delete pSoftSerial;
	delete pRecBuffer;

}

void SoftSerialRx::setUpdateCallback(void (*ptr)(byte* data, size_t data_size)){
	updateCallback=ptr;
}
void SoftSerialRx::begin(long speed){
	pSoftSerial->begin(speed);

}
// This function sets the current object as the "listening"
// one and returns true if it replaces another
bool SoftSerialRx::listen () {
	return  pSoftSerial->listen();
}

SoftwareSerial* SoftSerialRx::getSoftSerial(){
	return pSoftSerial;
}

bool SoftSerialRx::readNext(){
	return readNext(&_byte);
}

bool SoftSerialRx::waitOnMessage(byte* pData,size_t& data_size, unsigned long timeOut ){
	MPRINTLN("waitOnMessage");
	long restOfTime= timeOut;
	data_size=0;
	 pData = pRecBuffer;
	while (restOfTime >= 10) {
		if (readNext()) {
	 		MPRINTLN("waitOnMessage : message received");
			MPRINTSVAL("restOfTime: " ,restOfTime);
			data_size = prevDataCount;
			return true;
		}
		delay(10);
		restOfTime=restOfTime-10;
	}

	MPRINTSVAL("restOfTime TIMEOUT: " ,restOfTime);
	return false;
}

/**
 * returns true if incoming message was completed
 */
bool SoftSerialRx::readNext(byte* pByte){
	bool messReceived = false;

	if (!pSoftSerial->isListening()){
		MPRINTLN("NOT LISTEN");
		return messReceived;
	}
	if (pSoftSerial->available()> 0) {
			_byte= pSoftSerial->read();
			 pByte[0] =_byte;
			MPRINTSVAL("byte: " ,_byte);
			if (dataCollect) {
				if (dataCount < bufferSize) {
					pRecBuffer[dataCount]=_byte;
					dataCount++;
				}else {
					MPRINTSVAL("BUFFER OVERFLOW: DATA SIZE >=" ,bufferSize - sizeof serPostamble );
					dataCollect=false;
				}

			}

			if ( _byte == serPreamble[preAmCount] ) {
				//MPRINTSVAL("serPreamble COUNT:",preAmCount);
				if (preAmCount == (sizeof serPreamble) -1 ) {
					MPRINTLN("serPreamble COMPLETE");
					preAmCount=0;
					dataCollect=true;
					dataCount=0;
				}else {
					preAmCount++;
				}
			}else{
				preAmCount=0;
				if (_byte == serPreamble[preAmCount]) {
					//MPRINTSVAL("serPreamble COUNT:",preAmCount);
					preAmCount++;
				}


			}

			if ( _byte == serPostamble[postAmCount] ) {
						//MPRINTSVAL("serPostamble COUNT:",postAmCount);
						if (postAmCount == (sizeof serPostamble) -1 ) {
							MPRINTLN("serPostamble COMPLETE");
							prevDataCount = dataCount-sizeof serPostamble;
							MPRINTSVAL("MESSAGE SIZE: "  ,prevDataCount);

							if (updateCallback && dataCollect) {
								updateCallback(pRecBuffer,prevDataCount );

							}

							messReceived = true;
							MPRINTLN("messReceived = true;");
							postAmCount=0;
							dataCollect=false;
							dataCount=0;
						}else {
							postAmCount++;
						}
			}else{
				postAmCount=0;
				if (_byte == serPostamble[postAmCount]) {
					//MPRINTSVAL("serPostamble COUNT:",preAmCount);
					postAmCount++;
				}


			}
		}

	return messReceived;
}
