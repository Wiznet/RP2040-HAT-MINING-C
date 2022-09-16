/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "ILI9340.h"
#include "port_common.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define LCD_GPIO_H	1
#define LCD_GPIO_L	0

#define X_MIN  325
#define X_MAX  750
#define Y_MIN  840
#define Y_MAX  240

#define SPI_LCD_PORT spi1

#define PIN_LCD_SCK  	10
#define PIN_LCD_MOSI 	11
#define PIN_LCD_MISO 	12
#define PIN_LCD_CS   	13
#define PIN_LCD_DC    	14 
#define PIN_LCD_RESET   15 
#define PIN_LCD_LED		 5 

#define ILI9340_SPI_SPEED	5000 * 1000

#define FONT_TEXT_OFFSET 10

#define FONT_HIGHT_OFFSET_LINE_1     20
#define FONT_HIGHT_OFFSET_ONE_BLOCK  50

//1st line
#define FONT_WIDTH_X_SLAVE_TEXT          LCD_HIGHT_BASE_X+ 0
#define FONT_WIDTH_X_SLAVE_INDEX      	 LCD_HIGHT_BASE_X+32
#define FONT_WIDTH_X_RESULT       		 LCD_HIGHT_BASE_X+60
#define FONT_WIDTH_X_DIFF_TEXT       	 LCD_HIGHT_BASE_X+110
#define FONT_WIDTH_X_DIFF_NUM       	 LCD_HIGHT_BASE_X+155
//2nd line
#define FONT_WIDTH_X_HASH_NUM       	 LCD_HIGHT_BASE_X+15
#define FONT_WIDTH_X_HASH_HS_TEXT        LCD_HIGHT_BASE_X+75 
#define FONT_WIDTH_X_ELAPSED_TIME_TEXT1  LCD_HIGHT_BASE_X+130
#define FONT_WIDTH_X_ELAPSED_TIME_NUM	 LCD_HIGHT_BASE_X+140
#define FONT_WIDTH_X_ELAPSED_TIME_TEXT2  LCD_HIGHT_BASE_X+200



/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

static uint16_t ILI9340_WIDTH  = ILI9340_TFTWIDTH;
static uint16_t ILI9340_HEIGHT = ILI9340_TFTHEIGHT;

static uint8_t  TEXT_SIZE = 1;
static uint16_t PEN_THICKNESS = 1;
static uint16_t TEXT_OPTIONS = TEXT_DEFAULT;
static uint16_t BACKGROUND_COLOR = BLACK;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

void ILI9340_PauseMs(uint32_t ms)
{
	sleep_ms(ms);
}

void ILI9340_SetBackgroundColor(uint16_t bg)
{
	BACKGROUND_COLOR = bg;
}

void ILI9340_SetTextOptions(uint16_t opt)
{
	TEXT_OPTIONS = opt;
}

void ILI9340_SetTextSize(uint8_t ts)
{
	if (ts < 1)
		ts = 1;

	TEXT_SIZE = ts;
}

uint8_t ILI9340_GetTextSize()
{
	return TEXT_SIZE;
}

void ILI9340_SetPenThickness(uint16_t t)
{
	PEN_THICKNESS = t;
}

uint16_t ILI9340_GetPenThickness()
{
	return PEN_THICKNESS;
}

void ILI9340_TxRx_8bit(uint8_t data)
{
 	spi_write_blocking(SPI_LCD_PORT, &data, 1);
}

void ILI9340_Tx_16bit(uint16_t data)
{
	uint8_t hi = data >> 8, lo = data;

	ILI9340_TxRx_8bit(hi);
	ILI9340_TxRx_8bit(lo);
}

//Write a single command or data byte to the LCD
void ILI9340_WriteCommand(uint8_t command)
{
	ILI9340_DC_Ctrl(LCD_GPIO_L);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	ILI9340_TxRx_8bit(command);

	ILI9340_CS_Ctrl(LCD_GPIO_H);;
}

void ILI9340_WriteData(uint8_t data)
{
	ILI9340_DC_Ctrl(LCD_GPIO_H);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	ILI9340_TxRx_8bit(data);

	ILI9340_CS_Ctrl(LCD_GPIO_H);
}

void ILI9340_WriteDataBytes(uint16_t *data, uint16_t size)
{
	uint16_t i = 0;

	ILI9340_DC_Ctrl(LCD_GPIO_H);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	for (i=0;i<size;i++)
	{
		ILI9340_Tx_16bit(data[i]);
	}

	ILI9340_CS_Ctrl(LCD_GPIO_H);
}

