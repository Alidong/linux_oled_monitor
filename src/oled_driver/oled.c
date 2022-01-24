/*
 * Copyright (c) 2015, Vladimir Komendantskiy
 * MIT License
 *
 * OLED is a 128x64 dot matrix display driver and controller by Solomon
 * Systech. It is used by HelTec display modules.
 *
 * Reference:
 *
 * [1] OLED Advance Information. 128x64 Dot Matrix Segment/Common
 *     Driver with Controller. (Solomon Systech)
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

// real-time features
#include <sys/mman.h>
#include <sched.h>

#include "oled.h"
//#include "font.h"
//#include <wiringPiI2C.h>

static int i2c_write(int fd,const uint8_t* buf,uint16_t size)
{
   int retries;
   //设置地址长度：0为7位地址
   ioctl(fd,I2C_TENBIT,0);
   //设置从机地址
   if (ioctl(fd,I2C_SLAVE,OLED_I2C_ADDR) < 0)
   {
      printf("fail to set i2c device slave address!\n");
      close(fd);
      return -1;
   }
   //设置收不到ACK时的重试次数
   ioctl(fd,I2C_RETRIES,5);

   if (write(fd, buf, size) == size)
   {
      return 0;
   }
   else
   {
		printf("fail to write i2c slave device  !\n");
      return -1;
   }

}

static int i2c_read(int fd, uint8_t addr,uint8_t reg,uint8_t * val)
{
   int retries;
   //设置地址长度：0为7位地址
   ioctl(fd,I2C_TENBIT,0);
   //设置从机地址
   if (ioctl(fd,I2C_SLAVE,addr) < 0)
   {
      printf("fail to set i2c device slave address!\n");
      close(fd);
      return -1;
   }
   //设置收不到ACK时的重试次数
   ioctl(fd,I2C_RETRIES,5);

   if (write(fd, &reg, 1) == 1)
   {
      if (read(fd, val, 1) == 1)
      {
            return 0;
      }
   }
   else
   {
      return -1;
   }
}
static uint8_t textdatum = TL_DATUM;
static struct display_info *nativdisp;
int oled_close(struct display_info *disp) {
	if (close(disp->file) < 0)
		return -1;

	return 0;
}

void cleanup(int status, void *disp) {
	oled_close((struct display_info *)disp);
}

int oled_open(struct display_info *disp, char *filename) {

	//disp->file = wiringPiI2CSetupInterface (filename, disp->address);
	disp->file=open(filename,O_RDWR );
	if (disp->file < 0)
		return -1;
	return 0;
}

// write commands and data to /dev/i2c*
int oled_send(struct display_info *disp, struct sized_array *payload) {
	//if (write(disp->file, payload->array, payload->size) != payload->size)
	if (i2c_write(disp->file, payload->array, payload->size) !=0)
		return -1;
	return 0;
}
static int oled_singal_cmd(struct display_info *disp,uint8_t cmd){
	uint8_t cmd_b[2]={0,0};
	cmd_b[1]=cmd;
	if (write(disp->file, cmd_b, 2) !=2)
		return -1;
	return 0;
}
int oled_init(struct display_info *disp) {
	struct sched_param sch;
	int status = 0;
	struct sized_array payload;

	//	sch.sched_priority = 49;
	//
	//	status = sched_setscheduler(0, SCHED_FIFO, &sch);
	//	if (status < 0)
	//		return status;
	//
	//	status = mlockall(MCL_CURRENT | MCL_FUTURE);
	//	if (status < 0)
	//		return status;

	payload.size = sizeof(display_config);
	payload.array = display_config;

	status = oled_send(disp, &payload);
	if (status < 0)
		return 666;
	nativdisp=disp;
	memset(disp->buffer, 0, sizeof(disp->buffer));
	oled_clear(disp);
	return 0;
}

// send buffer to oled (show)
int oled_send_buffer(struct display_info *disp) {
	struct sized_array payload;
	uint8_t packet[129];
	int index;

	for (index = 0; index < (Ver_size/8); index++) {	
		oled_singal_cmd(disp,0xb0 + index); //page0-page1
     	oled_singal_cmd(disp,0x00);     //low column start address
        oled_singal_cmd(disp,0x10);     //high column start address
		packet[0] = OLED_CTRL_BYTE_DATA_STREAM;
		memcpy(packet + 1, disp->buffer[index], 128);
		payload.size = 129;
		payload.array = packet;
		oled_send(disp, &payload);
	}
	//memset(disp->buffer,0,sizeof(disp->buffer));
	return 0;
}

// clear screen
void oled_clear(struct display_info *disp) {
	memset(disp->buffer, 0, sizeof(disp->buffer));
	oled_send_buffer(disp);
}

// // put string to one of the 8 pages (128x8 px) 
// void oled_putstr(struct display_info *disp, uint8_t line, uint8_t *str) {
// 	uint8_t a;
// 	int slen = strlen(str);
// 	uint8_t fwidth = disp->font.width;
// 	uint8_t foffset = disp->font.offset;
// 	uint8_t fspacing = disp->font.spacing;
// 	int i=0;

// 	for (i=0; i<slen; i++) {
// 		a=(uint8_t)str[i];
// 		if (i >= 128/fwidth)
// 			break; // no text wrap
// 		memcpy(disp->buffer[line] + i*fwidth + fspacing, &disp->font.data[(a-foffset) * fwidth], fwidth);
// 	}
// }

// put one pixel at xy, on=1|0 (turn on|off pixel)
void oled_putpixel(struct display_info *disp, uint8_t x, uint8_t y, uint8_t on) {
	uint8_t pageId = y / 8;
	uint8_t bitOffset = y % 8;
	if (x < 128-1) {
		if (on != 0)
			disp->buffer[pageId][x] |= (1<<bitOffset);
		else
			disp->buffer[pageId][x] &= ~(1<<bitOffset);
	}
}

// // put string to the buffer at xy
// void oled_putstrto(struct display_info *disp, uint8_t x, uint8_t y, char *str) {
// 	uint8_t a;
// 	int slen = strlen(str);
// 	uint8_t fwidth = disp->font.width;
// 	uint8_t fheight = disp->font.height;
// 	uint8_t foffset = disp->font.offset;
// 	uint8_t fspacing = disp->font.spacing;
// 	int i=0;
// 	int j=0;
// 	int k=0;

// 	for (k=0; k<slen; k++) {
// 		a=(uint8_t)str[k];
// 		for (i=0; i<fwidth; i++) {
// 			for (j=0; j<fheight; j++) {
// 				if (((disp->font.data[(a-foffset)*fwidth + i] >> j) & 0x01))
// 					oled_putpixel(disp, x+i, y+j, 1);
// 				else
// 					oled_putpixel(disp, x+i, y+j, 0);
// 			}
// 		}
// 		x+=fwidth+fspacing;
// 	}
// }

// static int swap_coord(int x, int y)
// {
//     int t = x;
//     x = y;
//     y = t;
// }
// void GUI_FrameFlush(void){
// 	oled_send_buffer(nativdisp);
// }

// //画点
// //x:0~127
// //y:0~63
// //t:1 填充 0,清空
// void GUI_DrawPoint(int16_t x, int16_t y, uint8_t t)
// {

//     //对应 byte __Gram[512];	  // x= 64 ,y=8
//     //变换一哈形式
//     uint8_t pos, bx, temp = 0;
//     if (x > 127 || y > 63)
//         return; //超出范围了.
//     if (x < 0 || y < 0)
//         return;

//     pos = y / 8;
//     bx = y % 8;
//     temp = 1 << (bx);
//     if (t)
//       nativdisp->buffer[pos][x] |=temp; // 做到的效果是竖着存放 竖着为y ，横着为x
//     else
//         nativdisp->buffer[pos][x] &= ~temp;
// }
// void GUI_Set_StringAlign(uint8_t datum)
// {
//     textdatum = datum;
// }
// void GUI_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize, uint8_t invert)
// {
//     unsigned char c = 0, i = 0, j = 0, x_pre = x;
//     int k = 0;
//     uint8_t data_8;
//     uint16_t data_16;
//     int16_t sumX = 0;
//     uint8_t padding = 1, baseline = 0;
//     uint16_t cwidth; // Find the pixel width of the string in the font
//     uint16_t cheight = 8 * TextSize;
//     if (TextSize == 1)
//     {
//         cwidth = strlen(ch) * 6;
//     }
//     else
//     {
//         cwidth = strlen(ch) * 8;
//     }

//     switch (textdatum)
//     {
//     case TC_DATUM:
//         x -= cwidth / 2;
//         padding += 1;
//         break;
//     case TR_DATUM:
//         x -= cwidth;
//         padding += 2;
//         break;
//     case ML_DATUM:
//         y -= cheight / 2;
//         //padding += 0;
//         break;
//     case MC_DATUM:
//         x -= cwidth / 2;
//         y -= cheight / 2;
//         padding += 1;
//         break;
//     case MR_DATUM:
//         x -= cwidth;
//         y -= cheight / 2;
//         padding += 2;
//         break;
//     case BL_DATUM:
//         y -= cheight;
//         //padding += 0;
//         break;
//     case BC_DATUM:
//         x -= cwidth / 2;
//         y -= cheight;
//         padding += 1;
//         break;
//     case BR_DATUM:
//         x -= cwidth;
//         y -= cheight;
//         padding += 2;
//         break;
//     case L_BASELINE:
//         y -= baseline;
//         //padding += 0;
//         break;
//     case C_BASELINE:
//         x -= cwidth / 2;
//         y -= baseline;
//         padding += 1;
//         break;
//     case R_BASELINE:
//         x -= cwidth;
//         y -= baseline;
//         padding += 2;
//         break;
//     }
//     switch (TextSize)
//     {
//     case 1:
//     {
//         while (ch[j] != '\0')
//         {
//             if (ch[j] == '\n')
//             {
//                 y += 8;
//                 x = x_pre;
//                 j++;
//             }

//             c = ch[j] - 32;
//             for (i = 0; i < 6; i++)
//             {
//                 data_8 = F6x8[c][i];
//                 for (k = 0; k < 8; k++)
//                 {

//                     if ((data_8 >> k) & 0x01)
//                     {
//                         GUI_DrawPoint(x, y, invert);
//                     }
//                     else
//                     {
//                         GUI_DrawPoint(x, y, !invert);
//                     }
//                     y++;
//                 }
//                 y -= 8;
//                 x++;
//             }

//             //x += 6;
//             j++;
//         }
//     }
//     break;
//     case 2:
//     {
//         while (ch[j] != '\0')
//         {
//             if (ch[j] == '\n')
//             {
//                 y += 16;
//                 x = x_pre;
//                 j++;
//             }
//             c = ch[j] - 32;
//             for (i = 0; i < 8; i++)
//             {
//                 data_16 = (F8X16[c * 16 + i]) | (F8X16[c * 16 + i + 8] << 8);
//                 for (k = 0; k < 16; k++)
//                 {

//                     if ((data_16 >> k) & 0x01)
//                     {
//                         GUI_DrawPoint(x, y, invert);
//                     }
//                     else
//                     {
//                         GUI_DrawPoint(x, y, !invert);
//                     }
//                     y++;
//                 }
//                 y -= 16;
//                 x++;
//             }
//             j++;
//         }
//     }
//     break;
//     }
// }
// //画线
// //x1,y1:起点坐标
// //x2,y2:结束坐标
// void GUI_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t invert)
// {
//     uint16_t t;
//     int xerr = 0, yerr = 0, delta_x, delta_y, distance;
//     int incx, incy, uRow, uCol;
//     delta_x = x2 - x1; //计算坐标增量
//     delta_y = y2 - y1;
//     uRow = x1;
//     uCol = y1;
//     if (delta_x > 0)
//         incx = 1; //设置单步方向
//     else if (delta_x == 0)
//         incx = 0; //垂直线
//     else
//     {
//         incx = -1;
//         delta_x = -delta_x;
//     }
//     if (delta_y > 0)
//         incy = 1;
//     else if (delta_y == 0)
//         incy = 0; //水平线
//     else
//     {
//         incy = -1;
//         delta_y = -delta_y;
//     }
//     if (delta_x > delta_y)
//         distance = delta_x; //选取基本增量坐标轴
//     else
//         distance = delta_y;
//     for (t = 0; t <= distance + 1; t++) //画线输出
//     {
//         GUI_DrawPoint(uRow, uCol, invert); //画点
//         xerr += delta_x;
//         yerr += delta_y;
//         if (xerr > distance)
//         {
//             xerr -= distance;
//             uRow += incx;
//         }
//         if (yerr > distance)
//         {
//             yerr -= distance;
//             uCol += incy;
//         }
//     }
// }

// // Draw a triangle
// void GUI_DrawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
// {

//     GUI_DrawLine(x0, y0, x1, y1, color);
//     GUI_DrawLine(x1, y1, x2, y2, color);
//     GUI_DrawLine(x2, y2, x0, y0, color);
// }
// // Fill a triangle - original Adafruit function works well and code footprint is small
// void GUI_FillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
// {
//     int32_t a, b, y, last;

//     // Sort coordinates by Y order (y2 >= y1 >= y0)
//     if (y0 > y1)
//     {
//         swap_coord(y0, y1);
//         swap_coord(x0, x1);
//     }
//     if (y1 > y2)
//     {
//         swap_coord(y2, y1);
//         swap_coord(x2, x1);
//     }
//     if (y0 > y1)
//     {
//         swap_coord(y0, y1);
//         swap_coord(x0, x1);
//     }

//     if (y0 == y2)
//     { // Handle awkward all-on-same-line case as its own thing
//         a = b = x0;
//         if (x1 < a)
//             a = x1;
//         else if (x1 > b)
//             b = x1;
//         if (x2 < a)
//             a = x2;
//         else if (x2 > b)
//             b = x2;
//         GUI_DrawLine(a, y0, a + b - a + 1, y0, color);
//         // drawFastHLine(a, y0, b - a + 1, color);
//         return;
//     }

//     int32_t
//         dx01 = x1 - x0,
//         dy01 = y1 - y0,
//         dx02 = x2 - x0,
//         dy02 = y2 - y0,
//         dx12 = x2 - x1,
//         dy12 = y2 - y1,
//         sa = 0,
//         sb = 0;

//     // For upper part of triangle, find scanline crossings for segments
//     // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
//     // is included here (and second loop will be skipped, avoiding a /0
//     // error there), otherwise scanline y1 is skipped here and handled
//     // in the second loop...which also avoids a /0 error here if y0=y1
//     // (flat-topped triangle).
//     if (y1 == y2)
//         last = y1; // Include y1 scanline
//     else
//         last = y1 - 1; // Skip it

//     for (y = y0; y <= last; y++)
//     {
//         a = x0 + sa / dy01;
//         b = x0 + sb / dy02;
//         sa += dx01;
//         sb += dx02;

//         if (a > b)
//             swap_coord(a, b);
//         //drawFastHLine(a, y, b - a + 1, color);
//         GUI_DrawLine(a, y, a + b - a + 1, y, color);
//     }

//     // For lower part of triangle, find scanline crossings for segments
//     // 0-2 and 1-2.  This loop is skipped if y1=y2.
//     sa = dx12 * (y - y1);
//     sb = dx02 * (y - y0);
//     for (; y <= y2; y++)
//     {
//         a = x1 + sa / dy12;
//         b = x0 + sb / dy02;
//         sa += dx12;
//         sb += dx02;

//         if (a > b)
//             swap_coord(a, b);
//         //drawFastHLine(a, y, b - a + 1, color);
//         GUI_DrawLine(a, y, a + b - a + 1, y, color);
//     }
// }

// // Draw a rectangle
// void GUI_DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t invert)
// {

//     GUI_DrawLine(x, y, x + w - 1, y, invert);
//     GUI_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, invert);
//     // Avoid drawing corner pixels twice
//     GUI_DrawLine(x, y + 1, x, y + h - 2, invert);
//     GUI_DrawLine(x + w - 1, y + 1, x + w - 1, y + h - 2, invert);
// }

// //x,y:圆心坐标
// //r:圆的半径
// void GUI_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t invert)
// {
//     int32_t x = 0;
//     int32_t dx = 1;
//     int32_t dy = r + r;
//     int32_t p = -(r >> 1);

//     // These are ordered to minimise coordinate changes in x or y
//     // GUI_DrawPoint can then send fewer bounding box commands
//     GUI_DrawPoint(x0 + r, y0, invert);
//     GUI_DrawPoint(x0 - r, y0, invert);
//     GUI_DrawPoint(x0, y0 - r, invert);
//     GUI_DrawPoint(x0, y0 + r, invert);

//     while (x < r)
//     {

//         if (p >= 0)
//         {
//             dy -= 2;
//             p -= dy;
//             r--;
//         }

//         dx += 2;
//         p += dx;

//         x++;

//         // These are ordered to minimise coordinate changes in x or y
//         // GUI_DrawPoint can then send fewer bounding box commands
//         GUI_DrawPoint(x0 + x, y0 + r, invert);
//         GUI_DrawPoint(x0 - x, y0 + r, invert);
//         GUI_DrawPoint(x0 - x, y0 - r, invert);
//         GUI_DrawPoint(x0 + x, y0 - r, invert);

//         GUI_DrawPoint(x0 + r, y0 + x, invert);
//         GUI_DrawPoint(x0 - r, y0 + x, invert);
//         GUI_DrawPoint(x0 - r, y0 - x, invert);
//         // GUI_DrawPoint(x0 + r, y0 - x, invert);
//     }
// }
// /***************************************************************************************
// ** Function name:           drawCircleHelper
// ** Description:             Support function for circle drawing
// ***************************************************************************************/
// void GUI_CircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint8_t invert)
// {
//     int32_t f = 1 - r;
//     int32_t ddF_x = 1;
//     int32_t ddF_y = -2 * r;
//     int32_t x = 0;

