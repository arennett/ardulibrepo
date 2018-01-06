/*
 * SerialNodeNet.h
 *
 *  Created on: 02.01.2018
 *      Author: rea
 */

#ifndef SERIALNODENET_H_
#define SERIALNODENET_H_

#include <stddef.h>
#include "SerialNode.h"
#include "OnMessageHandler.h"
#include "OnPreConnectHandler.h"



namespace SerialMsgLib{

class SerialNodeNet {

public:

	virtual ~SerialNodeNet();

	static SerialNodeNet* init(byte systemId);

	static SerialNodeNet* getInstance();

	byte getSystemId();

	/**
	 *  getProcessingNode();
	 * return the node which was currently processed by processNodes();
	 */
	SerialNode* getProcessingNode();

	void setProcessingNode(SerialNode* pNode);

	/**
	 * SerialNode* getRootNode();
	 * return the first node in the node list
	 */
	SerialNode* getRootNode();

	void setRootNode(SerialNode* pNode);


	SerialNode* getNodeByAcb(tAcb* pAcb);


	/**
	 * static bool areAllNodesConnected();
	 * <  returns 		...true	if all nodes connected , or no node found
	 */
	bool areAllNodesConnected();

	/**
	 * void update(byte* pMessage,size_t messageSize,SerialPort *pPort);
	 * Is called if a message was received by serialRx.
	 * It checks the address, and searches for the node
	 * If a local node is found, it calls the onMessage routine of the node
	 * If the node isn't found , it forwards the message.
	 * >pMessage 	...complete message: header (+data)
	 * >messageSize	...size of header+data
	 * >pPort  		...the port on which the message was received
	 *
	 */
	void update(const byte* pMessage, size_t messageSize, SerialPort *pPort);

	/**
	 * forward(const byte* pMessage, size_t messageSize,SerialPort* pSourcePort)
	 * A message received from pSourcePort but no node found in the system
	 * If the node isn't found , it asks on all ports for
	 * the remote systemId, if found it forwards the message exclusive to the port.
	 * If not found, search for link (see forward) to a port
	 * If no links found (which is supposed to be only in case of CR),
	 * the message is sent to all ports without the ports that have the same remote systemId
	 * as the message fromAddress. This means, it is only forwarded to other systems.
	 * In case of connection request (CR):
	 * 1. delete possibly existing link to remote system
	 * 2. create a open link.
	 *    the link is closed by the ACK of the remote system
	 */
	bool forward(const byte* pMessage, size_t messageSize, SerialPort* pSourcePort);


	/**
	 * static void SerialNode::processNodes(bool bLifeCheck);
	 * this routine has to be put into the main loop
	 * it reads on all ports for all nodes in the SerialNodeNet
	 * > lifeCheck	...checks periodically all node connections (recommended)
	 */

	void  processNodes(bool lifeCheck=true);

	/**
	 * SerialNode* createNode(byte localNodeId, bool active=false, byte remoteSysId=0,byte remoteNodeId=0,SerialPort* pSerialPort=NULL);
	 * factory method for SerialNodes
	 * this is the proper method to create new nodes
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
	SerialNode* createNode(byte localNodeId, bool active = false, byte remoteSysId = 0, byte remoteNodeId = 0,SerialPort* pSerialPort = NULL);


	/*
	 * void setOnMessageHandler(OnMessageHandler* pOnMessageHandler)
	 * set your MessageHandler for nodes
	 * (>) pHeader 		Header of the received data, see OnMessageHandler.h
	 * (>) pData		Application Data
	 * (>) datasize		datasize of application data
	 * (>) pNode		node on that message was received
	 */

	void setOnMessageHandler(OnMessageHandler* pOnMessageHandler);

	/*
	 * void callOnMessage(const tSerialHeader* pHeader, const byte* pData, size_t dataSize, SerialNode* pNode);
	 * sets the user onMessageHandler, that handles all application messages
	 */

	void callOnMessage(const tSerialHeader* pHeader, const byte* pData, size_t dataSize, SerialNode* pNode);

	/*
	 * setOnPreConnectHandler(OnPreConnectHandler& rOnPreConnectHandler)
	 * sets a user preConnect handler, where a node can be set ready to connect (setReady())
	 */
	void setOnPreConnectHandler(OnPreConnectHandler* pOnPreConnectHandler);


	/*
	 * void callOnPreConnect(SerialNode* pNode);
	 * is called by onMessage before a node is connected
	 */
	void callOnPreConnect(SerialNode* pNode);


	/*
	 *  void checkConnection(tStamp periodMsec = 1000);
	 *  check for all period msec for each node, if there was a message received
	 *  in the last second (see SERIALONODE_TIMELIFECHECK_LATE), if not it send a life message
	 *  if node is disconnected it sends a CR message
	 *  > period	...time period between the lifeChecks, default 500 msec
	 */
	void checkConnection(SerialNode* pNode,tStamp periodMsec = 500);


private:
	// a single instance of SerialNodeNet
	static SerialNodeNet* pInst;

	SerialNodeNet(byte systemId);

	// the id of the local system
	// the node that is currently processed for life checking
	// if pProcessingNode has concurrent listener ports, the pProcessingNode's port is the current listener
	SerialNode* pProcessingNode=NULL;
	byte systemId=0;
	SerialNode* pSerialNodeList = NULL;
	LcbList lcbList;

	OnMessageHandler* pOnMessageHandler=NULL;
	OnPreConnectHandler* pOnPreConnectHandler=NULL;
};

};

#endif /* SERIALNODENET_H_ */
