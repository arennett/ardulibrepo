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
#include "AcbList.h"


#ifndef SERIALNODE_H_
#define SERIALNODE_H_

//Connection Control Block
typedef struct {
	tAddr localAddr;
	tAddr remoteAddr;    //remote address
	#define  CONNECTION_STATUS_NOT_READY  	1
	#define  CONNECTION_STATUS_READY 		2
	#define  CONNECTION_STATUS_DISCONNECTED 3
	#define  CONNECTION_STATUS_CONNECTED 	4
	byte status=0;
	bool active=false;
} tCcb;



class SerialNode {


public:

	static SerialTx  serialTx ;
	static SerialRx  serialRx ;
	static byte		 systemId ;


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
	 * sytemId		id of the local system
	 * 				the id must be unique for all sytem in the nodes network
	 */

	/**
	 * SerialNode::forward(const byte* pMessage, size_t messageSize,SerialPort* pSourcePort)
	 * A message received from pSourcePort but no node found in the system
	 * 1. If  a link (port1 <> port2)  isn't found create a link.
	 * 2. send the to the linked port
	 */
	static bool forward(const byte* pMessage, size_t messageSize,
	SerialPort* pSourcePort);

	static void Init(byte systemId);

	/*
	 * static unsigned int GetNextAktid();
	 * < the next aktid for send
	 */
	static unsigned int GetNextAktId();


	/*
	 * < return the node list
	 */
	static SerialNode* GetNodeList();



	/**
	 * SerialNode(byte addr,SerialPort* pSerialPort);
	 * - creates a SerialNode with a local address.
	 * A SeriaNode can transmit and receive messages to other nodes.
	 * To exchange messages the node have to be connected.
	 * All local nodes are directly reachable. All remote nodes are reachable
	 * over the ports. If you do not specify a specific serialport, the node tries to
	 * send or receive on all ports until it is connected and the nodes port is set.
	 * > lAddr       	... local address of this node . Must be unique in the node net.
	 * > rAddr		 	... remote address of this node.
	 * 						If set, the node sends connection requests to the remote node.
	 * 						If not set or 0, the node is passive and waits to be connected
	 * 						by a connection request of an active node
	 * > pSerialPort	... if set the node can only communicate over this port.
	 */
	SerialNode(byte localNodeId, byte remoteSysId=0,byte remoteNodeId=0,SerialPort* pSerialPort=NULL);

	virtual ~SerialNode();

	/*
	 * bool connect(byte remoteAddress,bool active=false,unsigned long timeOut=0,unsigned long reqPeriod=0);
	 * If the node is connected to another node, the nodes will be first diconnected.
	 * If the node is active, it send connection requests to the other node (remoteAddress)
	 * If the other node also tries to connect (passive) it replies an ACK on this node request.
	 * > remoteAddress 		address of node to be connected	to
	 * >					if		0 the given rAddr is used
	 * > timeout			timeout in msec , 0 wait until connect
	 * > checkPeriod        check period for connection, default 10 msec
	 * < returns 			true	if the nodes are connected before timeout expires
	 */
	 bool connectNode(tAddr remoteAddress,unsigned long timeOut=0,unsigned long checkPeriod=0);

	 static bool connectNodes(unsigned long timeOut, unsigned long reqPeriod);


	/*
	 * bool setReady(bool bReady);
	 * > bReady  true the node is ready to be connected
	 *
	 */
	void setReady(bool bReady);

	/*
	 * bool SerialNode::setActive(bool bActive)
	 *  set this node active for connect
	 *  requests to remote
	 *  after setActive the node is not ready
	 *  you have call setReady(true)
	 *  > bActive 	true, set node active
	 *  < true      node was set to active
	 *    false		node was set to passive
	 *    			or remote address unknown
	 */
	bool setActive(bool bActive);

	bool isActive();


	/*
	 * void onMessage(tSerialHeader* pHeader,byte* pData,size_t datasize);
	 * is called by the static Update Routine if a message for
	 * this node comes in
	 * > pPort 		...port on which the message came in
	 */
	void onMessage(tSerialHeader* pHeader,byte* pData,size_t datasize,SerialPort* pPort);



	/*
	 * bool send(tSerialHeader* pHeader,byte* pData,byte datasize);
	 * sends a message to the remote node.
	 * > cmd		see SerialHeader.h
	 * 				CMD_ACK,CMD_NAK		 confirm, disaffirm
	 * 				CMD_ACD  	... user command , you can use par 	, data opt.
	 * 				CMD_ARQ		... user request , you can use par	, data opt.
	 * 				CMD_ARP		...	user reply   , you can use par	, data opt.
	 * > replyOn    aktid 		... aktid of the received message, see SerialHeader
	 * > par		parameter or subcommand for commands  ACD,ARQ,ARP
	 *
	 * > pData		optionally data
	 * > datasize	size of data
	 * > replyTo	0x00		... send to connected remote address
	 * 				0x**  		... send to unconnected node
	 * < return 	aktid		... > 0 if message was sent
	 */
	tAktId send(tSerialCmd cmd,tAktId replyOn=0,byte par=0,byte* pData=NULL,byte datasize=0, byte replyToSys=0,byte replyToNode=0);


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


	/*
	 * 	bool isReadyToConnect() ;
	 * < returns true if node is ready to be connected
	 *
	 */
	bool isReadyToConnect() ;

	/*
	 * 	bool isReadyToConnect() ;
	 * < returns true if node is connected
	 *
	 */

	bool isConnected() ;


	void* pNext=NULL; 	// next SerialNode , alle nodes are in a linked list
	tCcb* pCcb = NULL;	// connection data
	void (*pCallBack)(tSerialHeader* pHeader,byte* pData,size_t datasize)=NULL; // user callback





private:

	static AcbList acbList;
	static SerialNode* pSerialNodeList;
	static unsigned int serialNodeAktId;

	/*
	 * called by static Update
	 */

	SerialPort* pSerialPort =NULL; // if set, tx and rx only use this port
								   // otherwise they send and listen on all ports

};






#endif /* SERIALNODE_H_ */