void ILI9340_Reset()
{
    ILI9340_Reset_Ctrl(LCD_GPIO_H);
    ILI9340_PauseMs(5);
    ILI9340_Reset_Ctrl(LCD_GPIO_L);
    ILI9340_PauseMs(20);
    ILI9340_Reset_Ctrl(LCD_GPIO_H);
    ILI9340_PauseMs(150);
    
    ILI9340_Initialize();
    ILI9340_PauseMs(25);
}   

void ILI9340_Initialize()
{
	ILI9340_WriteCommand(0xEF);
	ILI9340_WriteData(0x03);
	ILI9340_WriteData(0x80);
	ILI9340_WriteData(0x02);
	ILI9340_WriteCommand(0xCF);
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0xC1);
	ILI9340_WriteData(0x30);
	ILI9340_WriteCommand(0xED);
	ILI9340_WriteData(0x64);
	ILI9340_WriteData(0x03);
	ILI9340_WriteData(0x12);
	ILI9340_WriteData(0x81);
	ILI9340_WriteCommand(0xE8);
	ILI9340_WriteData(0x85);
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0x78);
	ILI9340_WriteCommand(0xCB);
	ILI9340_WriteData(0x39);
	ILI9340_WriteData(0x2C);
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0x34);
	ILI9340_WriteData(0x02);
	ILI9340_WriteCommand(0xF7);
	ILI9340_WriteData(0x20);
	ILI9340_WriteCommand(0xEA);
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0x00);

	ILI9340_WriteCommand(ILI9340_PWCTR1); //Power control
	ILI9340_WriteData(0x23); //VRH[5:0]
	ILI9340_WriteCommand(ILI9340_PWCTR2); //Power control
	ILI9340_WriteData(0x10); //SAP[2:0];BT[3:0]
	ILI9340_WriteCommand(ILI9340_VMCTR1); //VCM control
	ILI9340_WriteData(0x3e); //�Աȶȵ���
	ILI9340_WriteData(0x28);
	ILI9340_WriteCommand(ILI9340_VMCTR2); //VCM control2
	ILI9340_WriteData(0x86); //--
	ILI9340_WriteCommand(ILI9340_MADCTL); // Memory Access Control
	ILI9340_WriteData(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
	ILI9340_WriteCommand(ILI9340_PIXFMT);
	ILI9340_WriteData(0x55);
	ILI9340_WriteCommand(ILI9340_FRMCTR1);
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0x18);
	ILI9340_WriteCommand(ILI9340_DFUNCTR); // Display Function Control
	ILI9340_WriteData(0x08);
	ILI9340_WriteData(0x82);
	ILI9340_WriteData(0x27);
	ILI9340_WriteCommand(0xF2); // 3Gamma Function Disable
	ILI9340_WriteData(0x00);
	ILI9340_WriteCommand(ILI9340_GAMMASET); //Gamma curve selected
	ILI9340_WriteData(0x01);
	ILI9340_WriteCommand(ILI9340_GMCTRP1); //Set Gamma
	ILI9340_WriteData(0x0F);
	ILI9340_WriteData(0x31);
	ILI9340_WriteData(0x2B);
	ILI9340_WriteData(0x0C);
	ILI9340_WriteData(0x0E);
	ILI9340_WriteData(0x08);
	ILI9340_WriteData(0x4E);
	ILI9340_WriteData(0xF1);
	ILI9340_WriteData(0x37);
	ILI9340_WriteData(0x07);
	ILI9340_WriteData(0x10);
	ILI9340_WriteData(0x03);
	ILI9340_WriteData(0x0E);
	ILI9340_WriteData(0x09);
	ILI9340_WriteData(0x00);
	ILI9340_WriteCommand(ILI9340_GMCTRN1); //Set Gamma
	ILI9340_WriteData(0x00);
	ILI9340_WriteData(0x0E);
	ILI9340_WriteData(0x14);
	ILI9340_WriteData(0x03);
	ILI9340_WriteData(0x11);
	ILI9340_WriteData(0x07);
	ILI9340_WriteData(0x31);
	ILI9340_WriteData(0xC1);
	ILI9340_WriteData(0x48);
	ILI9340_WriteData(0x08);
	ILI9340_WriteData(0x0F);
	ILI9340_WriteData(0x0C);
	ILI9340_WriteData(0x31);
	ILI9340_WriteData(0x36);
	ILI9340_WriteData(0x0F);
	ILI9340_WriteCommand(ILI9340_SLPOUT); //Exit Sleep
 	ILI9340_PauseMs(120); 
	ILI9340_WriteCommand(ILI9340_DISPON); //Display onf
}

