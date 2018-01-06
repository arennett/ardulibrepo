/*
 * SerialPortRxMapper.h
 *
 *  Created on: 13.12.2017
 *      Author: User
 */

#ifndef SERIALPORTRXTXMAPPER_H_
#define SERIALPORTRXTXMAPPER_H_
#include <stddef.h>

namespace SerialMsgLib {

class SerialTx;
class SerialRx;
class SerialPort;
class SerialPortRxTxMapper {
public:
	SerialPortRxTxMapper();
	SerialPortRxTxMapper(SerialPort* pSerialPort);
	SerialPortRxTxMapper(SerialPort* pSerialPort,SerialRx* pSerialRx,SerialTx* pSerialTx);
	virtual ~SerialPortRxTxMapper();
	void createRxBuffer(size_t datasize);
	SerialRx* 	getRx();
	SerialTx*	getTx();
	SerialPort* getPort();


private:
	SerialPort* pSerialPort;
	SerialRx* pSerialRx;
	SerialTx* pSerialTx;

};
};
#endif /* SERIALPORTRXTXMAPPER_H_ */
