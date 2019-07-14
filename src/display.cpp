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

void updateDisplay(uint32_t onSecs, uint32_t offSecs, bool on, uint32_t current, bool autoSwitch)
{
	u8g2.clearBuffer();

	u8g2_uint_t y0 = 26;
	u8g2_uint_t y1 = 54;
	u8g2_uint_t x0 = 70;
	u8g2_uint_t x1 = 128;

	u8g2.setFont(u8g2_font_6x12_mf);
	drawStrR(x0, y0 + 9, "on");
	drawStrR(x0, y1 + 9, "off");

	u8g2.setFont(u8g2_font_10x20_mn);

	char buf[20];
	sprintf(buf, "%ld", (long)onSecs);
	drawStrR(x0, y0, buf);

	sprintf(buf, "%ld", (long)offSecs);
	drawStrR(x0, y1, buf);

	sprintf(buf, "%ld", (long)current);
	drawStrR(x1, on ? y0 : y1, buf);

	if (autoSwitch) {
		u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
		u8g2.drawStr(0, 2 + (on ? y0 : y1), "A");
	} else {
		u8g2.setFont(u8g2_font_6x12_mf);
		u8g2.drawStr(0, 9, "no auto switching");
	}

	u8g2.sendBuffer();
}