//     while (x < r)
//     {
//         if (f >= 0)
//         {
//             r--;
//             ddF_y += 2;
//             f += ddF_y;
//         }
//         x++;
//         ddF_x += 2;
//         f += ddF_x;
//         if (cornername & 0x4)
//         {
//             GUI_DrawPoint(x0 + x, y0 + r, invert);
//             GUI_DrawPoint(x0 + r, y0 + x, invert);
//         }
//         if (cornername & 0x2)
//         {
//             GUI_DrawPoint(x0 + x, y0 - r, invert);
//             GUI_DrawPoint(x0 + r, y0 - x, invert);
//         }
//         if (cornername & 0x8)
//         {
//             GUI_DrawPoint(x0 - r, y0 + x, invert);
//             GUI_DrawPoint(x0 - x, y0 + r, invert);
//         }
//         if (cornername & 0x1)
//         {
//             GUI_DrawPoint(x0 - r, y0 - x, invert);
//             GUI_DrawPoint(x0 - x, y0 - r, invert);
//         }
//     }
// }
// /***************************************************************************************
// ** Function name:           fillCircle
// ** Description:             draw a filled circle
// ***************************************************************************************/
// // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
// void GUI_FillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
// {
//     int32_t x = 0;
//     int32_t dx = 1;
//     int32_t dy = r + r;
//     int32_t p = -(r >> 1);
//     GUI_DrawLine(x0 - r, y0, x0 - r + dy + 1, y0, color);
//     while (x < r)
//     {

