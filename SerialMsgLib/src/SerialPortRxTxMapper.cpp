/*
 * SerialPortRxMapper.cpp
 *
 *  Created on: 13.12.2017
 *      Author: User
 */

#include "SerialPortRxTxMapper.h"
#include "SerialNode.h"


SerialPortRxTxMapper::SerialPortRxTxMapper(SerialPort* pSerialPort) {
	this->pSerialRx = new SerialRx();
	this->pSerialTx = new SerialTx();
	pSerialRx->setPort(pSerialPort);
	pSerialTx->setPort(pSerialPort);
	this->pSerialPort = pSerialPort;
	pSerialRx->setUpdateCallback(SerialNode::update);

}

SerialPortRxTxMapper::SerialPortRxTxMapper(SerialPort* pSerialPort,SerialRx* pSerialRx,SerialTx* pSerialTx) {
	this->pSerialRx = pSerialRx;
	this->pSerialTx = pSerialTx;
	this->pSerialPort = pSerialPort;
	pSerialRx->setUpdateCallback(SerialNode::update);

}

void SerialPortRxTxMapper::createRxBuffer(size_t datasize){

	pSerialRx->createBuffer(sizeof(tSerialHeader) + datasize);

}

SerialPortRxTxMapper::~SerialPortRxTxMapper() {
	// TODO Auto-generated destructor stub
}

