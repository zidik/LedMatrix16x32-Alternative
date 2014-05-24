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

uint8_t pixel_x_pos = 31;
uint8_t pixel_y_pos = 15;

RGBmatrixPanelAlt matrix = RGBmatrixPanelAlt(A, B, C, CLK, LAT, OE, false);

void setup()
{
	matrix.begin();
}

void loop()
{
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis > interval) {
		previousMillis = currentMillis;

		//delete last pixel
		matrix.drawPixel((int16_t)pixel_x_pos, (int16_t)pixel_y_pos, 0);

		//Running pixel
		if (++pixel_x_pos >= 32){
			pixel_x_pos = 0;
			if (++pixel_y_pos >= 16){
				pixel_y_pos = 0;
			}
		}
		matrix.drawPixel((int16_t)pixel_x_pos, (int16_t)pixel_y_pos, matrix.Color333(0, pixel_y_pos % 8, 0));
	}
}