void ILI9340_Rotate(uint8_t dir)
{
	dir = dir % 4;

	ILI9340_WriteCommand(ILI9340_MADCTL);

	switch (dir)
	{
	case ILI9340_ROT_LEFT:
		ILI9340_WriteData(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
		ILI9340_WIDTH = ILI9340_TFTWIDTH;
		ILI9340_HEIGHT= ILI9340_TFTHEIGHT;
		break;
	case ILI9340_ROT_RIGHT:
		ILI9340_WriteData(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
		ILI9340_WIDTH = ILI9340_TFTHEIGHT;
		ILI9340_HEIGHT = ILI9340_TFTWIDTH;
		break;
	case ILI9340_ROT_UP:
		ILI9340_WriteData(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
		ILI9340_WIDTH = ILI9340_TFTWIDTH;
		ILI9340_HEIGHT = ILI9340_TFTHEIGHT;
		break;
	case ILI9340_ROT_DOWN:
		ILI9340_WriteData(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
		ILI9340_WIDTH = ILI9340_TFTHEIGHT;
		ILI9340_HEIGHT = ILI9340_TFTWIDTH;
		break;
	}
}

void ILI9340_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t data[2];

	ILI9340_WriteCommand(ILI9340_CASET);

	data[0] = x1;
	data[1] = x2;

	ILI9340_WriteDataBytes(data, 2);

	ILI9340_WriteCommand(ILI9340_PASET);

	data[0] = y1;
	data[1] = y2;

	ILI9340_WriteDataBytes(data, 2);

	ILI9340_WriteCommand(ILI9340_RAMWR);

}

void ILI9340_VerticalLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
	if (h == 0) return;
	if (x > ILI9340_WIDTH || y > ILI9340_HEIGHT) return;

	ILI9340_FillRect(x-(PEN_THICKNESS/2),y-(PEN_THICKNESS/2),PEN_THICKNESS,h+PEN_THICKNESS,color);
}

void ILI9340_HorizontalLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
	if (w == 0) return;
	if (x > ILI9340_WIDTH || y > ILI9340_HEIGHT) return;

	ILI9340_FillRect(x-(PEN_THICKNESS/2), y-(PEN_THICKNESS/2), w+PEN_THICKNESS, PEN_THICKNESS, color);
}

void ILI9340_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	ILI9340_FillRect(x-PEN_THICKNESS/2,y-PEN_THICKNESS/2,PEN_THICKNESS,PEN_THICKNESS,color);
}

void ILI9340_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	ILI9340_HorizontalLine(x,y,w,color); //Top line
	ILI9340_HorizontalLine(x,(y+h),w,color); //bottom line
	ILI9340_VerticalLine(x,y,h,color); //left line
	ILI9340_VerticalLine((x+w),y,h,color); //right line
}

/**
 * Draw a filled rectangle with a horizontal gradient between c1 and c2
 */
void ILI9340_FillRect_HGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c1, uint16_t c2)
{
	uint32_t i = 0;

	if (w == 0 || h == 0) return;

	float ratio;

	uint8_t r1, g1, b1; //Individual color components
	uint8_t r2, g2, b2;

	uint16_t color;

	r1 = ILI9340_RedComponent(c1);
	g1 = ILI9340_GreenComponent(c1);
	b1 = ILI9340_BlueComponent(c1);

	r2 = ILI9340_RedComponent(c2);
	g2 = ILI9340_GreenComponent(c2);
	b2 = ILI9340_BlueComponent(c2);

	/*
	 * For a horizontal gradient, gradient changes every horizontal line
	 * Thus, at row = y, color = (r1, g1, b1)
	 * at row = (y + h), color = (r2, g2, b2)
	 */

	for (i=0;i<h;i++)
		{
			//Calculate ratio at each column
			ratio = (float) (i % h) / (float) h;
			color = ILI9340_ColorFromRGB(
					r1 + ratio * (r2 - r1),
					g1 + ratio * (g2 - g1),
					b1 + ratio * (b2 - b1));

			ILI9340_HorizontalLine(x,y+i,w,color);
		}
}

