/*
 * SerialHeaderRx.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADERRX_H_
#define SERIALHEADERRX_H_

#include <stddef.h>
#include "SerialRx.h"
#include "SerialPort.h"
#include "SoftSerialPort.h"
#include "SerialHeaderTx.h"

typedef struct {
	byte addr;
#define  CALLBACKMAPPER_STATUS_NOT_READY  0
#define  CALLBACKMAPPER_STATUS_READY 2
#define  CALLBACKMAPPER_STATUS_DISCONNECTED 3
#define  CALLBACKMAPPER_STATUS_CONNECTED 4


	byte status;
	void (*pUserCallBack)(const byte* data, size_t data_size);
	void* pNext = NULL;
} tCallBackMapper;

class SerialHeaderRx {
public:
	SerialHeaderRx(SerialPort* pSerialPort, size_t maxDataSize);
	virtual ~SerialHeaderRx();

	/*
	 * void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte addr);
	 * - here the user registers one ore more static callback methods
	 * - the callback method is only called if the received addrTo is equal to addr
	 * - you can register one callback per receiver addr
	 * > (*ptr) static call back function ptr
	 * > addr   addr of receiver
	 */
	void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte addr);

	/**
	 * void internalReceive(const byte* pData, size_t data_size);
	 * - is called by serialRx when serialRx receives a message
	 */
	void internalReceive(const byte* pData, size_t data_size);


	/*
	 * void isReadyToConnect(byte addr);
	 * - the receiver is ready and expects a connection request
	 * - this is typically the last command in the setup and must
	 * - be called after the setUpdateCallback for this addr
	 * > addr addr of the receiver
	 * < true if receiver is ready
	 */
	bool isReadyToConnect(byte addr);




	/**
	 * void begin(long speed);
	 * > like Serial.begin , inits serial connection
	 */
	void begin(long speed);

	/**
	 * readNext();
	 * > reads next byte into the buffer
	 * < return : true when data complete and callBack was called
	 */
	inline bool readNext() {
		return pSerialRx->readNext();
	}
	;

	/**
	 * bool readNext(byte* b);
	 * > reads next byte into the buffer
	 * < return : true when data complete and callBack was called
	 * b	...current byte (can be data or pre-/postamble bytes
	 */

	inline bool readNext(byte* b) {
		return pSerialRx->readNext(b);
	}
	;

	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	 * > waits until complete message is received or timeout is expired
	 * >
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs / 0 = DEFAULT_WAIT_ON_MESSAGE_TIMEOUT
	 * checkPeriod  ...time until next read trial is done
	 * onAktId      ...if > 0 the aktId is checked, we expect a reply
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout,
			unsigned long checkPeriod, byte addr,tAktId onAktId);

	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	 * - waits until complete message is received or timeout is expired
	 * - the default checkPeriod is 10msec
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs
	 * onAktId      ...if > 0 the aktId is checked, we expect a reply
	 *
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout,
			byte addr,tAktId onAktId);

	/**
	 * bool listen ();
	 * - if multiple software serials are used, listen
	 * - activate this software serial connection
	 * - since they are concurrent
	 * <returns: true ...if other connection was deactivated
	 */
	inline bool listen() {
		return pSerialRx->listen();
	}
	;

	/**
	 *  SerialPort* getSerialPort()
	 * <returns: pointer on internal port
	 */
	SerialPort* getSerialPort() {
		return pSerialRx->getSerialPort();
	}

	/**
	 * SerialHeaderTx is called when a
	 * message receives to check if an answer came in
	 *
	 */
	void setSerialHeaderTx(SerialHeaderTx* pSerialHeaderTx);

private:

	/**
	 * void getLastCallBackMapperEntry();
	 * - iterates through the mapper list, and returns the last entry
	 * < returns the last entry or null if list is empty*/
	tCallBackMapper* getLastCallBackMapperEntry();


	/*
	 * tCallBackMapper* getCallBackMapperEntry(byte addr);
	 * - get the CallBackMapper for addr
	 * > addr of the receiver
	 */
	tCallBackMapper* getCallBackMapperEntry(byte addr);

	/*
	 * void deleteCallBackList(){
	 * - deletes all entries from  the list
	 */
	void deleteCallBackList();

	SerialRx* pSerialRx = NULL;
	SerialHeaderTx 	*pSerialHeaderTx		= NULL;
	tCallBackMapper *pCallBackMapperList 	= NULL;


	/*
	 *void setConnected(bool connected);
	 * - after CR is received the receiver call back entry
	 *  is set on status connected if receiver is ready
	 *  to connected
	 *  see isReadyToConnected()
	 *  If the receiver is disconnected (connected=false)
	 *  you must setReadyToConnected(true) before you
	 *  can reconnect;
	 *
	 */
	void setConnected(bool connected,byte addr);

	bool isConnected(byte addr);

};

#endif /* SERIALHEADERRX_H_ */
