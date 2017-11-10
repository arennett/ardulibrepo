/*
 * SensoMsg.cpp
 *
 *  Created on: 10.11.2017
 *      Author: User
 */

#include "stddef.h"
#include "SensoMsg.h"

	SensoMsg::SensoMsg(bool send) {
		pMsgHdr=NULL;
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
		if (send) {
			MPRINTLN("ERROR SensoMsg::receive not allowed");
			return;
		}
		pMsgHdr=pData;
		if (len != pMsgHdr->dataSize) {
			MPRINTSVAL("WARNING: SensoMsg::receive len <> pMsgHdr->dataSize : " ,pMsgHdr->dataSize);
		}
	}

	void SensoMsg::send(SerialPort* pSerialPort) {
		if (!send) {
			MPRINTLN("WARNING SensoMsg::send a received message");
		}

		pSerialPort->write(serPreamble,sizeof serPreamble);
		pSerialPort->write(pMsgHdr->pData,pMsgHdr->dataSize);
		pSerialPort->write(serPostamble,sizeof serPostamble);
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
	void SensoMsg::setSubCmd(byte subcmd) {
		this->pMsgHdr->subcmd = subcmd;
	}
	;

	byte SensoMsg::getSubCmd() {
		return this->pMsgHdr->subcmd;
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



