

#ifndef __ST7735_H__
#define __ST7735_H__


#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04




// mini 160x80 display (it's unlikely you want the default orientation)
#define ST7735_IS_160X80 1
#define ST7735_XSTART 24
#define ST7735_YSTART 0
#define ST7735_WIDTH  80
#define ST7735_HEIGHT 160 

//the only working config for my display, Y controls the horizontal axis,
// X controls the vertical one, i fixed this on the draw method
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_BGR)

// mini 160x80, rotate left
/*
#define ST7735_IS_160X80 1
#define ST7735_XSTART 1
#define ST7735_YSTART 26
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 80
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
*/

// mini 160x80, rotate right 
/*
#define ST7735_IS_160X80 1
#define ST7735_XSTART 1
#define ST7735_YSTART 26
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 80
#define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
*/


/****************************/

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_GAMSET  0x26
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1



typedef enum {
	GAMMA_10 = 0x01,
	GAMMA_25 = 0x02,
	GAMMA_22 = 0x04,
	GAMMA_18 = 0x08
} GammaDef;



//structure to hold a x ,y ,z point in space
typedef struct {
    short x;
    short y;
    short z;
} Vector3D;

void ST7735_Init(void);

//----------------------------------------
//draw helpers
void ST7735_SetAddressWindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);

void ST7735_DrawPixel(unsigned char x, unsigned char y, unsigned short color);

void ST7735_DrawLine(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned short color);

void ST7735_DrawRect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned short color);

void ST7735_DrawFilledRect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned short color);

//--------------
// 3D
Vector3D ST7735_ProjectPoint(int x, int y, int z);

void ST7735_Draw3DPoint(int x, int y, int z, unsigned short color);

void ST7735_DrawBoxWireFrame(int startX, int startY, int startZ, int w, int h, int d, float angleX, float angleY, float angleZ, unsigned short color);

void ST7735_DrawPyramidWireFrame(int startX, int startY, int startZ, int w, int h, int d, float angleX, float angleY, float angleZ, unsigned short color);

void ST7735_DrawOctahedronWireFrame(int startX, int startY, int startZ, int w, int h, int d, float angleX, float angleY, float angleZ, unsigned short color);

void ST7735_DrawDodecahedronWireFrame(int startX, int startY, int startZ, int radius, float angleX, float angleY, float angleZ, unsigned short color);

#endif