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




class SerialNode {


public:

	static byte		 systemId ;


	/**
	 * void update(byte* pMessage,size_t messageSize,SerialPort *pPort);
	 * callback routine.
	 * Is called if a message was received by serialRx.
	 * It checks the address, and searches for the node
	 * If a local node is found, it calls the node callback routine.
	 * If the node isn't found , it asks on all ports for
	 * the remote systemId, if found it forwards the message exclusive to the port.
	 * If not found, search for link (see forward) to a port
	 * If no links found, the message is sent to all ports without the ports that have the same remote systemId
	 * as the message fromAddress. This means, it is only forwarded to other systems.
	 * >pMessage 	...complete message: header (+data)
	 * >messageSize	...size of header+data
	 * >pPort  		...the port on which the message was received
	 *
	 */
	static void update(const byte* pMessage,size_t messageSize,SerialPort *pPort);


	/**
	 * forward(const byte* pMessage, size_t messageSize,SerialPort* pSourcePort)
	 * A message received from pSourcePort but no node found in the system
	 * If the node isn't found , it asks on all ports for
	 * the remote systemId, if found it forwards the message exclusive to the port.
	 * If not found, search for link (see forward) to a port
	 * If no links found, the message is sent to all ports without the ports that have the same remote systemId
	 * as the message fromAddress. This means, it is only forwarded to other systems.
	 * In case of connection request:
	 * 1. delete existing link to remote system
	 * 2. create a open link.
	 *    the link is closed by the ACK of the remote system
	 */
	static bool forward(const byte* pMessage, size_t messageSize,
	SerialPort* pSourcePort);


	/**
	 * static void init(byte systemId);
	 * has to be called in the setup routine
	 * sytemId		id of the local system
	 * 				the id must be unique for all systems in the node network
	 */
	static void init(byte systemId);


	/**
	 * SerialNode* createNode(byte localNodeId, bool active=false, byte remoteSysId=0,byte remoteNodeId=0,SerialPort* pSerialPort=NULL);
	 * factory method for SerialNodes
	 * see SerialNode(...)
	 */
	static SerialNode* createNode(byte localNodeId, bool active=false, byte remoteSysId=0,byte remoteNodeId=0,SerialPort* pSerialPort=NULL);

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
	static tAktId writeToPort(tSerialHeader* pHeader,byte* pData, size_t datasize ,SerialPort* pPort);


	/* static SerialNode* GetNodeList();
	 * all instantiated nodes are linked in a node list.
	 * < returns 	...the root node of the node list
	 */
	static SerialNode* getNodeList();



	/**
	 * SerialNode(byte addr,SerialPort* pSerialPort);
	 * - creates a SerialNode with a local address.
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
	SerialNode(byte localNodeId, bool active=false, byte remoteSysId=0,byte remoteNodeId=0,SerialPort* pSerialPort=NULL);

	virtual ~SerialNode();

	/*
	 * bool connect(byte remoteAddress,bool active=false,unsigned long timeOut=0,unsigned long reqPeriod=0);
	 * If the node is connected to another node, the nodes will be first diconnected.
	 * If the node is active (remoteNodeId was passed), it send connection requests (CR) to the other node (remoteAddress)
	 * If the other node also tries to connect (passive) it replies an ACK on this node request.
	 * If the node is passive (no remoteId was passed) it wait for a CR of the active node
	 * > remoteSysId 		...remote systemId 	>0 	try to connect to system with remoteSysId
	 * 											 0  only for passive nodes, connect with nodes from every system
	 * > remoteNodeId		...remote nodeId	>0	connect to a remote node with id == remoteNodeId
	 * 											 0	active node tries to connect to all passive nodes of the remote system
	 * >                                      		which have no remote node id
	 * >										 	passive node waits to be connected from any node of the remote system
	 * >
	 * > timeout			...timeout in msec , default/0 = wait until connect
	 * > reqPeriod        	...period for active connection requests, default 200 msec
	 * < returns 			...true	if the node is connected before timeout expires
	 */
	 bool connect(byte remoteSysId=0,byte remoteNodeId=0,unsigned long timeOut=0,unsigned long checkPeriod=500);


	 /*
	  *  static bool connectNodes(unsigned long timeOut, unsigned long reqPeriod);
	  *  connect all active and passive nodes in the system
	  *  all nodes have to be ready to connect -> setReady(true)
	  *  > timeout		...timeout in msec , default/0 = wait until all nodes connected
	  *  > reqPeriod	...period for active connection requests, default 200 msec
	  *  < returns 		...true	if the nodes are connected before timeout expires
	  */
	 static bool connectNodes(unsigned long timeOut=0, unsigned long reqPeriod=200);


	 /**
	  * static bool areAllNodesConnected();
	  * <  returns 		...true	if all nodes connected , or no node found
	  */
	 static bool areAllNodesConnected();

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
	 * is called by the static update routine if a message for
	 * this node came in
	 * it handles all types of incoming messages
	 * > pHeader	...header
	 * > pData		...optional data
	 * > datasize	...size of optional data
	 * > pPort 		...port on which the message came in
	 */
	static void onMessage(tSerialHeader* pHeader,const byte* pData,size_t datasize,SerialPort* pPort,SerialNode* pNode);


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
	 * > replyToSys     			... remote system id
	 * > replyToNode	0x00		... send to connected remote address
	 * 				    0x**  		... send to unconnected node
	 * < return 		aktid		... > 0 if message was sent
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
	void setReceiveCallBack(void (*ptr)(const tSerialHeader* pHeader,const byte* pData,size_t datasize));


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

	/*  bool isConnected() ;
	 *  if port was set, the node can only connect over that port
	 *  but the node is not connected before the the status is set to connected
	 *  <true if node can communicate to remote node
	 */
	bool isConnected() ;


	/**
	 * public pointer variables
	 */
	void* pNext=NULL; 	// next SerialNode , all nodes are in a linked list
	tCcb* pCcb = NULL;	// connection data
	void (*pCallBack)(const tSerialHeader* pHeader,const byte* pData,size_t datasize)=NULL; // user callback
	SerialPort* pSerialPort =NULL; // if set the node is attached to a port

	static AcbList acbList;
	static LcbList lcbList;
	static SerialNode* pSerialNodeList;

private:



	/*
	 * called by static Update
	 */


};






#endif /* SERIALNODE_H_ */
