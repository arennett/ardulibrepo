/*
 * OledMessage.cpp
 *
 *  Created on: 26.01.2018
 *      Author: User
 */

#include "OledMessage.h"

OledMessage::OledMessage(){

}
OledMessage::OledMessage(tOledCmd cmd,const byte* _pParams,size_t paramsSize) {
	memset(buffer,0,OLEDMESSAGE_SIZE);
	buffer[0]=cmd;
	buffer[1]=paramsSize;

	if (paramsSize >0) {
		memcpy(&buffer[2],_pParams,paramsSize);
	}

}

OledMessage::~OledMessage() {

}

