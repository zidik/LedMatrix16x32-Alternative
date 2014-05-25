#include <Adafruit_GFX.h>
#include "RGBmatrixPanelAlternative.h"

#if defined(__AVR_ATmega32U4__)
	#define CLK A3  // MUST be on PORTB (PORTF if using 32u4)
	#define LAT A4
	#define OE  A5

	#define A   A0
	#define B   A1
	#define C   A2
#else
	#define CLK 8  // MUST be on PORTB (PORTF if using 32u4)
	#define LAT A3
	#define OE  9

	#define A   A0
	#define B   A1
	#define C   A2
#endif



//For running LED
long previousMillis = 0;
long interval = 20;

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

RGBmatrixPanelAlt matrix = RGBmatrixPanelAlt(A, B, C, CLK, LAT, OE, true);

const char str[] PROGMEM = "Adafruit 16x32 RGB LED Matrix";
int    textX = matrix.width(),
textMin = sizeof(str)* -12,
hue = 0;
int8_t ball[3][4] = {
	{ 3, 0, 1, 1 }, // Initial X,Y pos & velocity for 3 bouncy balls
	{ 17, 15, 1, -1 },
	{ 27, 4, -1, 1 }
};
static const uint16_t PROGMEM ballcolor[3] = {
	0x0080, // Green=1
	0x0002, // Blue=1
	0x1000  // Red=1
};

void setup()
{
	matrix.begin();
	//matrix.fillScreen(matrix.Color333(1, 0, 0));
	matrix.fillScreen(0);
	for (uint8_t i = 0; i <= 6; i++){
		
		matrix.drawFastHLine(0, i, 16, matrix.Color333(0, i+1, 0));
		matrix.drawFastHLine(16, i, 16, matrix.Color333(i+1, 0, 0));
		matrix.drawFastHLine(0, i+7, 32, matrix.Color333(0, 0, i+1));
		
		/*
		EI tööta päris
		matrix.drawFastHLine(8, i, 8, matrix.Color888(0, (uint8_t)((255 * ((uint16_t)i)) / 8), 0, true));
		matrix.drawFastHLine(24, i, 8, matrix.Color888(255 * (uint16_t)i / 8, 0, 0, true));
		matrix.drawFastHLine(16, i + 7, 16, matrix.Color888(0, 0, 255 * (uint16_t)i / 8, true));
		*/
		//matrix.drawFastHLine(24, i, 8, matrix.Color888((i + 1) << 4, 0, 0));
		//matrix.drawFastHLine(16, i + 7, 16, matrix.Color888(0, 0, (i + 1) << 4));
		
	}

	matrix.drawFastHLine(0, 15, 32, matrix.Color888(255,255,255));
	// Update display
	matrix.swapBuffers(true);
	delay(5000);

	matrix.setTextWrap(false); // Allow text to run off right edge
	matrix.setTextSize(2);

}

void loop()
{
	/*matrix.fillScreen(matrix.Color444(B00001010, 0, 0));
	matrix.swapBuffers(true);
	while (true);
	*/
	/*unsigned long currentMillis = millis();
	
	if (currentMillis - previousMillis > interval) {
		previousMillis = currentMillis;
		runPixel();
	}
	*/

	byte i;

	// Clear background
	matrix.fillScreen(0);

	// Bounce three balls around
	for (i = 0; i<3; i++) {
		// Draw 'ball'
		matrix.fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
		// Update X, Y position
		ball[i][0] += ball[i][2];
		ball[i][1] += ball[i][3];
		// Bounce off edges
		if ((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1)))
			ball[i][2] *= -1;
		if ((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1)))
			ball[i][3] *= -1;
	}

	// Draw big scrolly text on top
	matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
	matrix.setCursor(textX, 1);
	matrix.print(F2(str));

	// Move text left (w/wrap), increase hue
	if ((--textX) < textMin) textX = matrix.width();
	hue += 7;
	if (hue >= 1536) hue -= 1536;

	// Update display
	matrix.swapBuffers(true);
}

void runPixel()
{
	//Startingpoint
	static uint8_t pixel_x_pos = 31;
	static uint8_t pixel_y_pos = 15;

	//delete last pixel
	matrix.drawPixel((int16_t)pixel_x_pos, (int16_t)pixel_y_pos, 0);

	//Caltuclate new pos
	if (++pixel_x_pos >= 32){
		pixel_x_pos = 0;
		if (++pixel_y_pos >= 16){
			pixel_y_pos = 0;
		}
	}
	matrix.drawPixel((int16_t)pixel_x_pos, (int16_t)pixel_y_pos, matrix.Color333(0, pixel_y_pos % 8, 0));


}



