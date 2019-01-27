/*
 * WQWriter.h
 *
 *  Created on: 27.01.2019
 *      Author: User
 */

#ifndef WQWRITER_H_
#define WQWRITER_H_

class WQWriter {
public:
	WQWriter();
	void init(uint8_t pin_newdata,int i2c_master_address,void (*onRequestHandler)() );
	void write(tWQMessage message);
	void onRequestEvent(); // to be called from onRequestHandler
	virtual ~WQWriter();

private:
	Queue<tWQMessage> m_wqQueue;
};

#endif /* WQWRITER_H_ */
