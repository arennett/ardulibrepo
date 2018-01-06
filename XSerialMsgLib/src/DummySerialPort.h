/*
 * DummySerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef DummySerialPort_H_
#define DummySerialPort_H_

#include "Arduino.h"
#include "SerialMsg.h"
#include "SerialPort.h"

namespace SerialMsgLib {

class DummySerialPort: public SerialPort {
public:

	static DummySerialPort* pDummySerialPortList;
	static DummySerialPort*  getListenerPort();

	/**
	 * static byte count();
	 * nr of soft serial ports
	 */
	static byte count();

	/*
	 * static void cycleListenerPort();
	 * switch to next port if we have multiple DummySerialPorts
	 * if listen time expired and no data available
	 * only for software serial ports important
	 */
	static void cycleListenerPort();


	DummySerialPort();

	/**
	 * DummySerialPort(byte pinRx, byte pinTx,byte remoteSysId) ;
	 * >pinRx	receivePin on board
	 * >pinTx	transmitPin on board
	 * >remoteSysId systemId of the remote SerialNodeNet
	 *
	 */
	DummySerialPort(byte pinRx, byte pinTx,byte remoteSysId) ;

	/**
	 * DummySerialPort(byte remoteSysId);
	 * > remoteSysId systemId of the remote SerialNodeNet
	 */
	DummySerialPort(byte remoteSysId);
	virtual ~DummySerialPort();



	/*
	 * DummySerialPort* cycleNextDummySerialPort();
	 * switch to next port if we have multiple DummySerialPorts
	 */
	DummySerialPort* cycleNextDummySerialPort();


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

private:

	};
};

#endif /* DummySerialPort_H_ */
