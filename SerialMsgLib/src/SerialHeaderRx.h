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

typedef struct { //Connection Control Block
	byte localAddr;
	byte remoteAddr;    //remote address
	#define  CONNECTION_STATUS_NOT_READY  	1
	#define  CONNECTION_STATUS_READY 		2
	#define  CONNECTION_STATUS_DISCONNECTED 3
	#define  CONNECTION_STATUS_CONNECTED 	4
	byte status=0;
	bool master=false;
	void (*pUserCallBack)(const byte* data, size_t data_size);
	void* pNext = NULL;
} tCcb;

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
	 * > byte localAddr   addr of local receiver
	 * > byte remoteAddr  addr of remote transmitter
	 */
	void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte localAddr ,byte remoteAddr);


	/*
	 * 	void addConnection(byte localAddr, byte remoteAddr,bool master);
	 * creates a new Connection and adds it to the
	 * internal list of connections. Both communication endpoints have
	 * to create at least one connection
	 * > localAddr   addr of local  comm. endpoint
	 * > remoteAdrr  addr of remote comm. endpoint
	 * > master if true localAddr sends ConnectionRequest to remote AR
	 */
	void addConnection(byte localAddr, byte remoteAddr,bool master);

	/**
	 * void internalReceive(const byte* pData, size_t data_size);
	 * - is called by serialRx when serialRx receives a message
	 * - serialRx is the underlying receiver
	 */
	void internalReceive(const byte* pData, size_t data_size);


	/*
	 * bool isReadyToConnect(byte localAddr,byte remoteAddr);
	 * - the client is ready and expects a connection request
	 * > localAddr  addr of local  endpoint
	 * > remoteAddr addr of remote endpoint
	 * < true if receiver is ready
	 */
	bool isReadyToConnect(byte localAddr,byte remoteAddr);


	/**
	 * void begin(long speed);
	 * > like Serial.begin , inits serial connection
	 */
	void begin(long speed);

	/**
	 * bool readNext();
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
	};

	/**
	 * ool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout,
			unsigned long checkPeriod, byte addr,tAktId onAktId);
	 * > waits until complete message is received or timeout is expired
	 * >
	 * rpData 		...reference for : pointer on the received data
	 * rdata_size 	...reference data size
	 * timeout		...timeout msecs / 0 = DEFAULT_WAIT_ON_MESSAGE_TIMEOUT
	 * checkPeriod  ...time until next read trial is done
	 * toAddr		...address of the message
	 * onAktId      ...if > 0 the aktId is checked, we expect a reply
	 */
	bool waitOnMessage(byte*& rpData, size_t& rdata_size, unsigned long timeout,
			unsigned long checkPeriod, byte toAddr,tAktId onAktId);

	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	 * - waits until complete message is received or timeout is expired
	 * - the default checkPeriod is 10msec
	 * ppData 		...reference for : pointer on the received data
	 * rdata_size 	...reference data size
	 * timeout		...timeout msecs
	 * toAddr		...address of the message
	 * onAktId      ...if > 0 the aktId is checked, we expect a reply
	 *
	 */
	bool waitOnMessage(byte*& rpData, size_t& rdata_size, unsigned long timeout,
			byte toAdddr,tAktId onAktId);


	/*
	 * connect(unsigned long timeout,unsigned long reqPeriod);
	 * waits until all connections are up
	 * both sides (master, client) have
	 * > timeout : wait max timeout in msecs
	 * > period  : trial period for a master connection request
	 */
	bool connect(unsigned long timeout,unsigned long reqPeriod);


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

	tCcb* setConnectionStatus(byte localAddr, byte remoteAddr,byte status);

	byte getConnectionStatus(byte localAddr, byte remoteAddr);

	bool isConnected(byte localAddr,byte remoteAddr);


	bool connect(unsigned int timeout=WAITED_READ_TIMEOUT_DEFAULT_MSEC, unsigned long int reqPeriod=100);

	/*
	 * tCallBackMapper* getCallBackMapperEntry(byte addr);
	 * - get the CallBackMapper for addr
	 * > localAddr
	 * > remoteAddr
	 * > create if nothing found
	 */
	tCcb* getCcbEntry(byte localAddr ,byte remoteAddr,bool create=false);

	void mprintCcbList();

	void mprintCcb(tCcb* pCcb);



protected:



private:

	/**
	 * void getLastCallBackMapperEntry();
	 * - iterates through the mapper list, and returns the last entry
	 * < returns the last entry or null if list is empty*/
	tCcb* getLastCcbEntry();



	/*
	 * void deleteCcbList(){
	 * - deletes all entries from  the list
	 */
	void deleteCcbList();



	SerialRx* pSerialRx = NULL;
	SerialHeaderTx 	*pSerialHeaderTx		= NULL;
	tCcb *pCcbList 	= NULL;






};

#endif /* SERIALHEADERRX_H_ */
