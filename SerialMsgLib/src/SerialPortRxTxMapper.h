/*
 * SerialPortRxMapper.h
 *
 *  Created on: 13.12.2017
 *      Author: User
 */

#ifndef SERIALPORTRXTXMAPPER_H_
#define SERIALPORTRXTXMAPPER_H_


#include "SerialRx.h"
#include "SerialTx.h"

class SerialPort;

class SerialPortRxTxMapper {
public:
	SerialPortRxTxMapper(SerialPort* pSerialPort);
	SerialPortRxTxMapper(SerialPort* pSerialPort,SerialRx* pSerialRx,SerialTx* pSerialTx);
	virtual ~SerialPortRxTxMapper();
	void createRxBuffer(size_t datasize);
	SerialRx* pSerialRx;
	SerialTx* pSerialTx;

	SerialPort* pSerialPort;

};

#endif /* SERIALPORTRXTXMAPPER_H_ */
