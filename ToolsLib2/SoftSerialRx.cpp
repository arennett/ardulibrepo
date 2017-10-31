/*
 * SoftSerialRx.cpp
 *
 *  Created on: 30.10.2017
 *      Author: User
 */

#include "SoftSerialRx.h"
#define MPRINT_OFF
#include "tools.h"




SoftSerialRx::SoftSerialRx(byte pinRx,byte pinTx,size_t maxDataSize) {
	// TODO Auto-generated constructor stub
	pSoftSerial = new SoftwareSerial(pinRx,pinTx);
	updateCallback=NULL;
	this->bufferSize  = maxDataSize + sizeof serPostamble;
	pRecBuffer = new byte[bufferSize];



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
SoftwareSerial* getSoftSerial();

void SoftSerialRx::readNext(){
	if (!pSoftSerial->isListening()){
		MPRINTLN("NOT LISTEN");
		return;
	}
	if (pSoftSerial->available()> 0) {
			byte b= pSoftSerial->read();
			MPRINTSVAL("byte: " ,b);
			if (dataCollect) {
				if (dataCount < bufferSize) {
					pRecBuffer[dataCount]=b;
					dataCount++;
				}else {
					MPRINTSVAL("BUFFER OVERFLOW: DATA SIZE >=" ,bufferSize - sizeof serPostamble );
					dataCollect=false;
				}

			}

			if ( b == serPreamble[preAmCount] ) {
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
				if (b == serPreamble[preAmCount]) {
					//MPRINTSVAL("serPreamble COUNT:",preAmCount);
					preAmCount++;
				}


			}

			if ( b == serPostamble[postAmCount] ) {
						//MPRINTSVAL("serPostamble COUNT:",postAmCount);
						if (postAmCount == (sizeof serPostamble) -1 ) {
							MPRINTLN("serPostamble COMPLETE");

							MPRINTSVAL("MESSAGE SIZE: "  ,dataCount - sizeof serPostamble );

							if (updateCallback && dataCollect) {
								updateCallback(pRecBuffer,dataCount - sizeof serPostamble );
							}
							if (!updateCallback)	MPRINTLN("please set update callback function");


							postAmCount=0;
							dataCollect=false;
							dataCount=0;
						}else {
							postAmCount++;
						}
			}else{
				postAmCount=0;
				if (b == serPostamble[postAmCount]) {
					//MPRINTSVAL("serPostamble COUNT:",preAmCount);
					postAmCount++;
				}


			}
		}

}
