/*
 * SerialNode.h
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */
#include <stddef.h>
#include "SerialHeader.h"
#include "SerialTx.h"
#include "SerialRx.h"
#include "AcbList.h"
#include "LcbList.h"

#ifndef SERIALNODE_H_
#define SERIALNODE_H_

#define SERIALNODE_TIME_LIFECHECK_PERIOD_MSEC 				400   	// check nodes all sec
#define SERIALNODE_TIME_LIFECHECK_LATE_MSEC 				800  	// if we didn't hear anything for 3 sec 	->	send LIVE
#define SERIALNODE_TIME_LIFECHECK_LATE_EXPIRED_MSEC 		3000	// if we didn't hear anything for 20 sec 	->	reconnect
#define SERIALNODE_WAIT_FOR_REPLIES

namespace SerialMsgLib {

class SerialNode {

public:


	/**
	 * SerialNode(byte addr,SerialPort* pSerialPort);
	 * - creates a SerialNode with a local address,
	 *  please use SerialNodeNet::createNode to create nodes
	 *
	 * A SeriaNode can transmit and receive messages to other nodes.
	 * To exchange messages the node have to be connected.
	 * All local nodes are directly reachable. All remote nodes are reachable
	 * over the ports. If you do not specify a specific serialport, the node tries to
	 * send or receive on all ports until it is connected and the nodes port is set.
	 * > localNodeId    ... local address of this node . Must be unique for one system
	 * > active			...	true, node sends connection requests to remote node
	 * 					... false nodes waits for connection request from remote
	 * > remoteSystemId	... the id of the remote system. All connected systems have a unique id
	 * 						If set and node is active,  it sends connection requests to the remote node(s).
	 * 						If set and node is passive, it waits for CR from the remote node(s).
	 * 						If not set and node is passive and waits to be connected from any active node
	 * 						If not set and active, send CR to all remote passive system nodes, that wants
	 * 						to connect. (first wins)
	 *
	 * > remoteNodeId	... id of the remote node, see remoteSystemId
	 * 						If set and node is active, only try to connect to one remote node
	 * 						If set and node is passive, only wait for connect to one remote node
	 * 						If not set (or 0) and active try to connect to all remote nodes of a system (remoteSystemId)
	 * 						If not set (or 0) and passive wait for connect to  remote nodes a system (remoteSystemId)
	 * > pSerialPort	... if set the node can only communicate over this port.
	 */
	SerialNode(byte localNodeId, bool active = false, byte remoteSysId = 0, byte remoteNodeId = 0,
				SerialPort* pSerialPort = NULL);


	virtual ~SerialNode();


	/*
	 * void writeToPort(tSerialHeader* pHeader,byte* data, size_t datasize ,SerialPort* pPort);
	 * help routine
	 * creates an aktid if needed
	 * writes the message to a port
	 * > pHeader	...header
	 * > pData		...optional data
	 * > datasize	...size of optional data
	 * > pPort		...port to write to
	 * < returns	...the aktid of the sent header
	 */
	tAktId writeToPort(tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialPort* pPort);

	SerialPort* getPort();

	SerialNode* getNext();

	SerialNode* cycleNextNodeOnPort();

	inline byte getId() {
		return pCcb->localAddr.nodeId;
	}

	/**
	 *  bool reconnect();
	 *  this method is called if a node has to be connected by the checkLifeNodes routine
	 *  A node must be set set to ready state before it will be connected to its remode node
	 *  Therefore an OnPreconnect() callback can be registered to check conditions
	 *  and if conditions are complied , the node hast to be set to ready.
	 *
	 */
	void reconnect();

	/*
	 * bool setReady(bool bReady);
	 * > bReady  	...true the node is ready to be connected
	 */
	void setReady(bool bReady);

	/*
	 * bool SerialNode::setActive(bool bActive)
	 *  set this node as active or passive node
	 *  active node send connection requests to remote passive nodes
	 *  passive nodes waiting to be connected by remote active node
	 *  than you have to call setReady(true) for passive nodes
	 *  > bActive 	...true, set node active
	 *  < true     	...node was set to active
	 *    false		...node was set to passive
	 */
	bool setActive(bool bActive);

