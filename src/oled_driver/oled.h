/*
 * Copyright (c) 2015, Vladimir Komendantskiy
 * MIT License
 *
 * IOCTL interface to SSD1306 modules.
 *
 * Command sequences are sourced from an Arduino library by Sonal Pinto.
 */

#ifndef OLED_H
#define OLED_H

#include <stdint.h>
//#include "font.h"
#define USE_OLED_TYPE		2 //1-0.96inch,2-0.91inch
#define OLED_I2C_ADDR                   0x3c

// Control byte
#define OLED_CTRL_BYTE_CMD_SINGLE       0x80
#define OLED_CTRL_BYTE_CMD_STREAM       0x00
#define OLED_CTRL_BYTE_DATA_STREAM      0x40
// Fundamental commands (page 28)
#define OLED_CMD_SET_CONTRAST           0x81
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF
// Addressing Command Table (page 30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_COLUMN_RANGE       0x21
#define OLED_CMD_SET_PAGE_RANGE         0x22
// Hardware Config (page 31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3
#define OLED_CMD_SET_COM_PIN_MAP        0xDA
#define OLED_CMD_NOP                    0xE3
// Timing and Driving Scheme (page 32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5
#define OLED_CMD_SET_PRECHARGE          0xD9
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB
// Charge Pump (page 62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D
// SH1106 Display
#define OLED_SET_PAGE_ADDRESS            0xB0

#if USE_OLED_TYPE ==1
static const unsigned char display_config[] = {
	OLED_CTRL_BYTE_CMD_STREAM,
	OLED_CMD_DISPLAY_OFF,

	OLED_SET_PAGE_ADDRESS,
	0x02, /*set lower column address*/
	0x10, /*set higher column address*/

	OLED_CMD_SET_MUX_RATIO, 0x3F,
	// Set the display offset to 0
	OLED_CMD_SET_DISPLAY_OFFSET, 0x00,
	// Display start line to 0
	OLED_CMD_SET_DISPLAY_START_LINE,
	// Mirror the x-axis. In case you set it up such that the pins are north.
	// 0xA0 - in case pins are south - default
	OLED_CMD_SET_SEGMENT_REMAP,
	// Mirror the y-axis. In case you set it up such that the pins are north.
	// 0xC0 - in case pins are south - default
	OLED_CMD_SET_COM_SCAN_MODE,
	// Default - alternate COM pin map
	OLED_CMD_SET_COM_PIN_MAP, 0x12,
	// set contrast
	OLED_CMD_SET_CONTRAST, 0x7F,
	// Set display to enable rendering from GDDRAM (Graphic Display Data RAM)
	OLED_CMD_DISPLAY_RAM,
	// Normal mode!
	OLED_CMD_DISPLAY_NORMAL,
	// Default oscillator clock
	OLED_CMD_SET_DISPLAY_CLK_DIV, 0x80,
	// Enable the charge pump
	OLED_CMD_SET_CHARGE_PUMP, 0x14,
	// Set precharge cycles to high cap type
	OLED_CMD_SET_PRECHARGE, 0x22,
	// Set the V_COMH deselect volatage to max
	OLED_CMD_SET_VCOMH_DESELCT, 0x30,
	// Horizonatal addressing mode - same as the KS108 GLCD
	OLED_CMD_SET_MEMORY_ADDR_MODE, 0x00,
	// Turn the Display ON
	OLED_CMD_DISPLAY_ON
};
#else
static const unsigned char display_config[] = {
	OLED_CTRL_BYTE_CMD_STREAM,
	OLED_CMD_DISPLAY_OFF,
	//OLED_SET_PAGE_ADDRESS,
	0x40, /*set lower column address*/
	0xB0, /*set higher column address*/
	0xC8,
	0x81,
	0xff,
	0xa1,
	0xa6,
	0xa8,
	0x1f,
	0xd3,
	0x00,
	0xd5,
	0xf0,
	0xd9,
	0x22,
	0xda,
	0x02,
	0xdb,
	0x49,
	0x8d,
	0x14,
	OLED_CMD_DISPLAY_ON
};
#endif
static const unsigned char display_draw[] = {
	OLED_CTRL_BYTE_CMD_STREAM,
	// column 0 to 127
	OLED_CMD_SET_COLUMN_RANGE,
	0x00,
	0x7F,
	// page 0 to 7
	OLED_CMD_SET_PAGE_RANGE,
	0x00,
	0x07
};
#define Hor_size 128
#if USE_OLED_TYPE ==1
#define Ver_size 64
#elif USE_OLED_TYPE ==2
#define Ver_size 32
#endif
#define Hor_center 	Hor_size / 2
#define Ver_center 	Ver_size / 2
struct display_info {
	int address;
	int file;
	//struct font_info font;
	uint8_t buffer[Ver_size/8][128];
};

struct sized_array {
	int size;
	const uint8_t* array;
};
#define TL_DATUM 0    // Top left (default)
#define TC_DATUM 1    // Top centre
#define TR_DATUM 2    // Top right
#define ML_DATUM 3    // Middle left
#define CL_DATUM 3    // Centre left, same as above
#define MC_DATUM 4    // Middle centre
#define CC_DATUM 4    // Centre centre, same as above
#define MR_DATUM 5    // Middle right
#define CR_DATUM 5    // Centre right, same as above
#define BL_DATUM 6    // Bottom left
#define BC_DATUM 7    // Bottom centre
#define BR_DATUM 8    // Bottom right
#define L_BASELINE 9  // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline
void GUI_Set_StringAlign(uint8_t datum);
void GUI_DrawCircle(uint8_t x, uint8_t y, uint8_t r, uint8_t invert);
void GUI_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t invert);
void GUI_DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t invert);
void GUI_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize, uint8_t invert);
void GUI_DrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint8_t invert);
void GUI_FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t invert);
void GUI_FillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint8_t color);
void GUI_DrawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent, uint8_t invert);
void GUI_DrawBitmap(int16_t x, int16_t y, const uint8_t *bmp, uint8_t datum, uint8_t invert);
void GUI_DrawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
void GUI_FillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
void GUI_FillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
void GUI_FrameFlush(void);
extern int oled_close       (struct display_info* disp);
extern int oled_open        (struct display_info* disp, char* filename);
extern int oled_send        (struct display_info* disp, struct sized_array* payload);
extern int oled_init        (struct display_info* disp);
extern int oled_send_buffer (struct display_info* disp);
extern void oled_clear(struct display_info *disp);
extern void oled_putstr(struct display_info *disp, uint8_t line, uint8_t *str);
extern void oled_putpixel(struct display_info *disp, uint8_t x, uint8_t y, uint8_t on);
extern void oled_putstrto(struct display_info *disp, uint8_t x, uint8_t y, char *str);

#endif // OLED_H
