#include "st7735.h"
#include "gpio_spi_helper.h"

// To establish the physical SPI communication and control link with the STM32, you will use:
// * SCL (Connected to STM32 SCK)
// * SDA (Connected to STM32 MOSI)
// * CS (Connected to an STM32 GPIO pin)
// * DC (Connected to an STM32 GPIO pin)
// * RES (Connected to an STM32 GPIO pin)


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//SPI SETTINGS CODE
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#define DELAY 0x80

// based on Adafruit ST7735 library for Arduino
static const uint8_t
  init_cmds1[] = {            // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      ST7735_ROTATION,        //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

#if (defined(ST7735_IS_128X128) || defined(ST7735_IS_160X128))
  init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127
#endif // ST7735_IS_128X128

#ifdef ST7735_IS_160X80
  init_cmds2[] = {            // Init for 7735S, part 2 (160x80 display)
    3,                        //  3 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x4F,             //     XEND = 79
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F ,            //     XEND = 159
    ST7735_INVOFF, 0 },        //  3: Invert colors ST7735_INVON
#endif

  init_cmds3[] = {            // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Gamma Adjustments (pos. polarity), 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Gamma Adjustments (neg. polarity), 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//SETUP CODE
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------
// 1. Initialize SysTick WITHOUT interrupts (TICKINT = 0 -> value 0x05 instead of 0x07)
void sysTick_Init(void) {
    *(volatile uint32_t *)0xE000E010 = 0x00000000; // Disable
    *(volatile uint32_t *)0xE000E014 = 72000 - 1;   // 1ms reload (assuming 72MHz)
    *(volatile uint32_t *)0xE000E018 = 0x00000000; // Clear current value
    *(volatile uint32_t *)0xE000E010 = 0x00000005; // Enable, CPU clock source, NO interrupt (0x5)
}

// 2. Safe blocking delay using VAL down-counter wrapping (No interrupts required)
void delay_Ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        // Clear current value to sync the start of this millisecond
        *(volatile uint32_t *)0xE000E018 = 0;
        
        // The VAL register counts down from 71999 to 0. 
        // We wait until it counts down (i.e., current value is greater than the previous read, 
        // meaning it reloaded, or we track the drop).
        uint32_t last_val = *(volatile uint32_t *)0xE000E018;
        
        while (1) {
            uint32_t current_val = *(volatile uint32_t *)0xE000E018;
            // If current_val is greater than last_val, a wrap-around happened (1ms passed)
            if (current_val > last_val) {
                break;
            }
            last_val = current_val;
        }
    }
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void setupSTM32(void){

    // --------------------------------------------------
    //  ENABLE CLOCK FOR GPIOA 
    // (Peripherals are off by default on STM32)
    // --------------------------------------------------
    // RCC APB2 peripheral clock enable register (RCC_APB2ENR) 
    // Bit 2 is IOPAEN (Port A clock enable). offset 0x18.
    *(volatile uint32_t *)(RCC_BASE_ADDRESS + RCC_APB2ENR_REGISTER_OFFSET) |= (1 << GPIO_ENABLE_CONFIG_BIT_POS);

    // --------------------------------------------------
    //  ENABLE CLOCK FOR SPI1
    // --------------------------------------------------
    // RCC APB2 peripheral clock enable register (RCC_APB2ENR) 
    // Bit 12 is SPI1EN
    *(volatile uint32_t *)(RCC_BASE_ADDRESS + RCC_APB2ENR_REGISTER_OFFSET) |= (1 << SPI1_ENABLE_CONFIG_BIT_POS);


    // --------------------------------------------------
    //  CONFIGURE GPIO PINS FOR USE
    // --------------------------------------------------
     
    //-----------------------------
    //CONFIGURE CHIP_SELECT PIN 
    // Clear the 4 bits 
    GPIOA_CRL_REGISTER &= ~(0x0F << (CHIP_SELECT_PIN_N * GPIO_SIZE));

    // Set Pin to Output mode (Max speed 2MHz, Push-Pull -> value 0x2)
    GPIOA_CRL_REGISTER |=  (0x02 << (CHIP_SELECT_PIN_N * GPIO_SIZE));
    
    //-----------------------------
    //CONFIGURE DATA_COMMAND_PIN PIN 
    // Clear the 4 bits 
    GPIOA_CRL_REGISTER &= ~(0x0F << (DATA_COMMAND_PIN_N * GPIO_SIZE));

    // Set Pin to Output mode (Max speed 2MHz, Push-Pull -> value 0x2)
    GPIOA_CRL_REGISTER |=  (0x02 << (DATA_COMMAND_PIN_N * GPIO_SIZE));
    
    //-----------------------------
    //CONFIGURE RESET_PIN PIN 
    // Clear the 4 bits 
    GPIOA_CRL_REGISTER &= ~(0x0F << (RESET_PIN_N * GPIO_SIZE));

    // Set Pin to Output mode (Max speed 2MHz, Push-Pull -> value 0x2)
    GPIOA_CRL_REGISTER |=  (0x02 << (RESET_PIN_N * GPIO_SIZE));


    // --------------------------------------------------
    //  CONFIGURE GPIO BUTTON PINS FOR USE
    // --------------------------------------------------

    //change color btn
    // 1. Clear the 4 bits for Pin 9 in CRH
    GPIOA_CRH_REGISTER &= ~(0x0F << ((CHANGE_COLOR_BTN_PIN_N - 8) * 4));

    // 2. Set Pin 9 to Input mode with pull-up/pull-down (Binary 1000 = 0x8)
    GPIOA_CRH_REGISTER |=  (0x08 << ((CHANGE_COLOR_BTN_PIN_N - 8) * 4));

    // 3. Enable Pull-UP by setting the ODR bit to 1 (when released, it reads HIGH/1)
    GPIOA_ODR_REGISTER |= (1 << CHANGE_COLOR_BTN_PIN_N);

    // animation  minus btn
    // 1. Clear the 4 bits for Pin 10 in CRH
    GPIOA_CRH_REGISTER &= ~(0x0F << ((ANIMATION_MINUS_BTN_PIN_N - 8) * 4));

    // 2. Set Pin 10 to Input mode with pull-up/pull-down (Binary 1000 = 0x8)
    GPIOA_CRH_REGISTER |=  (0x08 << ((ANIMATION_MINUS_BTN_PIN_N - 8) * 4));

    // 3. Enable Pull-UP by setting the ODR bit to 1 (when released, it reads HIGH/1)
    GPIOA_ODR_REGISTER |= (1 << ANIMATION_MINUS_BTN_PIN_N);

    // animation  plus  btn
    // 1. Clear the 4 bits for Pin 11 in CRH
    GPIOA_CRH_REGISTER &= ~(0x0F << ((ANIMATION_PLUS_BTN_PIN_N - 8) * 4));

    // 2. Set Pin 11 to Input mode with pull-up/pull-down (Binary 1000 = 0x8)
    GPIOA_CRH_REGISTER |=  (0x08 << ((ANIMATION_PLUS_BTN_PIN_N - 8) * 4));

    // 3. Enable Pull-UP by setting the ODR bit to 1 (when released, it reads HIGH/1)
    GPIOA_ODR_REGISTER |= (1 << ANIMATION_PLUS_BTN_PIN_N);


    // --------------------------------------------------
    // CONFIGURE SPI1 PINS FOR USE
    // --------------------------------------------------
    //both spi1 pins are set like this:
    //configuration:  Alternate function output Push-Pull
    //mode :  Output mode, max speed 50 MHz
    
    // SPI1_SCLK pin (serial clock)
    GPIOA_CRL_REGISTER &= ~(0x0F << (SPI1_SCLK_N * GPIO_SIZE));
    GPIOA_CRL_REGISTER |=  (0x0B << (SPI1_SCLK_N * GPIO_SIZE));

    // SPI1_MOSI_PIN (master out/slave in)
    GPIOA_CRL_REGISTER &= ~(0x0F << (SPI1_MOSI_PIN_N * GPIO_SIZE)); //clear bits
    GPIOA_CRL_REGISTER |=  (0x0B << (SPI1_MOSI_PIN_N * GPIO_SIZE));

    // --------------------------------------------------
    // CONFIGURE SPI1 SETTINGS
    // --------------------------------------------------
    // set up SPI1 as a Master with an 8-bit frame format
    // * Bit 6 (SPE): SPI Enable (1 = Enable SPI)
    // * Bit 3-5 (BR[2:0]): Baud Rate Control (Sets your clock divider, e.g., dividing APB2 clock down to your target speed).
    // * Bit 2 (MSTR): Master Selection (1 = Master configuration)
    // * Bit 1 (CPOL): Clock Polarity (0 = Clock is low when idle)
    // * Bit 0 (CPHA): Clock Phase (0 = First clock transition is the first data capture edge)
    // * Bit 9 (SSM): Software Slave Management (1 = Manage NSS pin via software)
    // * Bit 8 (SSI): Internal Slave Select (1 = Keep internal slave select high so master mode isn't faulted)

    //The ST7735 controller typically expects CPOL = 0 and CPHA = 0 (Mode 0) or CPOL = 1 and CPHA = 1 (Mode 3).
    // SPI_CR1_REGISTER = (1 << 2) | (1 << 6) | (1 << 8) | (1 << 9) | (2 << 3);

    // or
    SPI_CR1_REGISTER = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 6) | (1 << 8) | (1 << 9) | (0 << 3);

};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void ST7735_ExecuteCommandList(const uint8_t *addr) {
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++; // First byte is the number of commands in this list
    
    while (numCommands--) {
        uint8_t cmd = *addr++;
        LCD_WriteCommand(cmd);

        numArgs = *addr++; // Number of arguments/bytes to follow
        
        // Check if the high bit is set for a delay flag
        if (numArgs & DELAY) {
            numArgs &= ~DELAY; // Clear the delay flag mask
            
            // Send any remaining arguments first
            for (int i = 0; i < numArgs; i++) {
                LCD_WriteData(*addr++);
            }
            
            // Next byte is the delay duration in milliseconds
            ms = *addr++;
            if (ms == 255) ms = 500; // Handle long delay extension
            
            delay_Ms(ms);

        } else {
            // Send standard data arguments
            for (int i = 0; i < numArgs; i++) {
                LCD_WriteData(*addr++);
            }
        }
    }
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void ST7735_Init(void) {
    // 1. Hardware Reset pulse (if using a RES pin)
    // Pull RES low,
    GPIOA_BRR_REGISTER = (1 << RESET_PIN_N);
    
    // wait 150ms 
    delay_Ms((150));
    
    //pull RES high    
    GPIOA_BSRR_REGISTER = (1 << RESET_PIN_N);

    // wait 50ms 
    delay_Ms((150));

    // 2. Run the initialization blocks sequentially
    ST7735_ExecuteCommandList(init_cmds1);
    ST7735_ExecuteCommandList(init_cmds2); // Choose your specific screen size array
    ST7735_ExecuteCommandList(init_cmds3);
    
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//SPI HELPER METHODS

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Open the data stream: CS Low, D/C High (Data mode)
void LCD_StartData(void) {
    GPIOA_BRR_REGISTER = (1 << CHIP_SELECT_PIN_N);   // CS Low
    GPIOA_BSRR_REGISTER = (1 << DATA_COMMAND_PIN_N); // D/C High (Data)
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Close the data stream: Wait for transmission to finish, then CS High
void LCD_EndData(void) {
    while (SPI_SR_REGISTER & (1 << 7));              // Wait until SPI is completely not busy (BSY bit)
    GPIOA_BSRR_REGISTER = (1 << CHIP_SELECT_PIN_N);  // CS High
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 1. Raw byte sender (assumes CS is already held LOW by the caller)
void SPI1_SendByte(uint8_t data) {
    while (!(SPI_SR_REGISTER & (1 << 1))); // Wait for TXE
    SPI_DR_REGISTER = data;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 2. Write a single command byte (CS goes low, sends command, CS goes high)
void LCD_WriteCommand(uint8_t cmd) {
    GPIOA_BRR_REGISTER = (1 << CHIP_SELECT_PIN_N);  // CS Low
    GPIOA_BRR_REGISTER = (1 << DATA_COMMAND_PIN_N); // D/C Low (Command)
    
    SPI1_SendByte(cmd);
    
    while (SPI_SR_REGISTER & (1 << 7));               // Wait until SPI is completely done (not busy)
    GPIOA_BSRR_REGISTER = (1 << CHIP_SELECT_PIN_N);  // CS High
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 3. Write a single data byte (CS goes low, sends data, CS goes high)
void LCD_WriteData(uint8_t data) {
    GPIOA_BRR_REGISTER = (1 << CHIP_SELECT_PIN_N);  // CS Low
    GPIOA_BSRR_REGISTER = (1 << DATA_COMMAND_PIN_N);// D/C High (Data)
    
    SPI1_SendByte(data);
    
    while (SPI_SR_REGISTER & (1 << 7));               // Wait until SPI is completely done
    GPIOA_BSRR_REGISTER = (1 << CHIP_SELECT_PIN_N);  // CS High
}