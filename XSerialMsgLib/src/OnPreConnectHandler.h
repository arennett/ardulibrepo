/*
 * OnPreConnectHandler.h
 *
 *  Created on: 05.01.2018
 *      Author: rea
 */

#ifndef ONPRECONNECTHANDLER_H_
#define ONPRECONNECTHANDLER_H_
#include "SerialNode.h"
namespace SerialMsgLib {

class OnPreConnectHandler {
public:
	OnPreConnectHandler();
	virtual ~OnPreConnectHandler();
	virtual void onPreConnect(SerialNode *pNode)=0;
};

} /* namespace SerialMsg */

#endif /* ONPRECONNECTHANDLER_H_ */
