#ifndef RGB_MATRIX_PANEL_ALTERNATIVE_H
#define RGB_MATRIX_PANEL_ALTERNATIVE_H

#include "Arduino.h"
#include "Adafruit_GFX/Adafruit_GFX.h"


class RGBmatrixPanelAlt : public Adafruit_GFX{
public:
	RGBmatrixPanelAlt(uint8_t a, uint8_t b, uint8_t c,
		uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf);

	void begin(void);
	void drawPixel(int16_t x, int16_t y, uint16_t c);
	void fillScreen(uint16_t c);
	void updateDisplay(void);
	void swapBuffers(boolean);
	void dumpMatrix(void);



	uint8_t
		*backBuffer(void);
	uint16_t
		Color333(uint8_t r, uint8_t g, uint8_t b),
		Color444(uint8_t r, uint8_t g, uint8_t b),
		Color888(uint8_t r, uint8_t g, uint8_t b),
		Color888(uint8_t r, uint8_t g, uint8_t b, boolean gflag),
		ColorHSV(long hue, uint8_t sat, uint8_t val, boolean gflag);
	

private:

	uint8_t         *matrixbuff[2];

	// Counters/pointers for interrupt handler:
	volatile uint8_t *buffptr;
	volatile uint8_t bank, plane;

	volatile uint8_t backindex;
	volatile boolean swapflag;
	

	// PORT register pointers, pin bitmasks, pin numbers:
	volatile uint8_t
		*latport, *oeport, *addraport, *addrbport, *addrcport, *addrdport;
	uint8_t
		sclkpin, latpin, oepin, addrapin, addrbpin, addrcpin, addrdpin,
		_sclk, _latch, _oe, _a, _b, _c, _d;
	

};

#endif