//         if (p >= 0)
//         {
//             dy -= 2;
//             p -= dy;
//             r--;
//         }

//         dx += 2;
//         p += dx;

//         x++;

//         GUI_DrawLine(x0 - r, y0 + x, x0 - r + 2 * r + 1, y0 + x, color);
//         GUI_DrawLine(x0 - r, y0 - x, x0 - r + 2 * r + 1, y0 - x, color);
//         GUI_DrawLine(x0 - x, y0 + r, x0 - x + 2 * x + 1, y0 + r, color);
//         GUI_DrawLine(x0 - x, y0 - r, x0 - x + 2 * x + 1, y0 - r, color);
//     }
// }
// /***************************************************************************************
// ** Function name:           fillCircleHelper
// ** Description:             Support function for filled circle drawing
// ***************************************************************************************/
// // Used to support drawing roundrects, changed to horizontal lines (faster in sprites)
// void GUI_FillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint8_t invert)
// {
//     int32_t f = 1 - r;
//     int32_t ddF_x = 1;
//     int32_t ddF_y = -r - r;
//     int32_t y = 0;

//     delta++;
//     while (y < r)
//     {
//         if (f >= 0)
//         {
//             r--;
//             ddF_y += 2;
//             f += ddF_y;
//         }
//         y++;
//         //x++;
//         ddF_x += 2;
//         f += ddF_x;

