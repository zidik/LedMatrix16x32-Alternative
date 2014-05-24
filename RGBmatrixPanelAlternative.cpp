#include "RGBmatrixPanelAlternative.h"

#define DATAPORT PORTD
#define DATADIR  DDRD
#define SCLKPORT PORTB

#define nBanks 4
#define nPlanes 4 //Minimum 2
#define nBytesInDatarow 64

static RGBmatrixPanelAlt *activePanel = NULL;

RGBmatrixPanelAlt::RGBmatrixPanelAlt(
	uint8_t a, uint8_t b, uint8_t c,
	uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf) :
	Adafruit_GFX(32, 16) {
	

	// Allocate and initialize matrix buffer:
	int buffsize = nBytesInDatarow * nPlanes * nBanks;
	int allocsize = (dbuf == true) ? (buffsize * 2) : buffsize;
	if (NULL == (matrixbuff[0] = (uint8_t *)malloc(allocsize))) return;
	memset(matrixbuff[0], 0, allocsize);
	// If not double-buffered, both buffers then point to the same address:
	matrixbuff[1] = (dbuf == true) ? &matrixbuff[0][buffsize] : matrixbuff[0];
	
	// Save pin numbers for use by begin() method later.
	_a = a;
	_b = b;
	_c = c;
	_sclk = sclk;
	_latch = latch;
	_oe = oe;

	// Look up port registers and pin masks ahead of time,
	// avoids many slow digitalWrite() calls later.
	sclkpin = digitalPinToBitMask(sclk);
	latport = portOutputRegister(digitalPinToPort(latch));
	latpin = digitalPinToBitMask(latch);
	oeport = portOutputRegister(digitalPinToPort(oe));
	oepin = digitalPinToBitMask(oe);
	addraport = portOutputRegister(digitalPinToPort(a));
	addrapin = digitalPinToBitMask(a);
	addrbport = portOutputRegister(digitalPinToPort(b));
	addrbpin = digitalPinToBitMask(b);
	addrcport = portOutputRegister(digitalPinToPort(c));
	addrcpin = digitalPinToBitMask(c);

	buffptr = matrixbuff[0];

	bank = nBanks - 1;
	plane = nPlanes - 1;
}

void RGBmatrixPanelAlt::begin(void) {
	activePanel = this;                      // For interrupt hander

	// Enable all comm & address pins as outputs, set default states:
	pinMode(_sclk, OUTPUT); SCLKPORT &= ~sclkpin;  // Low
	pinMode(_latch, OUTPUT); *latport &= ~latpin;   // Low
	pinMode(_oe, OUTPUT); *oeport |= oepin;     // High (disable output)
	pinMode(_a, OUTPUT); *addraport &= ~addrapin; // Low
	pinMode(_b, OUTPUT); *addrbport &= ~addrbpin; // Low
	pinMode(_c, OUTPUT); *addrcport &= ~addrcpin; // Low

	// The high six bits of the data port are set as outputs;
	// Might make this configurable in the future, but not yet.
	DATADIR = B11111100;
	DATAPORT = 0;

	// Set up Timer1 for interrupt:
	TCCR1A = _BV(WGM11); // Mode 14 (fast PWM), OC1A off
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Mode 14, no prescale
	ICR1 = 100;
	TIMSK1 |= _BV(TOIE1); // Enable Timer1 interrupt
	sei();                // Enable global interrupts
}

// Promote 3/3/3 RGB to Adafruit_GFX 5/6/5
uint16_t RGBmatrixPanelAlt::Color333(uint8_t r, uint8_t g, uint8_t b) {
	// RRRrrGGGgggBBBbb
	return ((r & 0x7) << 13) | ((r & 0x6) << 10) |
		((g & 0x7) << 8) | ((g & 0x7) << 5) |
		((b & 0x7) << 2) | ((b & 0x6) >> 1);
}

