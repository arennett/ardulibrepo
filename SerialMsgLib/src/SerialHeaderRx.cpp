/*
 * SerialHeaderRx.cpp
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#include "SerialHeader.h"
#include "SerialRx.h"
#include "SerialHeaderRx.h"

SerialHeaderRx::SerialHeaderRx(SerialPort* pSerialPort, size_t maxDataSize) {
	this->pSerialPort = pSerialPort;
	this->pSerialRx = new SerialRx(pSerialPort,maxDataSize + sizeof(tSerialHeader));
	((SerialRx*)pSerialRx)->setSerialHeaderRx(this);

}

SerialHeaderRx::~SerialHeaderRx(){
	delete pSerialRx;
	deleteCallBackList();
}

void SerialHeaderRx::setUpdateCallback(
	void (*ptr)(const byte* pData, size_t data_size), byte addr) {
	tCallBackMapper* pLast = getLastCallBackMapperEntry();
	tCallBackMapper* pNext =new tCallBackMapper();;
	if (pLast == NULL) {
		pCallBackMapperList=pNext;
	}else{
		pLast->pNext=pNext;
	}
	pNext->addr = addr;
	pNext->pUserCallBack = ptr;
}

void SerialHeaderRx::internalCallBack(const byte* pData, size_t data_size) {
	tSerialHeader* pSerialHeader;
	pSerialHeader=(tSerialHeader*)pData;
	const byte* pUserData = pData + sizeof(pSerialHeader);
	tCallBackMapper* pNext = pCallBackMapperList;

	while (pNext != NULL) {
		if (pSerialHeader->addrTo==pNext->addr) {
			pNext->pUserCallBack(pUserData,data_size-sizeof (tSerialHeader));
		}
		pNext =(tCallBackMapper*) pNext->pNext;
	}
}

tCallBackMapper* SerialHeaderRx::getLastCallBackMapperEntry() {
	tCallBackMapper* pLast= pCallBackMapperList;
	while (pLast != NULL && pLast->pNext !=NULL) {
		pLast =(tCallBackMapper*) pLast->pNext;
	}
	return pLast;
}

void SerialHeaderRx::deleteCallBackList(){
	/* recursive way ->bad
	if (pEntry!=NULL) {
		if (((tCallBackMapper*)pEntry)->pNext !=NULL) {
			deleteCallBackList(((tCallBackMapper*)pEntry)->pNext);
		}
		delete pEntry;
	}
	*/
	tCallBackMapper* pLastEntry;
	while ((pLastEntry=getLastCallBackMapperEntry())!=NULL) {
		delete pLastEntry;
	}

}



