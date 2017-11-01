/*
 * SoftSerialTx.cpp
 *
 *  Created on: 31.10.2017
 *      Author: User
 */
#include "SoftSerialTx.h"
#include "SoftSerial.h"

SoftSerialTx::SoftSerialTx(byte pinRx,byte pinTx){
	pSoftSerial = new SoftwareSerial(pinRx,pinTx);
}

SoftSerialTx::~SoftSerialTx() {

	delete pSoftSerial;
}

void SoftSerialTx::begin(long speed){
	pSoftSerial->begin(speed);

}


SoftwareSerial* SoftSerialRx::getSoftSerial(){
	return pSoftSerial;
}


void SoftSerialTx::sendData(byte* data, size_t datasize) {

	pSoftSerial->write(serPreamble,sizeof serPreamble);
	pSoftSerial->write(data,datasize);
	pSoftSerial->write(serPostamble,sizeof serPostamble);
}