//Fill a rectangle with a vertical gradient between c1 and c2
void ILI9340_FillRect_VGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c1, uint16_t c2)
{
	uint16_t i = 0;

	if (w == 0 || h == 0) return;

	float ratio;

	uint8_t r1, g1, b1; //Individual color components
	uint8_t r2, g2, b2;

	uint16_t color;

	r1 = ILI9340_RedComponent(c1);
	g1 = ILI9340_GreenComponent(c1);
	b1 = ILI9340_BlueComponent(c1);

	r2 = ILI9340_RedComponent(c2);
	g2 = ILI9340_GreenComponent(c2);
	b2 = ILI9340_BlueComponent(c2);

	for (i=0;i<w;i++)
	{
		//Calculate ratio at each column
		ratio = (float) (i % w) / (float) w;
		color = ILI9340_ColorFromRGB(
				r1 + ratio * (r2 - r1),
				g1 + ratio * (g2 - g1),
				b1 + ratio * (b2 - b1));

		ILI9340_VerticalLine(x+i,y,h,color);
	}
}

void ILI9340_DrawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmapData)
{
	if (w == 0 || h == 0) return;

	uint32_t i = 0;
	uint32_t nPix = (uint32_t) w * (uint32_t) h;

	ILI9340_SetAddressWindow(x, y, x+w-1, y+h-1);

	ILI9340_DC_Ctrl(LCD_GPIO_H);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	for (i=0;i<(nPix-1);i++)
	{
		ILI9340_Tx_16bit(bitmapData[i]);
	}

	ILI9340_CS_Ctrl(LCD_GPIO_H);
}

void ILI9340_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	if (w == 0 || h == 0) return;

	uint32_t nPix = (uint32_t) w * (uint32_t) h;
 
 	ILI9340_SetAddressWindow(x, y, x+w-1, y+h-1);

	ILI9340_DC_Ctrl(LCD_GPIO_H);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	while (nPix > 0)
	{
		ILI9340_Tx_16bit(color);

		nPix--;
	}

	ILI9340_CS_Ctrl(LCD_GPIO_H);
}

void ILI9340_FillScreen(uint16_t color)
{
	ILI9340_FillRect(0, 0, ILI9340_WIDTH, ILI9340_HEIGHT, color);
}

void ILI9340_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ILI9340_DrawPixel(x0 , y0+r, color);
	ILI9340_DrawPixel(x0 , y0-r, color);
	ILI9340_DrawPixel(x0+r, y0 , color);
	ILI9340_DrawPixel(x0-r, y0 , color);

	while (x<y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;

		ddF_x += 2;

		f += ddF_x;

		ILI9340_DrawPixel(x0 + x, y0 + y, color);
		ILI9340_DrawPixel(x0 - x, y0 + y, color);
		ILI9340_DrawPixel(x0 + x, y0 - y, color);
		ILI9340_DrawPixel(x0 - x, y0 - y, color);
		ILI9340_DrawPixel(x0 + y, y0 + x, color);
		ILI9340_DrawPixel(x0 - y, y0 + x, color);
		ILI9340_DrawPixel(x0 + y, y0 - x, color);
		ILI9340_DrawPixel(x0 - y, y0 - x, color);
	}
}

void ILI9340_FillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
	int16_t rSquared = r * r;
	int16_t d;

	int16_t i,j;

	for (j=-r;j<r;j++)
	{
		for (i=-r;i<r;i++)
		{
			d = (i * i) + (j * j);
			if (d < 0) d *= -1;

			if (d <= rSquared)
			{
				ILI9340_DrawPixel(x+i,y+j,color);}
		}
	}
}

void ILI9340_ClearScreen()
{
	ILI9340_FillScreen(BACKGROUND_COLOR);
}

uint8_t ILI9340_RedComponent(uint16_t color)
{
	return (color >> 8) & 0xF8;
}

uint8_t ILI9340_GreenComponent(uint16_t color)
{
	return (color >> 3) & 0xFC;
}

uint8_t ILI9340_BlueComponent(uint16_t color)
{
	return (color << 3) & 0xFF;
}

uint16_t ILI9340_ColorFromRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return (uint16_t) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t ILI9340_DrawCharString(uint16_t x, uint16_t y, char* c, size_t len, uint16_t color)
{
	uint16_t x_cursor=x;

	for(int i=0; i<len; i++)
	{
		ILI9340_DrawChar(x_cursor, y, *(c +i), color);
		x_cursor= x_cursor +FONT_TEXT_OFFSET;
	}
	return x_cursor;
}


