
//written for a STM32F103C8T6 and a 80x60 screen SPI with 16-bit colors


#include <stdio.h>
#include "gpio_spi_helper.h"
#include "st7735.h"


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
int main(void) {
    //setup stm32 (bus, peripherials,pins...)
    setupSTM32();

    // 2. Initialize the SysTick timer ( once!)
    sysTick_Init();

    //setup st7735 (lcd screen)
    ST7735_Init();

    //draw test squares

    ST7735_DrawFilledRect(0, 0, 80, 160, ST7735_BLACK); // Fill with Black / clear screen

    ST7735_DrawFilledRect(0, 0, 30, 30, ST7735_GREEN);
    ST7735_DrawFilledRect(40, 0, 30, 30, ST7735_CORAL);
    ST7735_DrawFilledRect(80, 0, 30, 30, ST7735_YELLOW);
    ST7735_DrawFilledRect(0, 40, 30, 30, ST7735_ORANGE_1);
    ST7735_DrawFilledRect(40, 40, 30, 30, ST7735_RED);
    ST7735_DrawFilledRect(80, 40, 30, 30, ST7735_INDIGO);



    // Keep the program running so it doesn't exit
    while (1) {
        // Do nothing, keep screen active
    }
}