	/*
	 * 	bool isActive();
	 *  < return 	...true... active node
	 *    			...false...passive node
	 */
	bool isActive();

	/*
	 * void onMessage(tSerialHeader* pHeader,byte* pData,size_t datasize);
	 * is called by the static update routine of SerialNodeNet if a message for
	 * this node came in
	 * it handles all types of incoming messages
	 * > pHeader	...header
	 * > pData		...optional data
	 * > datasize	...size of optional data
	 * > pPort 		...port on which the message came in
	 */
	void onMessage(tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialPort* pPort);

	/*
	 * bool send(tSerialHeader* pHeader,byte* pData,byte datasize);
	 * sends a message to the remote node.
	 * > cmd		see SerialHeader.h
	 * 				CMD_ACK,CMD_NAK		... confirm, disaffirm (you can use par of header,if a reply is expected)
	 * 				CMD_ACD  			... user command, you can use par, data opt.
	 * 				CMD_ARQ				... user request, you can use par, data opt.
	 * 				CMD_ARP				...	user reply  , you can use par, data opt.
	 * > replyOn    aktid 				... aktid of the received message, see SerialHeader
	 * > par		parameter or subcommand for commands  ACD,ARQ,ARP
	 *
	 * > pData		optionally data
	 * > datasize	size of data
	 * > replyToSys     			... remote system id
	 * > replyToNode	0x00		... send to connected remote address
	 * 				    0x**  		... send to unconnected node
	 * < return 		aktid		... > 0 if message was sent
	 */
	tAktId send(tSerialCmd cmd, tAktId replyOn = 0, byte par = 0, const byte* pData =	NULL, size_t datasize = 0, byte replyToSys = 0, byte replyToNode = 0);

	/**
	 * waits on the reply
	 * > aktid		...aktid of the sent message
	 * > timeout 	...in msec
	 */
	bool waitOnReply(tAktId aktId, tStamp timeout = 500);

	/*
	 * 	bool isReadyToConnect() ;
	 * < returns true if node is ready to be connected
	 *
	 */
	bool isReadyToConnect();

	/*
	 * 	bool isReadyToConnect() ;
	 * < returns true if node is connected
	 *
	 */

	/*  bool isConnected() ;
	 *  if port was set, the node can only connect over that port
	 *  but the node is not connected before the the status is set to connected
	 *  <true if node can communicate to remote node
	 */
	bool isConnected() {
		// if port was set, the node can only connect over that port
		// but the node is not connected before the the status is set to connected
		bool isConnected = pSerialPort && pCcb->status == CONNECTION_STATUS_CONNECTED;

		return isConnected;
	}

	/*
	 * bool SerialNode::isLifeCheckLate();
	 * < true 	...lifecheck is late, lifeCheck will send live now
	 */
	bool isLifeCheckLate();

	/*
	 * bool SerialNode::isLifeCheckExpired();
	 * < true 	...lifecheck is expired, lifeCheck will change connection status now
	 *
	 */
	bool isLifeCheckExpired();



	inline tStamp getLastLifeCheckTime() {
		return lastLiveCheckTimeStamp;
	}

	inline void setLastLifeCheckTime(tStamp stamp) {
		lastLiveCheckTimeStamp=stamp;
	}


	inline tCcb* getCcb() {
		return pCcb;
	}

	inline tAktId getLastSendAktId(){
		return lastSendAcbAktId;
	}

private:
	tCcb* pCcb = NULL;	// connection data

	tStamp lastReceiveTimeStamp = 0; // if older as 1 sec  -> send live (and expect ack or nak)
	//                        ( if nak onMessage() will disconnect)
	// if older as 2 sec  -> set connection status disconnected

	tStamp lastLiveCheckTimeStamp =0;
	tAktId    lastSendAcbAktId = 0;

	void* pNext = NULL; 	// next SerialNode , all nodes are in a linked list
	SerialPort* pSerialPort = NULL; // if set the node is attached to a port


};

};

#endif /* SERIALNODE_H_ */