void ILI9340_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color)
{
	// Extract character data
	uint16_t address = getCharAddress(c);
	uint16_t i,j, ascii_data[2]; // Interators

	ILI9340_SetAddressWindow(x, y, x+FONT_9x16_WIDTH-1, y+FONT_9x16_HIGHT-1);

	ILI9340_DC_Ctrl(LCD_GPIO_H);
	ILI9340_CS_Ctrl(LCD_GPIO_L);

	for (j=0; j<FONT_9x16_HIGHT; j++)
	{
		ascii_data[0] = font9x16_ref[address];
		ascii_data[1] = font9x16_ref[address+1];

		for (i=0; i<8; i++)
		{
			if (ascii_data[0]& (0x1<<7-i))	ILI9340_Tx_16bit(color);
			else							ILI9340_Tx_16bit(BACKGROUND_COLOR);
		}
		if (ascii_data[1]& (0x1<<7))		ILI9340_Tx_16bit(color);
		else								ILI9340_Tx_16bit(BACKGROUND_COLOR);

		address=address+2;
	}

	ILI9340_CS_Ctrl(LCD_GPIO_H);

}

void ILI9340_Write_Mining_State(uint16_t y1, char* slv_index, char* result, char* difficulty, char* hashrate, char* elapsed_time_s)
{
	y1= (y1* FONT_HIGHT_OFFSET_ONE_BLOCK) + LCD_WIDTH_BASE_Y;
	uint16_t y2= y1+FONT_HIGHT_OFFSET_LINE_1;
	
	ILI9340_FillRect(0,y1,100,16,0);
	ILI9340_DrawCharString(FONT_WIDTH_X_SLAVE_TEXT , y1, "Slv"    , 3 , WHITE);
	ILI9340_DrawCharString(FONT_WIDTH_X_SLAVE_INDEX, y1, slv_index, LCD_STRING_SIZE_SLAVE_INDEX, WHITE); //remove
	ILI9340_DrawCharString(FONT_WIDTH_X_RESULT     , y1, result   , LCD_STRING_SIZE_RESULT, (0==strncmp(result, "GOOD",4))?WHITE:RED); //remove

	ILI9340_DrawCharString(FONT_WIDTH_X_DIFF_TEXT , y1, "Diff", 4, YELLOW);
	ILI9340_DrawCharString(FONT_WIDTH_X_DIFF_NUM  , y1, difficulty, LCD_STRING_SIZE_DIFFICULTY, YELLOW);//remove

	ILI9340_DrawCharString(FONT_WIDTH_X_HASH_NUM    , y2, hashrate, LCD_STRING_SIZE_HASH_RATE, GREEN);//remove
	ILI9340_DrawCharString(FONT_WIDTH_X_HASH_HS_TEXT, y2, "KH/s", 4, GREEN);

	ILI9340_DrawCharString(FONT_WIDTH_X_ELAPSED_TIME_TEXT1, y2, "("  , sizeof("(")-1  , BLUE);
	ILI9340_DrawCharString(FONT_WIDTH_X_ELAPSED_TIME_NUM  , y2, elapsed_time_s, LCD_STRING_SIZE_ELAP_TIME, BLUE);//remove
	ILI9340_DrawCharString(FONT_WIDTH_X_ELAPSED_TIME_TEXT2, y2, "s)" , sizeof("s)")-1 , BLUE);
}

void ILI9340_Reset_Ctrl(uint8_t signal)
{
	gpio_put(PIN_LCD_RESET, signal);
}

void ILI9340_DC_Ctrl(uint8_t signal)
{
	gpio_put(PIN_LCD_DC, signal);
}

void ILI9340_CS_Ctrl(uint8_t signal)
{
	gpio_put(PIN_LCD_CS, signal);
}

void ILI9340_spi_initialize(void)
{
    spi_init(SPI_LCD_PORT, ILI9340_SPI_SPEED);

    gpio_set_function(PIN_LCD_SCK , GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_LCD_CS);
    gpio_set_dir(PIN_LCD_CS, GPIO_OUT);
    gpio_put(PIN_LCD_CS, 1);

	gpio_init(PIN_LCD_DC);
    gpio_set_dir(PIN_LCD_DC, GPIO_OUT);
    gpio_put(PIN_LCD_DC, 1);

	gpio_init(PIN_LCD_LED);
    gpio_set_dir(PIN_LCD_LED, GPIO_OUT);
    gpio_put(PIN_LCD_LED, 1);

	gpio_init(PIN_LCD_RESET);
    gpio_set_dir(PIN_LCD_RESET, GPIO_OUT);
}

void ILI9340_init(void)
{
	ILI9340_spi_initialize();
	ILI9340_Reset();
    ILI9340_FillScreen(BACKGROUND_COLOR);

	ILI9340_DrawCharString(LCD_HIGHT_BASE_X, LCD_WIDTH_BASE_Y, "LODING..", sizeof("LODING..")-1, WHITE);
}
