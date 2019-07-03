#include "U8g2lib.h"

#include "display.h"

// OLED display 128x64 with SSD1306 controller
// on I2C GPIOs SCL 5 and SDA 4
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, 4, 5);

void initDisplay()
{
	// high I2c clock still results in about 100ms buffer transmission to OLED:
	u8g2.setBusClock(1000000);
	u8g2.begin();
}

void drawStrR(u8g2_uint_t x, u8g2_uint_t y, const char *str) {
	u8g2_uint_t w = u8g2.getStrWidth(str);
	u8g2.drawStr(x-w, y, str);
}

void updateDisplay(uint16_t onSecs, uint16_t offSecs, bool on, uint16_t remaining)
{
	u8g2.clearBuffer();

	u8g2.setFont(u8g2_font_crox5h_tf);

	u8g2_uint_t y0 = 28;
	u8g2_uint_t y1 = 60;
	u8g2_uint_t x0 = 62;
	u8g2_uint_t x1 = 126;
	
	char buf[20];
	sprintf(buf, "%d", onSecs);
	drawStrR(x0, y0, buf);

	sprintf(buf, "%d", offSecs);
	drawStrR(x0, y1, buf);

	sprintf(buf, "%d", remaining);
	drawStrR(x1, on ? y0 : y1, buf);

	u8g2.sendBuffer();
}