//         if (cornername & 0x1)
//         {
//             GUI_DrawLine(x0 - r, y0 + y, x0 - r + r + r + delta, y0 + y, invert);
//             GUI_DrawLine(x0 - y, y0 + r, x0 - y + y + y + delta, y0 + r, invert);
//         }
//         if (cornername & 0x2)
//         {
//             GUI_DrawLine(x0 - r, y0 - y, x0 - r + r + r + delta, y0 - y, invert); // 11995, 1090
//             GUI_DrawLine(x0 - y, y0 - r, x0 - y + y + y + delta, y0 - r, invert);
//         }
//     }
// }
// /***************************************************************************************
// ** Function name:           drawRoundRect
// ** Description:             Draw a rounded corner rectangle outline
// ***************************************************************************************/
// // Draw a rounded rectangle
// void GUI_DrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint8_t invert)
// {

//     GUI_DrawLine(x + r, y, x + w - r, y, invert);                 // Top
//     GUI_DrawLine(x + r, y + h - 1, x + w - r, y + h - 1, invert); // Bottom
//     GUI_DrawLine(x, y + r, x, y + h - r, invert);                 // Left
//     GUI_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r, invert); // Right
//     // draw four corners
//     GUI_CircleHelper(x + r, y + r, r, 1, invert);
//     GUI_CircleHelper(x + w - r - 1, y + r, r, 2, invert);
//     GUI_CircleHelper(x + w - r - 1, y + h - r - 1, r, 4, invert);
//     GUI_CircleHelper(x + r, y + h - r - 1, r, 8, invert);
// }
// void GUI_FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t invert)
// {
//     uint8_t i, k;
//     for (k = 0; k < h; k++)
//     {
//         for (i = 0; i < w; i++)
//         {
//             GUI_DrawPoint(i + x, k + y, invert);
//         }
//     }
// }
// /***************************************************************************************
// ** Function name:           fillRoundRect
// ** Description:             Draw a rounded corner filled rectangle
// ***************************************************************************************/
// // Fill a rounded rectangle, changed to horizontal lines (faster in sprites)
// void GUI_FillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint8_t color)
// {

