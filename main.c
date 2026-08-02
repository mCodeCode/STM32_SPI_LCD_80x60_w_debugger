
//written for a STM32F103C8T6 and a 80x60 screen SPI with 16-bit colors


#include <stdio.h>
#include "gpio_spi_helper.h"
#include "st7735.h"


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

Vector3D cubeSize = {
    35, 
    35,
    35
};

Vector3D pyramidSize = {
    50, 
    50,
    50
};

Vector3D octaSize = {
    35, 
    35,
    35
};

Vector3D dodecahedronSize = {
    30, 
    30,
    30
};



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
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
    
    // Fill with Black --- clear screen
    ST7735_DrawFilledRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, ST7735_BLACK); 

    //------------------------------------------------------
    //------------------------------------------------------
    //------------------------------------------------------
    //------------------------------------------------------

    uint32_t frameCounter = 0;
    uint8_t currentShapeIndex = 0;
    uint16_t colorPalette[] = {ST7735_FOREST_GREEN, ST7735_INDIGO, ST7735_ORANGE_1, ST7735_TURQUOISE, ST7735_GOLD, ST7735_CRIMSON, ST7735_WHITE};
    uint8_t colorIndex = 0;

    // Track rotation angles (matching float or int depending on your precomputeTrig function)
    float angleX = 0, angleY = 0, angleZ = 0; 

    // Keep this variable outside your while(1) loop (e.g., right before it)
    uint8_t lastColorButtonState = 0;
    uint8_t lastAnimMinusButtonState = 0;
    uint8_t lastAnimPlusButtonState = 0;

    while (1) {

        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //ANIMATION LOGIC
        // 1. Frame-based transition counter (switches every ~100 frames)
        frameCounter++;
        if (frameCounter >= 60) {
            frameCounter = 0; // Reset counter
            
            currentShapeIndex = (currentShapeIndex + 1) % 4; // Cycle shapes (0 to 3)
            colorIndex = (colorIndex + 1) % 7;             // Cycle colors (0 to 6)
        }

        // 2. Clear screen / update animation angles
        ST7735_DrawFilledRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, ST7735_BLACK);
        angleX += 2.0f;
        angleY += 3.0f;
        angleZ += 3.0f;

        // 3. Draw the active shape from the playlist loop
        if (currentShapeIndex == 0) {
            ST7735_DrawBoxWireFrame(0, 0, 0, cubeSize.x, cubeSize.y, cubeSize.z, angleX, angleY, angleZ, colorPalette[colorIndex]);
        } 
        else if (currentShapeIndex == 1) {
            ST7735_DrawPyramidWireFrame(0, -5, 0, pyramidSize.x, pyramidSize.y, pyramidSize.z, 0, angleY, 0, colorPalette[colorIndex]);
        } 
        else if (currentShapeIndex == 2) {
            ST7735_DrawOctahedronWireFrame(0, -20, 0, octaSize.x, octaSize.y, octaSize.z, 0, angleY, 0, colorPalette[colorIndex]);
        } 
        else if (currentShapeIndex == 3) {
            ST7735_DrawDodecahedronWireFrame(0, 0, 0, 30, 0, angleY, 0, colorPalette[colorIndex]);
        }


        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //------------------------------------------------------------------
        //BUTTON INTERACTIONS
        
        //------------------------------------------------------------------
        //CHANGE COLOR OF CURRENT SHAPE
        // 2. Read current button state (Inverted: 0 when pressed, 1 when released)
        uint8_t currentColorButtonState = (GPIOA_IDR_REGISTER & (1 << CHANGE_COLOR_BTN_PIN_N)) ? 1 : 0;
        
        // 3. Detect press transition (trigger on the falling edge: unpressed [1] -> pressed [0])
        if (!currentColorButtonState && lastColorButtonState) {
            // Simple software debounce
            for (volatile int i = 0; i < 10000; i++);
            
            // Re-verify state (ensure it's still pressed / 0)
            if (!(GPIOA_IDR_REGISTER & (1 << CHANGE_COLOR_BTN_PIN_N))) {
                colorIndex = (colorIndex + 1) % 7; 
            }
        }
        
        // Save state for the next loop iteration
        lastColorButtonState = currentColorButtonState;
        
        
        //------------------------------------------------------------------
        //CHANGE ANIMATION LEFT
        // 2. Read current button state (Inverted: 0 when pressed, 1 when released)
        uint8_t animMinusButtonState = (GPIOA_IDR_REGISTER & (1 << ANIMATION_MINUS_BTN_PIN_N)) ? 1 : 0;

        // 3. Detect press transition (trigger on the falling edge: unpressed [1] -> pressed [0])
        if (!animMinusButtonState && lastAnimMinusButtonState) {
            // Simple software debounce
            for (volatile int i = 0; i < 10000; i++);
            
            // Re-verify state (ensure it's still pressed / 0)
            if (!(GPIOA_IDR_REGISTER & (1 << ANIMATION_MINUS_BTN_PIN_N))) {
                currentShapeIndex = (currentShapeIndex - 1 + 4) % 4; 
            }
        }

        // Save state for the next loop iteration
        lastAnimMinusButtonState = animMinusButtonState;



        //------------------------------------------------------------------
        //CHANGE ANIMATION RIGHT
        // 2. Read current button state (Inverted: 0 when pressed, 1 when released)
        uint8_t animPlusButtonState = (GPIOA_IDR_REGISTER & (1 << ANIMATION_PLUS_BTN_PIN_N)) ? 1 : 0;

        // 3. Detect press transition (trigger on the falling edge: unpressed [1] -> pressed [0])
        if (!animPlusButtonState && lastAnimPlusButtonState) {
            // Simple software debounce
            for (volatile int i = 0; i < 10000; i++);
            
            // Re-verify state (ensure it's still pressed / 0)
            if (!(GPIOA_IDR_REGISTER & (1 << ANIMATION_PLUS_BTN_PIN_N))) {
                currentShapeIndex = (currentShapeIndex + 1) % 4; 
            }
        }

        // Save state for the next loop iteration
        lastAnimPlusButtonState = animPlusButtonState;

    }

}