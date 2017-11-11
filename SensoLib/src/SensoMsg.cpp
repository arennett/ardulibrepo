/*
 * SensoMsg.cpp
 *
 *  Created on: 10.11.2017
 *      Author: User
 */

#include "stddef.h"
#include <SerialMsg.h>
#include "SensoMsg.h"

	SensoMsg::SensoMsg(bool send) {
		pMsgHdr=NULL;
		this->sendType=send;
		deleteMsgHdr=false;
		if  (send) {
			pMsgHdr = new tSensoMsgHdr();
			deleteMsgHdr=true;
		}
	}

	SensoMsg::~SensoMsg(){

		if (deleteMsgHdr && pMsgHdr) {
			delete pMsgHdr;
		}

	}
	void  SensoMsg::receive(byte* pData ,size_t len){
		if (sendType) {
			MPRINTLN("ERROR SensoMsg::receive - receive not allowed");
			return;
		}
		pMsgHdr=(tSensoMsgHdr*)pData;
		if (len != pMsgHdr->dataSize) {
			MPRINTSVAL("WARNING: SensoMsg::receive - len <> pMsgHdr->dataSize : " ,pMsgHdr->dataSize);
		}
	}

	void SensoMsg::send(SerialTx* pSerialTx) {
		if (!pMsgHdr) {
			MPRINTLN("WARNING SensoMsg::send - no message found");
		}

		if (!sendType) {
			MPRINTLN("WARNING SensoMsg::send - you send a received message");
		}

		pMsgHdr->aktId=aktId++;
		pSerialTx->sendData(pMsgHdr->pData,pMsgHdr->dataSize);
		int s=pMsgHdr->dataSize;
		byte* p = pMsgHdr->pData;
		DPRINTLN("SensoMsg::send: ");
		for (;s--;p++) {
			DPRINTHEX(pData);
		}
		DPRINTLN("");
	}

	void SensoMsg::setDataSize(size_t size) {
		pMsgHdr->dataSize=size;;
	}

	size_t SensoMsg::getDataSize() {
		return pMsgHdr->dataSize;
	}

	tSensoMsgHdr* SensoMsg::getMsgHdr() {
		return pMsgHdr;
	}

	void SensoMsg::setCmd(byte cmd) {
		this->pMsgHdr->cmd = cmd;
	}
	;
	byte SensoMsg::getCmd() {
		return this->pMsgHdr->cmd;
	}
	;
	void SensoMsg::setPar(byte par) {
		this->pMsgHdr->par= par;
	}
	;

	byte SensoMsg::getPar() {
		return this->pMsgHdr->par;
	}

	int SensoMsg::getBitArray() {
		return pMsgHdr->bitArray;
	}

	void SensoMsg::setBitArray(int bits){
		 pMsgHdr->bitArray=bits;
	}

	byte* SensoMsg::getData(){
		return pMsgHdr->pData;
	}

	void SensoMsg::setData(byte* pData){
		pMsgHdr->pData=pData;
	}

	void SensoMsg::setBit(byte num) {
		if (num < 16) {
			bitWrite(pMsgHdr->bitArray, num, 1);
		}
	}
	bool SensoMsg::isBitSet(byte num) {
		if (num < 16) {
			return (pMsgHdr->bitArray & (1 << num));
		}
	}



