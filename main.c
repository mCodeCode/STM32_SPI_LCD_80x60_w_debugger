/*
********************************************************************************
*                                                                              *
* Copyright (c) 2017 Andrea Loi                                                *
*                                                                              *
* Permission is hereby granted, free of charge, to any person obtaining a      *
* copy of this software and associated documentation files (the "Software"),   *
* to deal in the Software without restriction, including without limitation    *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,     *
* and/or sell copies of the Software, and to permit persons to whom the        *
* Software is furnished to do so, subject to the following conditions:         *
*                                                                              *
* The above copyright notice and this permission notice shall be included      *
* in all copies or substantial portions of the Software.                       *
*                                                                              *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL      *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER   *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER          *
* DEALINGS IN THE SOFTWARE.                                                    *
*                                                                              *
********************************************************************************
* Your code goes here.                                                         *
********************************************************************************
*/

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

    // ST7735_DrawFilledRect(0, 0, 80, 160, ST7735_WHITE); // Fill with Black
    ST7735_DrawFilledRect(0, 0, 30, 30, ST7735_GREEN);
    ST7735_DrawFilledRect(40, 0, 30, 30, ST7735_ORANGE_1);
    ST7735_DrawFilledRect(0, 40, 30, 30, ST7735_YELLOW);
    ST7735_DrawFilledRect(40, 40, 30, 30, ST7735_WHITE);


    // Keep the program running so it doesn't exit
    while (1) {
        // Do nothing, keep screen active
    }
}