void RGBmatrixPanelAlt::drawPixel(int16_t x, int16_t y, uint16_t c) {
	uint8_t r, g, b, bit, limit, *ptr;
	// Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
	// 4/4/4.  Pluck out relevant bits while separating into R,G,B:
	r = c >> 12;        // RRRRrggggggbbbbb
	g = (c >> 7) & 0xF; // rrrrrGGGGggbbbbb
	b = (c >> 1) & 0xF; // rrrrrggggggBBBBb

	// Loop counter stuff
	limit = 1 << nPlanes;


	uint16_t baseaddr = (y % 4) * nBytesInDatarow * nPlanes;
	if (y % 8 < 4){
		baseaddr += ((x >> 3) + 1) * 16 - 1 - x % 8;
	}
	else{
		baseaddr += (x >> 3) * 16 + x % 8;
	}
	ptr = &matrixbuff[0][baseaddr];
	if (y < 8){
		for (bit = 1; bit < limit; bit <<= 1) {
			*ptr &= ~B00011100;             // Mask out R,G,B in one op
			if (r & bit) *ptr |= B00000100;  // Plane N R: bit 2
			if (g & bit) *ptr |= B00001000;  // Plane N G: bit 3
			if (b & bit) *ptr |= B00010000;  // Plane N B: bit 4
			ptr += 64;                  // Advance to next bit plane
		}
	}
	else{

		for (bit = 1; bit < limit; bit <<= 1) {
			*ptr &= ~B11100000;             // Mask out R,G,B in one op
			if (r & bit) *ptr |= B00100000;  // Plane N R: bit 5
			if (g & bit) *ptr |= B01000000;  // Plane N G: bit 6
			if (b & bit) *ptr |= B10000000;  // Plane N B: bit 7
			ptr += 64;                  // Advance to next bit plane
		}
	}
}

void RGBmatrixPanelAlt::fillScreen(uint16_t c) {
	if ((c == 0x0000) || (c == 0xffff)) {
		// For black or white, all bits in frame buffer will be identically
		// set or unset (regardless of weird bit packing), so it's OK to just
		// quickly memset the whole thing:
		memset(matrixbuff[0], c, nBytesInDatarow * nPlanes * nBanks);
	}
	else {
		// Otherwise, need to handle it the long way:
		Adafruit_GFX::fillScreen(c);
	}
}

ISR(TIMER1_OVF_vect, ISR_BLOCK) { // ISR_BLOCK important -- see notes later
	activePanel->updateDisplay();   // Call refresh func for active display
	TIFR1 |= TOV1;                  // Clear Timer1 interrupt flag
}

#define CALLOVERHEAD 60   // Actual value measured = 56
#define LOOPTIME     200  // Actual value measured = 188

void RGBmatrixPanelAlt::updateDisplay(void){
	uint8_t  tick, tock, *ptr;
	uint16_t t, duration;


	//Disable Output during bank switching
	*oeport |= oepin;  // Disable LED output during row/plane switchover
	*latport |= latpin; // Latch data loaded during *prior* interrup

	duration = (((LOOPTIME * 2) + CALLOVERHEAD * 2) << plane) - CALLOVERHEAD;


	plane++;
	if (plane >= nPlanes){
		plane = 0;
		bank++;
		if (bank >= nBanks){
			bank = 0;
			/*Swapping*/
			buffptr = matrixbuff[0];
		}
	}
	else if (plane == 1){
		// Plane 0 was loaded on prior interrupt invocation and is about to
		// latch now, so update the row address lines before we do that:

		if (bank & 0x1)	*addraport |= addrapin;
		else			*addraport &= ~addrapin;
		if (bank & 0x2)	*addrbport |= addrbpin;
		else            *addrbport &= ~addrbpin;
	}


	ICR1 = duration; // Set interval for next interrupt
	TCNT1 = 0;        // Restart interrupt timer

	//SWITCH??
	*oeport &= ~oepin;   // Re-enable output
	*latport &= ~latpin;  // Latch down


	tock = SCLKPORT;
	tick = tock | sclkpin;

	ptr = (uint8_t *)buffptr;


	#define pew asm volatile(               \
		"ld  __tmp_reg__, %a[ptr]+"    "\n\t"   \
		"out %[data]    , __tmp_reg__" "\n\t"   \
		"out %[clk]     , %[tick]"     "\n\t"   \
		"out %[clk]     , %[tock]"     "\n"     \
		:: [ptr]  "e" (ptr),                    \
		[data] "I" (_SFR_IO_ADDR(DATAPORT)),	\
		[clk]  "I" (_SFR_IO_ADDR(SCLKPORT)),	\
		[tick] "r" (tick),						\
		[tock] "r" (tock));

	// Loop is unrolled for speed:
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew
	pew pew pew pew pew pew pew pew

	buffptr += 0;

}

void RGBmatrixPanelAlt::dumpMatrix(void) {

	int i, buffsize = nBytesInDatarow * nPlanes * nBanks;

	Serial.print(" \n --- \n{\n");

	for (i = 0; i<buffsize; i++) {
		if (matrixbuff[0][i] < 0x10) Serial.print(' ');
		if (matrixbuff[0][i] == 0) Serial.print(' ');
		else Serial.print(matrixbuff[0][i], HEX);
		if (i < (buffsize - 1)) {
			if ((i & 63) == 63) Serial.print(",\n  ");
			else             Serial.print(',');
		}
	}
	Serial.println("\n};");
}