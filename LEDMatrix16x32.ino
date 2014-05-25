#include <Adafruit_GFX.h>
#include "RGBmatrixPanelAlternative.h"

#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9

#define A   A0
#define B   A1
#define C   A2


long previousMillis = 0;        // will store last time LED was updated
long interval = 20;           // interval at which to blink (milliseconds)

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
	for (uint8_t i = 0; i <= 7; i++){
		matrix.drawFastHLine(0, 7-i, 16, matrix.Color333(0, 8-i, 0));
		matrix.drawFastHLine(16, 7-i, 16, matrix.Color333(8-i, 0, 0));
		matrix.drawFastHLine(0, 14-i, 32, matrix.Color333(0, 0, 8-i));
	}

	matrix.drawFastHLine(0, 15, 32, matrix.Color333(7,7,7));
	// Update display
	matrix.swapBuffers(true);
	delay(2000);

	matrix.setTextWrap(false); // Allow text to run off right edge
	matrix.setTextSize(2);

}

void loop()
{
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



