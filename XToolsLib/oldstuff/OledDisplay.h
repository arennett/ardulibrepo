 /*
 * OledDisplay.h
 *
 * encapsulate all display issues
 *  128x64 size display using I2C to communicate
 */


#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H


#include <Adafruit_SSD1306.h>
//#include <SPI.h>
//#include <Wire.h>
#include <Adafruit_GFX.h>



#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define OLED_RESET 4
#define OLED_INACTIVE false


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

class OledDisplay {

public:

    // constructor
    OledDisplay();

    //init must be called in setup
    void init();
    void clear();
    void update();
    void drawLine(byte x, byte y, byte x2, byte y2,byte fontSize);
    void drawRectangle(byte x, byte y, byte x2, byte y2,byte fontSize);
    void drawCircle(byte x,byte y,byte r,byte fontColor);
    void drawText(char* pText,byte x,byte y,byte fontSize,byte fontColor);
    // Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
    // using the specified foreground color (unset bits are transparent).
    void drawBitMap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t colorFore, uint16_t colorBack);


    // this function must be called inside loop()
    void demo();


    void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) ;
    void testdrawchar(void) ;
    void testdrawcircle(void) ;
    void testfillrect(void) ;
    void testdrawtriangle(void) ;
    void testfilltriangle(void) ;
    void testdrawroundrect(void) ;
    void testfillroundrect(void) ;
    void testdrawrect(void) ;
    void testdrawline() ;
    void testscrolltext(void) ;
    void testHello(void);

private:


protected:


};

#endif
