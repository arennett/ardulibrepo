/*
 * WQReader.h
 *
 *  Created on: 27.01.2019
 *      Author: User
 */

#ifndef WQREADER_H_
#define WQREADER_H_



class WQReader {
public:


	WQReader(uint8_t pin_newdata,int i2c_master_address);
	virtual ~WQReader();
	void init();

	bool process (tWQMessage& message);

private:
	uint8_t m_pin_newdata;
	int     m_i2c_master_address;


};

#endif /* WQREADER_H_ */