//     GUI_FillRect(x, y + r, w, h - r - r, color);
//     // draw four corners
//     GUI_FillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 2, color);
//     GUI_FillCircleHelper(x + r, y + r, r, 2, w - r - r - 2, color);
// }
// /***************************************************************************************
// ** Function name:           drawProgressBar
// ** Description:             Draw a progress bar - increasing percentage only
// ***************************************************************************************/
// void GUI_DrawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent, uint8_t invert)
// {

//     int8_t margin = 2;
//     int16_t barHeight = h - 2 * margin;
//     int16_t barWidth = w - 2 * margin;
//     if (percent > 100)
//     {
//         percent = 100;
//     }
//     if (percent < 0)
//     {
//         percent = 0;
//     }

//     GUI_DrawRoundRect(x, y, w, h, 3, invert);
//     GUI_FillRect(x + margin, y + margin, barWidth * percent / 100.0, barHeight, invert);
// }
// void GUI_DrawBitmap(int16_t x, int16_t y, const uint8_t *bmp, uint8_t datum, uint8_t invert)
// {
//     int32_t i, j, n, byteWidth;
//     int16_t w, h;
//     uint8_t temp;
//     w = (*(bmp + 2) << 8) | (*(bmp + 3));
//     h = (*(bmp + 4) << 8) | (*(bmp + 5));
//     byteWidth = (h + 7) / 8;
//     switch (datum)
//     {
//     case TC_DATUM:
//         x -= w / 2;
//         break;
//     case TR_DATUM:
//         x -= w;

//         break;
//     case ML_DATUM:
//         y -= h / 2;
//         //padding += 0;
//         break;
//     case MC_DATUM:
//         x -= w / 2;
//         y -= h / 2;
//         break;
//     case MR_DATUM:
//         x -= w;
//         y -= h / 2;
//         break;
//     case BL_DATUM:
//         y -= h;
//         //padding += 0;
//         break;
//     case BC_DATUM:
//         x -= w / 2;
//         y -= h;
//         break;
//     case BR_DATUM:
//         x -= w;
//         y -= h;
//         break;
//     }
//     for (j = 0; j < byteWidth; j++)
//     {
//         for (i = 0; i < w; i++)
//         {
//             for (n = 7; n >= 0; n--)
//             {
//                 if ((bmp[i + 6 + j * w] >> n) & 0x01)
//                 {
//                     GUI_DrawPoint(x + i, y + j * 8 + 7 - n, 1);
//                 }
//             }
//         }
//     }
// }
