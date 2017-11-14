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

typedef struct {
	byte addr;
	void (*pUserCallBack)(const byte* data, size_t data_size);
	void* pNext = NULL;
} tCallBackMapper;

class SerialHeaderRx {
public:
	SerialHeaderRx(SerialPort* pSerialPort, size_t maxDataSize);
	virtual ~SerialHeaderRx();

	/*
	 * void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte addr);
	 * here the user registers one ore more static callback methods
	 * the callback method is only called if the received addrTo is equal to addr
	 */
	void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),
			byte addr);

	/**
	 * void internalCallBack(const byte* pData, size_t data_size);
	 * - is called by serialTx when serialTx receives a message
	 */
	void internalCallBack(const byte* pData, size_t data_size);

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
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs
	 * checkPeriod  ...time until next read trial is done
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout,
			unsigned long checkPeriod, byte addr);

	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	 * > waits until complete message is received or timeout is expired
	 * > the checkPeriod is 10msec
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs
	 *
	 *
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout,
			byte addr);

	/**
	 * bool listen ();
	 * >if multiple software serials are used, listen
	 * >activate this software serial connection
	 * >since they are concurrent
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
	;

private:

	/**
	 * void getLastCallBackMapperEntry();
	 * - iterates through the mapper list, and returns the last entry
	 * < returns the last entry or null if list is empty*/
	tCallBackMapper* getLastCallBackMapperEntry();

	/*
	 * void deleteCallBackList(){
	 * - deletes all entries from  the list
	 */
	void deleteCallBackList();

	SerialRx* pSerialRx = NULL;
	tCallBackMapper *pCallBackMapperList = NULL;


};

#endif /* SERIALHEADERRX_H_ */
