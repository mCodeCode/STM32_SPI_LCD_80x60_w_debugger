
#ifndef GPIO_SPI_HELPER_H
#define GPIO_SPI_HELPER_H


// *****************************************************************************
// *                          < Sized integer types >                          *
// *****************************************************************************
// * If you're including stdint.h you must remove the lines below.             *
// *****************************************************************************
#define int32_t     int
#define int16_t     short
#define int8_t      char
#define uint32_t    unsigned int
#define uint16_t    unsigned short
#define uint8_t     unsigned char


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_ORANGE_1   0xFD20
#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// To establish the physical SPI communication and control link with the STM32, you will use:
// * SCL (Connected to STM32 SCK)
// * SDA (Connected to STM32 MOSI)
// * CS (Connected to an STM32 GPIO pin)
// * DC (Connected to an STM32 GPIO pin)
// * RES (Connected to an STM32 GPIO pin)


// 1. Define the base address and offset
#define GPIOA_BASE_ADDRESS           0x40010800
#define GPIOA_CRL_REGISTER_OFFSET    0x00
#define GPIOA_BSRR_REGISTER_OFFSET   0x10
#define GPIOA_BRR_REGISTER_OFFSET    0x14

#define RCC_BASE_ADDRESS             0x40021000
#define RCC_APB2ENR_REGISTER_OFFSET  0x18


//SPI -----
#define SPI_BASE_ADDRESS             0x40013000
//for the spi peripherial [settings] register and enable
#define SPI_CR1_REGISTER_OFFSET      0x00

//spi Status Register 
// Used to check if the peripheral is busy or if the transmission buffer is ready for new data.
#define SPI_SR_REGISTER_OFFSET      0x08

//spi peripherial [transmission] register
//Writing a byte to this register 
//automatically triggers the hardware to shift it out over the MOSI pin (PA7).
#define SPI_DR_REGISTER_OFFSET      0x0C


//REGISTER SELECTION INSIDE GPIO PERIPHERIAL PORT
#define GPIOA_CRL_REGISTER  (*(volatile uint32_t *)(GPIOA_BASE_ADDRESS + GPIOA_CRL_REGISTER_OFFSET))

#define GPIOA_BSRR_REGISTER  (*(volatile uint32_t *)(GPIOA_BASE_ADDRESS + GPIOA_BSRR_REGISTER_OFFSET))

#define GPIOA_BRR_REGISTER  (*(volatile uint32_t *)(GPIOA_BASE_ADDRESS + GPIOA_BRR_REGISTER_OFFSET))

//REGISTER SELECTION INSIDE SPI PERIPHERIAL
#define SPI_CR1_REGISTER (*(volatile uint32_t *)(SPI_BASE_ADDRESS + SPI_CR1_REGISTER_OFFSET))
#define SPI_SR_REGISTER (*(volatile uint32_t *)(SPI_BASE_ADDRESS + SPI_SR_REGISTER_OFFSET))
#define SPI_DR_REGISTER (*(volatile uint32_t *)(SPI_BASE_ADDRESS + SPI_DR_REGISTER_OFFSET))


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

//GPIO PINS  //const uint32_t
#define GPIO_SIZE          4
#define CHIP_SELECT_PIN_N  1
#define DATA_COMMAND_PIN_N 2
#define RESET_PIN_N        3

//SPI PINS 
//according to the reference manual, the spi pins are :
// chip select pin, can use gpio in output mode too, this pin is optinal
//but must remain free on the board
// SPI1_NSS ---> PA4 
//not needed

// Serial Clock output for SPI masters and input for SPI slaves.
// SPI1_SCK ---> PA5
#define SPI1_SCLK_N 5
// SPI1_MISO ---> PA6
//not needed
// SPI1_MOSI ---> PA7
#define SPI1_MOSI_PIN_N 7
//this pins are the ones by default assigned to spi, you can select another group by setting
// a remap parameter, but is not needed here

#define GPIO_ENABLE_CONFIG_BIT_POS 2
#define SPI1_ENABLE_CONFIG_BIT_POS 12


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void setupSTM32(void);
void delay_Ms(uint32_t ms);
void sysTick_Init(void);
void SPI1_SendByte(uint8_t data);
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);

#endif
