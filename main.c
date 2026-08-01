
//written for a STM32F103C8T6 and a 80x60 screen SPI with 16-bit colors


#include <stdio.h>
#include "gpio_spi_helper.h"
#include "st7735.h"


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
float currentCubeAngleY = 0.0f;
float currentPyramidAngleY = 0.0f;

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


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void cubeAnimation(void){
    // Fill with Black --- clear screen
    // ST7735_DrawFilledRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, ST7735_BLACK);
    ST7735_DrawBoxWireFrame(0, 0, 0, cubeSize.x, cubeSize.y, cubeSize.z, currentCubeAngleY, currentCubeAngleY, currentCubeAngleY, ST7735_BLACK);
            
    // Increment angle for the next frame
    currentCubeAngleY += 4.0f;
    
    // Keep the angle within 0 - 360 degrees to prevent overflow
    if (currentCubeAngleY >= 360.0f) {
        currentCubeAngleY -= 360.0f;
    }

    
    ST7735_DrawBoxWireFrame(0, 0, 0, cubeSize.x, cubeSize.y, cubeSize.z, currentCubeAngleY, currentCubeAngleY, currentCubeAngleY, ST7735_FOREST_GREEN);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void pyramidAnimation(void){

    
    // Fill with Black --- clear previous frame
    ST7735_DrawPyramidWireFrame(0, -10, 0, pyramidSize.x, pyramidSize.y, pyramidSize.z, 0, currentPyramidAngleY, 0, ST7735_BLACK);
            
    // Increment angle for the next frame
    currentPyramidAngleY += 4.0f;
    
    // Keep the angle within 0 - 360 degrees to prevent overflow
    if (currentPyramidAngleY >= 360.0f) {
        currentPyramidAngleY -= 360.0f;
    }

    
    ST7735_DrawPyramidWireFrame(0, -10, 0, pyramidSize.x, pyramidSize.y, pyramidSize.z, 0, currentPyramidAngleY, 0, ST7735_FOREST_GREEN);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void octahedronAnimation(void){

    
    // Fill with Black --- clear previous frame
    ST7735_DrawPyramidWireFrame(0, -15, 0, 30, 30, 30, 0, currentPyramidAngleY, 0, ST7735_BLACK);
    ST7735_DrawPyramidWireFrame(0, 15, 0, 30, 30, 30, 180.0f, currentPyramidAngleY, 0, ST7735_BLACK);
            
    // Increment angle for the next frame
    currentPyramidAngleY += 4.0f;
    
    // Keep the angle within 0 - 360 degrees to prevent overflow
    if (currentPyramidAngleY >= 360.0f) {
        currentPyramidAngleY -= 360.0f;
    }

    
    ST7735_DrawPyramidWireFrame(0, -15, 0, 30, 30, 30, 0, currentPyramidAngleY, 0, ST7735_FOREST_GREEN);
    ST7735_DrawPyramidWireFrame(0, 15, 0, 30, 30, 30, 180.0f, currentPyramidAngleY, 0, ST7735_FOREST_GREEN);
}



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
    

    // Keep the program running so it doesn't exit
    while (1) {


        //--------------------------------------------------------------------
        //--------------------------------------------------------------------
        //rotate a cube in realtime
        // cubeAnimation();

        //--------------------------------------------------------------------
        //--------------------------------------------------------------------
        //rotate a piramid in realtime
        // pyramidAnimation();

        //--------------------------------------------------------------------
        //--------------------------------------------------------------------
        octahedronAnimation();

        //--------------------------------------------------------------------
        //--------------------------------------------------------------------


        //--------------------------------------------------------------------
        //--------------------------------------------------------------------
    }
}