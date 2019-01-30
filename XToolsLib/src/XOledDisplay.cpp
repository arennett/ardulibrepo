/*
 * XOledm_display.cpp
 *
 *  Created on: 30.01.2019
 *      Author: User
 */
#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "XTools.h"
#include "XOledDisplay.h"

XOledDisplay::XOledDisplay() {
	m_pDisplay = new Adafruit_SSD1306(XOLED_RESET);

}

XOledDisplay::~XOledDisplay() {
	delete m_pDisplay;
}

void XOledDisplay::init() {
	if (XOLED_INACTIVE) return;

	// Serial.begin(9600);

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	 m_pDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
	 update();
	 delay(200);
	 clear();
	 // init done
}

void XOledDisplay::clear() {
	if (XOLED_INACTIVE) return;
	m_pDisplay->clearDisplay();
	update();
}

void XOledDisplay::update() {
	if (XOLED_INACTIVE) return;
	m_pDisplay->display();
}

void XOledDisplay::drawLine(byte x, byte y, byte x2, byte y2,byte fontColor) {
	if (XOLED_INACTIVE) return;
	m_pDisplay->drawLine(x, y, x2,y2,fontColor);

 }

void XOledDisplay::drawRectangle(byte x, byte y, byte x2, byte y2,byte fontColor) {
	if (XOLED_INACTIVE) return;
	m_pDisplay->drawRect(x, y, x2-x,y2-y,fontColor);

 }

void XOledDisplay::drawCircle(byte x,byte y,byte r,byte fontColor) {
	if (XOLED_INACTIVE) return;
	m_pDisplay->drawCircle(x, y,r,fontColor);

 }


void XOledDisplay::drawText(char* pText,byte x,byte y,byte fontSize,byte fontColor) {
	if (XOLED_INACTIVE) return;
	m_pDisplay->setTextSize(fontSize);
	m_pDisplay->setTextColor(fontColor);
	m_pDisplay->setCursor(x,y);
	m_pDisplay->print(pText);
 }
// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
// using the specified foreground color (unset bits are transparent).
void XOledDisplay::drawBitMap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t colorFore, uint16_t colorBack) {
	m_pDisplay->drawBitmap(x,y,bitmap, w, h, colorFore,colorBack);
}
