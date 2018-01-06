/*
 * SoftSerialTx.h
 *
 *  Created on: 31.10.2017
 *      Author: User
 */

#ifndef SERIALTX_H_
#define SERIALTX_H_
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SerialMsg.h"



namespace SerialMsgLib {

class SerialPort;

class SerialTx {

public:


	/**
	 * SerialTx()
	 * > constructor for serial message transmitter
	 */

	SerialTx();

	/**
	 * SerialTx()
	 * - constructor for serial message transmitter
	 * - using an external SerialPort port
	 * >pSerialPort			*
	*/
	SerialTx(SerialPort* pSerialPort);


	/**
	 * ~SoftSerialTx() {
	 * > Destructor
	 */
	virtual ~SerialTx();



	bool setPort(SerialPort* pSerialPort);


	/**
	 * void sendData(byte* pDdata, size_t datasize);
	 * - sends a byte array to a the serialport
	 *   SerialTx adds a preamble and a postamble
	 *   to the data.
	 * > pData address of the the byte array
	 * > size of data
	 */
	void sendData(const byte* pData, size_t datasize);

	/*
	 * void sendRawData(byte* pData, size_t datasize);
	 * - sends rawData to the SerialPort
	 * - if you like to send multiple dataobjects
	 * - 1 sendPreamble()
	 * - 2 send the dataobjects whith sendRawData
	 * - 3 sendPostamble ()
	 *
	 * > pData		address of the the byte array
	 * > datasize	size of data
	 */
	void sendRawData(const byte* pData, size_t datasize);


	/**
	 * void sendPreamble();
	 * - sends the preamble to the serial port
	 */
	void sendPreamble();

	/**
	 * void sendPreamble();
	 * - sends the prostamble to the serial port
	 */
	void sendPostamble();




	/**
	 * SoftwareSerial* getSoftSerial();
	 * <returns: pointer on internal serial port
	 */
	SerialPort* getSerialPort();


private:
	SerialPort* pSerialPort;
};
};
#endif /* SERIALTX_H_ */
