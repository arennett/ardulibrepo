/*
 * SoftSerialTx.cpp
 *
 *  Created on: 31.10.2017
 *      Author: Andre Rennett
 *
 *  Implementation for SoftSerialTx
 *  comments see SoftSerialTx.h
 */
#include <SoftwareSerial.h>
#include "SoftSerialTx.h"
#include "SoftSerial.h"
#include "tools.h"


SoftSerialTx::SoftSerialTx(byte pinRx,byte pinTx){
	pSoftSerial = new SoftwareSerial(pinRx,pinTx);
	deleteSoftSerial =true;
}


SoftSerialTx::SoftSerialTx(SoftwareSerial* pSoftSerial){
	this->pSoftSerial = pSoftSerial;
	deleteSoftSerial = false;
}


SoftSerialTx::~SoftSerialTx() {

	if (deleteSoftSerial) {
		delete pSoftSerial;
	}
}

void SoftSerialTx::begin(long speed){
	pSoftSerial->begin(speed);

}


SoftwareSerial* SoftSerialTx::getSoftSerial(){
	return pSoftSerial;
}


void SoftSerialTx::sendData(byte* data, size_t datasize) {

	pSoftSerial->write(serPreamble,sizeof serPreamble);
	pSoftSerial->write(data,datasize);
	pSoftSerial->write(serPostamble,sizeof serPostamble);
	MPRINTLN("write data");
}
