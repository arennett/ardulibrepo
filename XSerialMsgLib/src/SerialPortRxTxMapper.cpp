/*
 * SerialPortRxMapper.cpp
 *
 *  Created on: 13.12.2017
 *      Author: User
 */
#include "SerialPortRxTxMapper.h"
#include "SerialNode.h"
#include "SerialNodeNet.h"

namespace SerialMsgLib {
SerialPortRxTxMapper::SerialPortRxTxMapper(){
	pSerialRx=NULL;
	pSerialTx=NULL;
	pSerialPort=NULL;
	XPRINTLNSVAL("SerialPortRxTxMapper::SerialPortRxTxMapper() free ",freeRam());
}

SerialPortRxTxMapper::~SerialPortRxTxMapper() {
	delete pSerialRx;
	delete pSerialTx;

}

SerialPortRxTxMapper::SerialPortRxTxMapper(SerialPort* pSerialPort) {

	pSerialRx = new SerialRx();

	pSerialTx = new SerialTx();

	pSerialRx->setPort(pSerialPort);
	pSerialTx->setPort(pSerialPort);

	this->pSerialPort = pSerialPort;
	//pSerialRx->setUpdateCallback(SerialNodeNet::getInstance()->update);
	XPRINTLNSVAL("SerialPortRxTxMapper::SerialPortRxTxMapper() free ",freeRam());

}

SerialPortRxTxMapper::SerialPortRxTxMapper(SerialPort* pSerialPort,SerialRx* pSerialRx,SerialTx* pSerialTx) {
	return;
	this->pSerialRx = pSerialRx;
	this->pSerialTx = pSerialTx;
	this->pSerialPort = pSerialPort;
	//pSerialRx->setUpdateCallback(SerialNodeNet::getInstance()->update);
	XPRINTLNSVAL("SerialPortRxTxMapper::SerialPortRxTxMapper() free ",freeRam());
}

void SerialPortRxTxMapper::createRxBuffer(size_t datasize){
	pSerialRx->createBuffer(sizeof(tSerialHeader) + datasize);

}

SerialRx* 	SerialPortRxTxMapper::getRx(){
	return pSerialRx;
}
SerialTx*	SerialPortRxTxMapper::getTx(){
	return pSerialTx;
}
SerialPort* SerialPortRxTxMapper::getPort(){
	return pSerialPort;
}
};


