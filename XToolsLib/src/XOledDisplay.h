/*
 * XOledDisplay.h
 *
 *  Created on: 30.01.2019
 *      Author: User
 */

#ifndef XOLEDDISPLAY_H_
#define XOLEDDISPLAY_H_
#include <Adafruit_SSD1306.h>
#define XOLED_INACTIVE false
#define XYZ 3
#define XOLED_RESET 4



class XOledDisplay {
public:
	XOledDisplay();
	virtual ~XOledDisplay();
	 //init must be called in setup
	void init();
	void clear();
	void update();
	void drawLine(byte x, byte y, byte x2, byte y2,byte fontColor);
	void drawRectangle(byte x, byte y, byte x2, byte y2,byte fontColor);
	void drawCircle(byte x,byte y,byte r,byte fontColor);
	void drawText(char* pText,byte x,byte y,byte fontSize,byte fontColor);
	// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
	// using the specified foreground color (unset bits are transparent).
	void drawBitMap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t colorFore, uint16_t colorBack);


	Adafruit_SSD1306* m_pDisplay;
};

#endif /* XOLEDDISPLAY_H_ */
