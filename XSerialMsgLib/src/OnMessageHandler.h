/*
 * OnMessageHandler.h
 *
 *  Created on: 05.01.2018
 *      Author: rea
 */

#ifndef ONMESSAGEHANDLER_H_
#define ONMESSAGEHANDLER_H_
#include "SerialNode.h"
namespace SerialMsgLib {

class OnMessageHandler {
public:
	OnMessageHandler();
	virtual ~OnMessageHandler();
	virtual void onMessage(const tSerialHeader* pHeader, const unsigned char *pData, unsigned int dataSize, SerialNode *pNode)=0;
};

} /* namespace SerialMsg */

#endif /* ONMESSAGEHANDLER_H_ */
