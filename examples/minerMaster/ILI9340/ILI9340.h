#ifndef ILI9340_H_
#define ILI9340_H_

#include "ILI9340_defines.h"
#include "ILI9340_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Low-level inteface functions
 * These functions must be provided in user code
 */

#define LCD_HIGHT_BASE_X            5
#define LCD_WIDTH_BASE_Y            10

#define LCD_STRING_SIZE_SLAVE_INDEX 2
#define LCD_STRING_SIZE_RESULT      4
#define LCD_STRING_SIZE_HASH_RATE   6
#define LCD_STRING_SIZE_DIFFICULTY  7
#define LCD_STRING_SIZE_ELAP_TIME   6

#define LCD_DUINO_INFO_MEM_ARR_SIZE     6
#define LCD_DUINO_INFO_MEM_CNT_MAX      LCD_DUINO_INFO_MEM_ARR_SIZE-1 //0~5
#define LCD_DUINO_INFO_PRINT_CNT_MAX    LCD_DUINO_INFO_MEM_ARR_SIZE //1~6

void ILI9340_DC_Ctrl(uint8_t signal);
void ILI9340_CS_Ctrl(uint8_t signal);
void ILI9340_Reset_Ctrl(uint8_t signal);

void ILI9340_TxRx_8bit(uint8_t data);
void ILI9340_Tx_16bit(uint16_t data);

void ILI9340_PauseMs(uint32_t ms);

// Generic graphic functions
uint16_t ILI9340_ColorFromRGB(uint8_t r, uint8_t g, uint8_t b); //!< Convert an RGB color into LCD color
uint8_t ILI9340_RedComponent(uint16_t);
uint8_t ILI9340_GreenComponent(uint16_t);
uint8_t ILI9340_BlueComponent(uint16_t);
void ILI9340_SetPenThickness(uint16_t t);
void ILI9340_SetTextOptions(uint16_t opt);
void ILI9340_SetBackgroundColor(uint16_t bg);
void ILI9340_SetTextSize(uint8_t ts);
uint8_t ILI9340_GetTextSize();

// SPI functions
void ILI9340_WriteCommand(uint8_t command);
void ILI9340_WriteData(uint8_t data);
void ILI9340_WriteDataBytes(uint16_t *data, uint16_t size);

void ILI9340_Reset();
void ILI9340_init(void);
void ILI9340_Initialize();	
void ILI9340_Rotate(uint8_t dir);
void ILI9340_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void ILI9340_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9340_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9340_FillScreen(uint16_t color);
void ILI9340_ClearScreen();	//!< Clear the screen (background color)

void ILI9340_DrawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmapData);

void ILI9340_FillRect_HGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c1, uint16_t c2);
void ILI9340_FillRect_VGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c1, uint16_t c2);

void ILI9340_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

void ILI9340_FillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void ILI9340_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

void ILI9340_HorizontalLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void ILI9340_VerticalLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);

//Character functions
void ILI9340_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color);
void ILI9340_DrawString(uint16_t x, uint16_t y, char *data, uint16_t color);
uint16_t ILI9340_DrawCharString(uint16_t x, uint16_t y, char* c, size_t len, uint16_t color);
void ILI9340_Write_Mining_State(uint16_t y, char* slv_index, char* result, char* difficulty, char* hashrate, char* elapsed_time_s);
#ifdef __cplusplus
}
#endif

#endif /* SRC_ILI9340_H_ */
