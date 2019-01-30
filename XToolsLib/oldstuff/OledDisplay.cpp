 /*
 * SimpleTimer.cpp
 *
 * SimpleTimer - A timer library for Arduino.
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "OledDisplay.h"

Adafruit_SSD1306 display(OLED_RESET);


OledDisplay::OledDisplay() {
}

void OledDisplay::init() {
	if (OLED_INACTIVE) return;

	// Serial.begin(9600);

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
	 update();
	 delay(100);
	 clear();
	 // init done
}

void OledDisplay::clear() {
	if (OLED_INACTIVE) return;
	display.clearDisplay();
	update();
}

void OledDisplay::update() {
	if (OLED_INACTIVE) return;
	display.display();
}

void OledDisplay::drawLine(byte x, byte y, byte x2, byte y2,byte fontSize) {
	if (OLED_INACTIVE) return;
	display.drawLine(x, y, x2,y2,WHITE);

 }

void OledDisplay::drawRectangle(byte x, byte y, byte x2, byte y2,byte fontSize) {
	if (OLED_INACTIVE) return;
	display.drawRect(x, y, x2-x,y2-y,WHITE);

 }

void OledDisplay::drawCircle(byte x,byte y,byte r,byte fontColor) {
	if (OLED_INACTIVE) return;
	display.drawCircle(x, y,r,fontColor);

 }

void OledDisplay::drawText(char* pText,byte x,byte y,byte fontSize,byte fontColor) {
	if (OLED_INACTIVE) return;
	display.setTextSize(fontSize);
	display.setTextColor(fontColor);
	display.setCursor(x,y);
	display.print(pText);
 }
// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
// using the specified foreground color (unset bits are transparent).
void OledDisplay::drawBitMap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t colorFore, uint16_t colorBack) {
	  display.drawBitmap(x,y,bitmap, w, h, colorFore,colorBack);
}


void OledDisplay::demo() {
	if (OLED_INACTIVE) return;
	 // Show image buffer on the display hardware.
	  // Since the buffer is intialized with an Adafruit splashscreen
	  // internally, this will display the splashscreen.
	  display.display();
	  delay(2000);

	  // Clear the buffer.
	  display.clearDisplay();

	  // draw a single pixel
	  display.drawPixel(10, 10, WHITE);
	  // Show the display buffer on the hardware.
	  // NOTE: You _must_ call display after making any drawing commands
	  // to make them visible on the display hardware!
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw many lines
	  testdrawline();
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw rectangles
	  testdrawrect();
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw multiple rectangles
	  testfillrect();
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw mulitple circles
	  testdrawcircle();
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw a white circle, 10 pixel radius
	  display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  testdrawroundrect();
	  delay(2000);
	  display.clearDisplay();

	  testfillroundrect();
	  delay(2000);
	  display.clearDisplay();

	  testdrawtriangle();
	  delay(2000);
	  display.clearDisplay();

	  testfilltriangle();
	  delay(2000);
	  display.clearDisplay();

	  // draw the first ~12 characters in the font
	  testdrawchar();
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // draw scrolling text
	  testscrolltext();
	  delay(2000);
	  display.clearDisplay();

	  // text display tests
	  display.setTextSize(1);
	  display.setTextColor(WHITE);
	  display.setCursor(0,0);
	  //display.println("Hello, world!");
	  display.setTextColor(BLACK, WHITE); // 'inverted' text
	  display.println(3.141592);
	  display.setTextSize(2);
	  display.setTextColor(WHITE);
	  //display.print("0x"); display.println(0xDEADBEEF, HEX);
	  display.display();
	  delay(2000);
	  display.clearDisplay();

	  // miniature bitmap display
	  display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
	  display.display();
	  delay(1);

	  // invert the display
	  display.invertDisplay(true);
	  delay(1000);
	  display.invertDisplay(false);
	  delay(1000);
	  display.clearDisplay();

	  // draw a bitmap icon and 'animate' movement
	  testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);

   }


void OledDisplay::testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
	if (OLED_INACTIVE) return;
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

//    Serial.print("x: ");
//    Serial.print(icons[f][XPOS], DEC);
//    Serial.print(" y: ");
//    Serial.print(icons[f][YPOS], DEC);
//    Serial.print(" dy: ");
//    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
    }
    display.display();
    delay(200);

    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
   }
}


void OledDisplay::testdrawchar(void) {
	if (OLED_INACTIVE) return;
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}

void OledDisplay::testdrawcircle(void) {
	if (OLED_INACTIVE) return;
  for (int16_t i=display.height()-40; i>display.height()-48; i-=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();

  }
}

void OledDisplay::testHello(void) {
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	//display.println("Hello, world!");
    display.display();
}

void OledDisplay::testfillrect(void) {
	if (OLED_INACTIVE) return;
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    delay(1);
    color++;
  }
}

void OledDisplay::testdrawtriangle(void) {
	if (OLED_INACTIVE) return;
  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
    delay(1);
  }
}

void OledDisplay::testfilltriangle(void) {
	if (OLED_INACTIVE) return;
  uint8_t color = WHITE;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

void OledDisplay::testdrawroundrect(void) {
	if (OLED_INACTIVE) return;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
    delay(1);
  }
}

void OledDisplay::testfillroundrect(void) {
	if (OLED_INACTIVE) return;
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

void OledDisplay::testdrawrect(void) {
	if (OLED_INACTIVE) return;
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
    delay(1);
  }
}

void OledDisplay::testdrawline() {
	if (OLED_INACTIVE) return;
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }
  delay(250);
}

void OledDisplay::testscrolltext(void) {
	if (OLED_INACTIVE) return;
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  //display.println("André");
  display.display();
  delay(1);

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}

