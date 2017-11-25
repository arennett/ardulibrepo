/*
 * SerialNode.h
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */
#include <stddef.h>
#include "SerialPort.h"
#include "SerialTx.h"
#include "SerialRx.h"
#include "SerialHeader.h"

#ifndef SERIALNODE_H_
#define SERIALNODE_H_


typedef struct { //Connection Control Block
	byte localAddr;
	byte remoteAddr;    //remote address
	#define  CONNECTION_STATUS_NOT_READY  	1
	#define  CONNECTION_STATUS_READY 		2
	#define  CONNECTION_STATUS_DISCONNECTED 3
	#define  CONNECTION_STATUS_CONNECTED 	4
	byte status=0;
	bool master=false;
} tCcb;

SerialTx  serialTx ;
SerialRx  serialRx ;








class SerialNode {


public:

	/**
	 * void Update(byte* pMessage,size_t messageSize,SerialPort *pPort);
	 * callback routine.
	 * is called if a message was received by serialRx.
	 * It checks the address, and searches for the node
	 * if a local node found, it calls the nodes callback routine.
	 * If node not found , it asks on all ports for
	 * this address, if found it forwards the message.
	 * >pPort  the port on which the message was received
	 */

	static void Update(const byte* pMessage,size_t messageSize,SerialPort *pPort);

	/**
	 * static void Init();
	 * has to be called in the setup routine
	 * > maxDataSize 	size of optional data (see pData)
	 * 					dont care about the header
	 */
	static void Init(size_t maxDataSize);

	/*
	 * static unsigned int GetNextAktid();
	 * < the next aktid for send
	 */
	static unsigned int GetNextAktId();


	/**
	 * SerialNode(byte addr,SerialPort* pSerialPort);
	 * - creates a SerialNode with a local address.
	 * A SeriaNode can transmit and receive messages to other nodes.
	 * To exchange messages the node have to be connected.
	 * All local nodes are directly reachable. All remote nodes are reachable
	 * over the ports. If you do not specify a specific serialport, the node tries to
	 * send or receive on all ports until it is connected and the nodes port is set.
	 * > address       	... address of this node . Must be unique in the node net.
	 * > pSerialPort	... if set the node can only communicate over this port.
	 */
	SerialNode(byte address,SerialPort* pSerialPort=NULL);

	virtual ~SerialNode();

	/*
	 * bool connect(byte remoteAddress,bool active=false,unsigned long timeOut=0,unsigned long reqPeriod=0);
	 * If the node is connected to another node, the nodes will be first diconnected.
	 * If the node is active, it send connection requests to the other node (remoteAddress)
	 * If the other node also tries to connect (passive) it replies an ACK on this node request.
	 * > remoteAddress 		address of node to be connected	to
	 * > active				true, 	the node send CR messages.
	 * 						false	the node is listening, and wants to be connected
	 * < returns 			true	if the nodes are connected before timeout expires
	 */
	bool connect(byte remoteAddress,bool active=true,unsigned long timeOut=0,unsigned long reqPeriod=0);


	/*
	 * void setPort(SerialPort* pPort);
	 * set the port for this node
	 */
	void setPort(SerialPort* pPort);

	/*
	 * bool send(tSerialHeader* pHeader,byte* pData,byte datasize);
	 * sends a message to the remote node.
	 * > cmd		see SerialHeader.h
	 * 				CMD_ACK,CMD_NAK		 confirm, disaffirm
	 * 				CMD_ACD  	... user command , you can use par 	, data opt.
	 * 				CMD_ARQ		... user request , you can use par	, data opt.
	 * 				CMD_ARP		...	user reply   , you can use par	, data opt.
	 * > par		parameter or subcommand for commands  ACD,ARQ,ARP
	 *
	 * > pData		optionally data
	 * > datasize	size of data
	 * > replyOn    aktid 		... aktid of the received message, see SerialHeader
	 * > replyTo	0x00		... send to connected remote address
	 * 				0x**  		... send to unconnected node
	 * < return 	aktid		... > 0 if message was sent
	 */
	tAktId send(tSerialCmd cmd,byte par=0,byte* pData=NULL,byte datasize=0,tAktId replyOn=0, byte replyTo=0);


	/**
	 * waits on the reply
	 * > aktid		...aktid of the sent message
	 * > timeout 	...in msec
	 */
	bool  waitOnReply(tAktId aktId,unsigned long timeout);


	/*
	 * void setReceiveCallBack(void (*ptr)(byte* pData,size_t datasize));
	 * set your callback method for this node.
	 * (>) pHeader 		Header of the received data, see SerialHeader.h
	 * (>) pData		Application Data
	 * (>) datasize		datasize of application data
	 */
	void setReceiveCallBack(void (*ptr)(tSerialHeader* pHeader,byte* pData,size_t datasize));


	void* pNext=NULL; 	// next SerialNode , alle nodes are in a linked list
	tCcb* pCcb = NULL;	// connection data
	void (*pCallBack)(tSerialHeader* pHeader,byte* pData,size_t datasize)=NULL; // user callback
	static SerialNode* pNodeList=NULL;

private:


	/*
	 * called bye
	 */
	bool internalReceive(byte*& pData,size_t datasize);


	SerialPort* pSerialPort =NULL; // if set, tx and rx only use this port
								   // otherwise they send and listen on all ports




};



void serialRxCallBack(byte* pData, size_t datasize);


#endif /* SERIALNODE_H_ */
