/*
 * SoftSerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SOFTSERIALPORT_H_
#define SOFTSERIALPORT_H_

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SerialMsg.h"
#include "SerialPort.h"

namespace SerialMsgLib {

class SoftSerialPort: public SerialPort {
public:

	static SoftSerialPort* pSoftSerialPortList;
	static SoftSerialPort* getListenerPort();
	static SoftSerialPort* pMaster;

	/**
	 * static byte count();
	 * nr of soft serial ports
	 */
	static byte count();

	/*
	 * static void cycleListenerPort();
	 * switch to next port if we have multiple SoftSerialPorts
	 * if listen time expired and no data available
	 * only for software serial ports important
	 */
	static void cycleListenerPort();

	/*
	 * deletes current selection of master port
	 */
	static void resetMaster();


	SoftSerialPort();

	/**
	 * SoftSerialPort(byte pinRx, byte pinTx,byte remoteSysId) ;
	 * >pinRx	receivePin on board
	 * >pinTx	transmitPin on board
	 * >remoteSysId systemId of the remote SerialNodeNet
	 *
	 */
	SoftSerialPort(byte pinRx, byte pinTx,byte remoteSysId) ;

	/**
	 * SoftSerialPort(SoftwareSerial* pSoftwareSerial,byte remoteSysId);
	 * > pSoftwareSerial 	pointer of an SoftwareSerialPort
	 * > remoteSysId systemId of the remote SerialNodeNet
	 */
	SoftSerialPort(SoftwareSerial* pSoftwareSerial,byte remoteSysId);
	virtual ~SoftSerialPort();



	/*
	 * SoftSerialPort* cycleNextSoftSerialPort();
	 * switch to next port if we have multiple SoftSerialPorts
	 */
	SoftSerialPort* cycleNextSoftSerialPort();


	/**
	 * virtual tPortType getType();
	 * < return	port type id, see defines PORTTYPE_ in SerialPort
	 *
	 */
	virtual tPortType getType();

	/**
	 * standard port methods following...
	 */
	virtual byte read();
	virtual bool write(byte b);
	virtual size_t write(const byte* bb,size_t len);

	virtual void begin(long speed);
	virtual bool listen();
	virtual int  available();
	virtual bool isListening();

	/*
	 * if there is more as one SoftSerialPort, you have
	 * to select a master for listening on commands
	 */
	bool isMaster();

	/*
	 *
	 */
	void setMaster();


private:
	SoftwareSerial* pSoftwareSerial;


	};

}

#endif /* SOFTSERIALPORT_H_ */
