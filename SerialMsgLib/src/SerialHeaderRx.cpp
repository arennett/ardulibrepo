/*
 * SerialHeaderRx.cpp
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#include "SerialHeader.h"
#include "SerialHeaderRx.h"

SerialHeaderRx::SerialHeaderRx(SerialPort* pSerialPort, size_t maxDataSize) {
	this->pSerialPort = pSerialPort;
	this->pSerialRx = new SerialRx(pSerialPort,maxDataSize + sizeof(tSerialHeader));
	pSerialRx->setSerialHeaderRx(this);

}

SerialHeaderRx::~SerialHeaderRx(){
	delete pSerialRx;
}



void SerialHeaderRx::setUpdateCallback(
		void (*ptr)(byte* pData, size_t data_size), byte addr) {
	tCallBackMapper* pNextMapper = pCallBackMapperList;

	if (pNextMapper == NULL) {
		pNextMapper = new tCallBackMapper();
	} else {
		while (pNextMapper->pNext != NULL) {
			pNextMapper = (tCallBackMapper*)pNextMapper->pNext;
		}
		pNextMapper->pNext = new tCallBackMapper();
		pNextMapper = (tCallBackMapper*)pNextMapper->pNext;
	}
	pNextMapper->addr = addr;
	pNextMapper->pUserCallBack = ptr;

}

void SerialHeaderRx::internalCallBack(byte* pData, size_t data_size) {
	tSerialHeader* pSerialHeader;
	pSerialHeader=(tSerialHeader*)pData;
	byte* pUserData = pData + sizeof(pSerialHeader);
	tCallBackMapper* pNextMapper = pCallBackMapperList;

		while (pNextMapper != NULL) {
			if (pSerialHeader->addrTo==pNextMapper->addr) {
				pNextMapper->pUserCallBack(pUserData,data_size-sizeof (tSerialHeader));
			}
			pNextMapper =(tCallBackMapper*) pNextMapper->pNext;
		}

}

