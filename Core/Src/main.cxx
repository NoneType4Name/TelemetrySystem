/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dcmi.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_dma_ex.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// extern "C"
// {
// #include "ov2640.h"
// }
#include "driver_ov2640.h"
extern "C"
{
#include "ov2640_interface.h"
}
#include "stm32h750xx.h"
#include "stm32h7xx_hal_gpio.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// #define VAL_SET( x, mask, rshift, lshift ) \
//     ( ( ( ( x ) >> rshift ) & mask ) << lshift )
// #define HSIZE                0x51 /* H_SIZE[7:0] (real/4) */
// #define HSIZE_SET( x )       VAL_SET( x, 0xFF, 2, 0 )
// #define VSIZE                0x52 /* V_SIZE[7:0] (real/4) */
// #define VSIZE_SET( x )       VAL_SET( x, 0xFF, 2, 0 )
// #define XOFFL                0x53 /* OFFSET_X[7:0] */
// #define XOFFL_SET( x )       VAL_SET( x, 0xFF, 0, 0 )
// #define YOFFL                0x54 /* OFFSET_Y[7:0] */
// #define YOFFL_SET( x )       VAL_SET( x, 0xFF, 0, 0 )
// #define VHYX                 0x55 /* Offset and size completion */
// #define VHYX_VSIZE_SET( x )  VAL_SET( x, 0x1, ( 8 + 2 ), 7 )
// #define VHYX_HSIZE_SET( x )  VAL_SET( x, 0x1, ( 8 + 2 ), 3 )
// #define VHYX_YOFF_SET( x )   VAL_SET( x, 0x3, 8, 4 )
// #define VHYX_XOFF_SET( x )   VAL_SET( x, 0x3, 8, 0 )
// #define DPRP                 0x56
// #define TEST                 0x57 /* Horizontal size completion */
// #define TEST_HSIZE_SET( x )  VAL_SET( x, 0x1, ( 9 + 2 ), 7 )
// #define ZMOW                 0x5A /* Zoom: Out Width  OUTW[7:0] (real/4) */
// #define ZMOW_OUTW_SET( x )   VAL_SET( x, 0xFF, 2, 0 )
// #define ZMOH                 0x5B /* Zoom: Out Height OUTH[7:0] (real/4) */
// #define ZMOH_OUTH_SET( x )   VAL_SET( x, 0xFF, 2, 0 )
// #define ZMHH                 0x5C /* Zoom: Speed and H&W completion */
// #define ZMHH_ZSPEED_SET( x ) VAL_SET( x, 0x0F, 0, 4 )
// #define ZMHH_OUTH_SET( x )   VAL_SET( x, 0x1, ( 8 + 2 ), 2 )
// #define ZMHH_OUTW_SET( x )   VAL_SET( x, 0x3, ( 8 + 2 ), 0 )
// #define HSIZE8               0xC0 /* Image Horizontal Size HSIZE[10:3] */
// #define HSIZE8_SET( x )      VAL_SET( x, 0xFF, 3, 0 )
// #define VSIZE8               0xC1 /* Image Vertical Size VSIZE[10:3] */
// #define VSIZE8_SET( x )      VAL_SET( x, 0xFF, 3, 0 )
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c1;

SD_HandleTypeDef hsd1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( void );
static void MPU_Config( void );
static void MX_GPIO_Init( void );
static void MX_DMA_Init( void );
static void MX_I2C1_Init( void );
/* USER CODE BEGIN PFP */

void HAL_DMA_CpltCallback( DMA_HandleTypeDef * );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// const unsigned char OV2640_RGB565_INIT[][ 2 ] = {
//     { 0xff, 0x00 },
//     { 0x2c, 0xff },
//     { 0x2e, 0xdf },
//     { 0xff, 0x01 },
//     { 0x3c, 0x32 },
//     { 0x11, 0x00 },
//     { 0x09, 0x02 },
//     { 0x04, 0xD8 },
//     { 0x13, 0xe5 },
//     { 0x14, 0x48 },
//     { 0x2c, 0x0c },
//     { 0x33, 0x78 },
//     { 0x3a, 0x33 },
//     { 0x3b, 0xfB },
//     { 0x3e, 0x00 },
//     { 0x43, 0x11 },
//     { 0x16, 0x10 },
//     { 0x39, 0x92 },
//     { 0x35, 0xda },
//     { 0x22, 0x1a },
//     { 0x37, 0xc3 },
//     { 0x23, 0x00 },
//     { 0x34, 0xc0 },
//     { 0x36, 0x1a },
//     { 0x06, 0x88 },
//     { 0x07, 0xc0 },
//     { 0x0d, 0x87 },
//     { 0x0e, 0x41 },
//     { 0x4c, 0x00 },
//     { 0x48, 0x00 },
//     { 0x5B, 0x00 },
//     { 0x42, 0x03 },
//     { 0x4a, 0x81 },
//     { 0x21, 0x99 },
//     { 0x24, 0x40 },
//     { 0x25, 0x38 },
//     { 0x26, 0x82 },
//     { 0x5c, 0x00 },
//     { 0x63, 0x00 },
//     { 0x46, 0x22 },
//     { 0x0c, 0x3c },
//     { 0x61, 0x70 },
//     { 0x62, 0x80 },
//     { 0x7c, 0x05 },
//     { 0x20, 0x80 },
//     { 0x28, 0x30 },
//     { 0x6c, 0x00 },
//     { 0x6d, 0x80 },
//     { 0x6e, 0x00 },
//     { 0x70, 0x02 },
//     { 0x71, 0x94 },
//     { 0x73, 0xc1 },
//     { 0x3d, 0x34 },
//     { 0x5a, 0x57 },
//     { 0x12, 0x40 },
//     { 0x17, 0x11 },
//     { 0x18, 0x43 },
//     { 0x19, 0x00 },
//     { 0x1a, 0x4b },
//     { 0x32, 0x09 },
//     { 0x37, 0xc0 },
//     { 0x4f, 0xca },
//     { 0x50, 0xa8 },
//     { 0x5a, 0x23 },
//     { 0x6d, 0x00 },
//     { 0x3d, 0x38 },
//     { 0xff, 0x00 },
//     { 0xe5, 0x7f },
//     { 0xf9, 0xc0 },
//     { 0x41, 0x24 },
//     { 0xe0, 0x14 },
//     { 0x76, 0xff },
//     { 0x33, 0xa0 },
//     { 0x42, 0x20 },
//     { 0x43, 0x18 },
//     { 0x4c, 0x00 },
//     { 0x87, 0xd5 },
//     { 0x88, 0x3f },
//     { 0xd7, 0x03 },
//     { 0xd9, 0x10 },
//     { 0xd3, 0x82 },
//     { 0xc8, 0x08 },
//     { 0xc9, 0x80 },
//     { 0x7c, 0x00 },
//     { 0x7d, 0x00 },
//     { 0x7c, 0x03 },
//     { 0x7d, 0x48 },
//     { 0x7d, 0x48 },
//     { 0x7c, 0x08 },
//     { 0x7d, 0x20 },
//     { 0x7d, 0x10 },
//     { 0x7d, 0x0e },
//     { 0x90, 0x00 },
//     { 0x91, 0x0e },
//     { 0x91, 0x1a },
//     { 0x91, 0x31 },
//     { 0x91, 0x5a },
//     { 0x91, 0x69 },
//     { 0x91, 0x75 },
//     { 0x91, 0x7e },
//     { 0x91, 0x88 },
//     { 0x91, 0x8f },
//     { 0x91, 0x96 },
//     { 0x91, 0xa3 },
//     { 0x91, 0xaf },
//     { 0x91, 0xc4 },
//     { 0x91, 0xd7 },
//     { 0x91, 0xe8 },
//     { 0x91, 0x20 },
//     { 0x92, 0x00 },
//     { 0x93, 0x06 },
//     { 0x93, 0xe3 },
//     { 0x93, 0x05 },
//     { 0x93, 0x05 },
//     { 0x93, 0x00 },
//     { 0x93, 0x04 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x93, 0x00 },
//     { 0x96, 0x00 },
//     { 0x97, 0x08 },
//     { 0x97, 0x19 },
//     { 0x97, 0x02 },
//     { 0x97, 0x0c },
//     { 0x97, 0x24 },
//     { 0x97, 0x30 },
//     { 0x97, 0x28 },
//     { 0x97, 0x26 },
//     { 0x97, 0x02 },
//     { 0x97, 0x98 },
//     { 0x97, 0x80 },
//     { 0x97, 0x00 },
//     { 0x97, 0x00 },
//     { 0xc3, 0xed },
//     { 0xa4, 0x00 },
//     { 0xa8, 0x00 },
//     { 0xc5, 0x11 },
//     { 0xc6, 0x51 },
//     { 0xbf, 0x80 },
//     { 0xc7, 0x10 },
//     { 0xb6, 0x66 },
//     { 0xb8, 0xA5 },
//     { 0xb7, 0x64 },
//     { 0xb9, 0x7C },
//     { 0xb3, 0xaf },
//     { 0xb4, 0x97 },
//     { 0xb5, 0xFF },
//     { 0xb0, 0xC5 },
//     { 0xb1, 0x94 },
//     { 0xb2, 0x0f },
//     { 0xc4, 0x5c },
//     { 0xc0, 0x64 },
//     { 0xc1, 0x4B },
//     { 0x8c, 0x00 },
//     { 0x86, 0x3D },
//     { 0x50, 0x00 },
//     { 0x51, 0xC8 },
//     { 0x52, 0x96 },
//     { 0x53, 0x00 },
//     { 0x54, 0x00 },
//     { 0x55, 0x00 },
//     { 0x5a, 0xC8 },
//     { 0x5b, 0x96 },
//     { 0x5c, 0x00 },
//     { 0xd3, 0x04 },
//     { 0xc3, 0xed },
//     { 0x7f, 0x00 },
//     { 0xda, 0x09 },
//     { 0xe5, 0x1f },
//     { 0xe1, 0x67 },
//     { 0xe0, 0x00 },
//     { 0xdd, 0x7f },
//     { 0x05, 0x00 },
//     { 0xff, 0xff } };

// const unsigned char OV2640_RGB565_REG_TBL[][ 2 ] {
//     { 0xFF, 0x00 },
//     { 0xDA, 0x09 },
//     { 0xD7, 0x03 },
//     { 0xDF, 0x02 },
//     { 0x33, 0xa0 },
//     { 0x3C, 0x00 },
//     { 0xe1, 0x67 },
//     { 0xff, 0x01 },
//     { 0xe0, 0x00 },
//     { 0xe1, 0x00 },
//     { 0xe5, 0x00 },
//     { 0xd7, 0x00 },
//     { 0xda, 0x00 },
//     { 0xe0, 0x00 },
//     { 0xff, 0xff } };

// enum imageResolution imgRes = RES_320X240;
#define WIDTH  300
#define HEIGHT 240
static ov2640_handle_t ov2640_handle; /**< ov2640 handle */
uint16_t frameBuffers[ 2 ][ WIDTH * HEIGHT ] __attribute__( ( section( ".RAM_D2" ) ) ) __attribute__( ( aligned( 32 ) ) );
size_t frameLen { 0 };
uint8_t *curentFrameBuffer;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main( void )
{
    /* USER CODE BEGIN 1 */
    DRIVER_OV2640_LINK_INIT( &ov2640_handle, ov2640_handle_t );
    DRIVER_OV2640_LINK_SCCB_INIT( &ov2640_handle, ov2640_interface_sccb_init );
    DRIVER_OV2640_LINK_SCCB_DEINIT( &ov2640_handle, ov2640_interface_sccb_deinit );
    DRIVER_OV2640_LINK_SCCB_READ( &ov2640_handle, ov2640_interface_sccb_read );
    DRIVER_OV2640_LINK_SCCB_WRITE( &ov2640_handle, ov2640_interface_sccb_write );
    DRIVER_OV2640_LINK_POWER_DOWN_INIT( &ov2640_handle, ov2640_interface_power_down_init );
    DRIVER_OV2640_LINK_POWER_DOWN_DEINIT( &ov2640_handle, ov2640_interface_power_down_deinit );
    DRIVER_OV2640_LINK_POWER_DOWN_WRITE( &ov2640_handle, ov2640_interface_power_down_write );
    DRIVER_OV2640_LINK_RESET_INIT( &ov2640_handle, ov2640_interface_reset_init );
    DRIVER_OV2640_LINK_RESET_DEINIT( &ov2640_handle, ov2640_interface_reset_deinit );
    DRIVER_OV2640_LINK_RESET_WRITE( &ov2640_handle, ov2640_interface_reset_write );
    DRIVER_OV2640_LINK_DELAY_MS( &ov2640_handle, ov2640_interface_delay_ms );
    DRIVER_OV2640_LINK_DEBUG_PRINT( &ov2640_handle, ov2640_interface_debug_print );
    /* USER CODE END 1 */

    /* MPU Configuration--------------------------------------------------------*/

    MPU_Config();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SDMMC1_SD_Init();
    MX_FATFS_Init();
    MX_DCMI_Init();
    MX_I2C1_Init();
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN 2 */

    // ov2640 init
    ov2640_init( &ov2640_handle );

    // configure RGB565 mode
    ov2640_table_init( &ov2640_handle );
    ov2640_table_rgb565_init( &ov2640_handle );

    // configure resolution
    ov2640_set_resolution( &ov2640_handle, OV2640_RESOLUTION_UXGA );

    // configure image size
    // set output size
    ov2640_set_output_width( &ov2640_handle, WIDTH / 4 );
    ov2640_set_output_height( &ov2640_handle, HEIGHT / 4 );

    // set image size
    ov2640_set_image_horizontal( &ov2640_handle, 800 );
    ov2640_set_image_vertical( &ov2640_handle, 600 );

    // enable AGC and AEC
    // ov2640_set_agc_control( &ov2640_handle, OV2640_CONTROL_AUTO );
    // ov2640_set_exposure_control( &ov2640_handle, OV2640_CONTROL_AUTO );

    // SCCB_Write( 0xFF, 0x01 );
    // SCCB_Write( 0x12, 0x80 );
    // HAL_Delay( 50 );
    // OV2640_Configuration( OV2640_RGB565_INIT );
    // HAL_Delay( 10 );
    // OV2640_Configuration( OV2640_RGB565_REG_TBL );
    // HAL_Delay( 10 );
    // SCCB_Write( HSIZE8, HSIZE8_SET( 800 / 4 ) );
    // SCCB_Write( VSIZE8, VSIZE8_SET( 1200 / 4 ) );
    // SCCB_Write( HSIZE, HSIZE_SET( 1600 / 4 / 2 ) );
    // SCCB_Write( VSIZE, VSIZE_SET( 1200 / 4 / 2 ) );
    // SCCB_Write( XOFFL, XOFFL_SET( 0 ) );
    // SCCB_Write( YOFFL, YOFFL_SET( 0 ) );
    // SCCB_Write( VHYX, VHYX_HSIZE_SET( WIDTH / 4 ) | VHYX_VSIZE_SET( HEIGHT / 4 ) | VHYX_XOFF_SET( 0 ) | VHYX_YOFF_SET( 0 ) );
    // SCCB_Write( TEST, TEST_HSIZE_SET( WIDTH / 4 ) );
    // unsigned int outh;
    // unsigned int outw;
    // unsigned char temp;
    // outw = WIDTH / 4;
    // outh = HEIGHT / 4;
    // SCCB_Write( 0XFF, 0X00 );
    // SCCB_Write( 0XE0, 0X04 );
    // SCCB_Write( 0X5A, outw & 0XFF );
    // SCCB_Write( 0X5B, outh & 0XFF );
    // temp = ( outw >> 8 ) & 0X03;
    // temp |= ( outh >> 6 ) & 0X04;
    // SCCB_Write( 0X5C, temp );
    // SCCB_Write( 0XE0, 0X00 );

    // unsigned int hsize;
    // unsigned int vsize;
    // hsize = 800 / 4;
    // vsize = 600 / 4;
    // SCCB_Write( 0XFF, 0X00 );
    // SCCB_Write( 0XE0, 0X04 );
    // SCCB_Write( 0X51, hsize & 0XFF );
    // SCCB_Write( 0X52, vsize & 0XFF );
    // SCCB_Write( 0X53, 0 & 0XFF );
    // SCCB_Write( 0X54, 0 & 0XFF );
    // temp = ( vsize >> 1 ) & 0X80;
    // temp |= ( 0 >> 4 ) & 0X70;
    // temp |= ( hsize >> 5 ) & 0X08;
    // temp |= ( 0 >> 8 ) & 0X07;
    // SCCB_Write( 0X55, temp );
    // SCCB_Write( 0X57, ( hsize >> 2 ) & 0X80 );
    // SCCB_Write( 0XE0, 0X00 );

    // SCCB_Write( 0xFF, 0x01 );
    // SCCB_Write( 0x11, 0x04 ); // CLK = XVCLK / 5
    // SCCB_Write( 0XE0, 0X00 );
    // ov2640_set_clock_divider( &ov2640_handle, 0x04 );
    // ov2640_set_brightness( &ov2640_handle, OV2640_BRIGHTNESS_POSITIVE_1 );
    // ov2640_set_contrast( &ov2640_handle, OV2640_CONTRAST_POSITIVE_2 );
    // ov2640_set_color_saturation( &ov2640_handle, OV2640_COLOR_SATURATION_POSITIVE_2 );
    // ov2640_set_awb( &ov2640_handle, OV2640_BOOL_TRUE );
    // ov2640_set_awb_gain( &ov2640_handle, OV2640_BOOL_TRUE );
    // ov2640_set_band_filter( &ov2640_handle, OV2640_BOOL_FALSE );
    // OV2640_Brightness( Brightness2 );
    // HAL_Delay( 10 );

    // OV2640_Contrast( Contrast2 );
    // HAL_Delay( 10 );

    // OV2640_Saturation( Saturation2 );
    // HAL_Delay( 10 );

    // SCCB_Write( 0xFF, 0x00 ); // DSP bank

    // SCCB_Write( 0xC3, 0xFD ); // CTRL1: AWB = 0

    // SCCB_Write( 0xff, 0x01 );

    // // auto exp и AGC (COM8)
    // SCCB_Write( 0x13, 0xff ); // COM8: enable AGC and AEC

    // // min amplify AGC (COM9)
    // SCCB_Write( 0x14, 0xe0 ); // COM9: 128x max amp

    // // exp time (AEC)
    // SCCB_Write( 0x45, 0x3f );
    // SCCB_Write( 0x10, 0xff );

    // SCCB_Write( 0x13, 0xff ); // COM8: banding filter OFF

    // SCCB_Write( 0xff, 0x00 );
    // HAL_Delay( 100 );

    // HAL_DMA_RegisterCallback( &hdma_dcmi, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_CpltCallback );
    memset( &frameBuffers, 0, WIDTH * HEIGHT * 2 );
    HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_CONTINUOUS, ( uint32_t ) &frameBuffers, WIDTH * HEIGHT ); // DMA write by WORDs

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while ( 1 )
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Supply configuration update enable
     */
    HAL_PWREx_ConfigSupply( PWR_LDO_SUPPLY );

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE3 );

    while ( !__HAL_PWR_GET_FLAG( PWR_FLAG_VOSRDY ) )
    {
    }

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 6;
    RCC_OscInitStruct.PLL.PLLN       = 50;
    RCC_OscInitStruct.PLL.PLLP       = 2;
    RCC_OscInitStruct.PLL.PLLQ       = 2;
    RCC_OscInitStruct.PLL.PLLR       = 2;
    RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN   = 0;
    if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

    if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 ) != HAL_OK )
    {
        Error_Handler();
    }
    HAL_RCC_MCOConfig( RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1 );
}

/**
 * @brief DCMI Initialization Function
 * @param None
 * @retval None
 */
void MX_DCMI_Init( void )
{
    /* USER CODE BEGIN DCMI_Init 0 */

    /* USER CODE END DCMI_Init 0 */

    /* USER CODE BEGIN DCMI_Init 1 */

    /* USER CODE END DCMI_Init 1 */
    hdcmi.Instance              = DCMI;
    hdcmi.Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity       = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate      = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode         = DCMI_JPEG_DISABLE;
    hdcmi.Init.ByteSelectMode   = DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart  = DCMI_OEBS_ODD;
    hdcmi.Init.LineSelectMode   = DCMI_LSM_ALL;
    hdcmi.Init.LineSelectStart  = DCMI_OELS_ODD;
    if ( HAL_DCMI_Init( &hdcmi ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN DCMI_Init 2 */

    /* USER CODE END DCMI_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init( void )
{
    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance              = I2C1;
    hi2c1.Init.Timing           = 0x00C0EAFF;
    hi2c1.Init.OwnAddress1      = 0;
    hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2      = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if ( HAL_I2C_Init( &hi2c1 ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Configure Analogue filter
     */
    if ( HAL_I2CEx_ConfigAnalogFilter( &hi2c1, I2C_ANALOGFILTER_ENABLE ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Configure Digital filter
     */
    if ( HAL_I2CEx_ConfigDigitalFilter( &hi2c1, 0 ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief SDMMC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_SDMMC1_SD_Init( void )
{
    /* USER CODE BEGIN SDMMC1_Init 0 */

    /* USER CODE END SDMMC1_Init 0 */

    /* USER CODE BEGIN SDMMC1_Init 1 */

    /* USER CODE END SDMMC1_Init 1 */
    hsd1.Instance                 = SDMMC1;
    hsd1.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv            = 4;
    if ( HAL_SD_Init( &hsd1 ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SDMMC1_Init 2 */

    /* USER CODE END SDMMC1_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init( void )
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority( DMA1_Stream0_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( DMA1_Stream0_IRQn );
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET );

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( DCMI_PWDN_GPIO_Port, DCMI_PWDN_Pin, GPIO_PIN_RESET );

    /*Configure GPIO pin : LED_Pin */
    GPIO_InitStruct.Pin   = LED_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( LED_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : BUTTON_Pin */
    GPIO_InitStruct.Pin  = BUTTON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( BUTTON_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : DCMI_PWDN_Pin */
    GPIO_InitStruct.Pin   = DCMI_PWDN_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( DCMI_PWDN_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : DCMI_XCLK_Pin */
    GPIO_InitStruct.Pin       = DCMI_XCLK_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init( DCMI_XCLK_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : SDMMC1_SW_Pin */
    GPIO_InitStruct.Pin  = SDMMC1_SW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( SDMMC1_SW_GPIO_Port, &GPIO_InitStruct );

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority( SDMMC1_SW_EXTI_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( SDMMC1_SW_EXTI_IRQn );

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_DCMI_FrameEventCallback( DCMI_HandleTypeDef *hdcmi )
{
    frameLen          = WIDTH * HEIGHT * 2;
    curentFrameBuffer = reinterpret_cast<uint8_t *>( frameBuffers[ 1 ] );
    uint8_t spliter[] { "bgn" };
    auto d = CDC_Transmit_FS( spliter, 3 );
}

void HAL_DMA_CpltCallback( DMA_HandleTypeDef *hdcmi )
{
    frameLen          = WIDTH * HEIGHT * 2;
    curentFrameBuffer = reinterpret_cast<uint8_t *>( frameBuffers[ 0 ] );
    uint8_t spliter[] { "bgn" };
    CDC_Transmit_FS( spliter, 3 );
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
    if ( GPIO_Pin == GPIO_PIN_4 )
    {
        __HAL_GPIO_EXTI_CLEAR_FLAG( GPIO_PIN_0 );
        if ( HAL_GPIO_ReadPin( SDMMC1_SW_GPIO_Port, SDMMC1_SW_Pin ) )
        {
            HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_SET );
        }
        else
            HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_RESET );
    }
    else
    {
        __NOP();
    }
}

void vprint( const char *fmt, va_list argp )
{
    char string[ 200 ];
    if ( 0 < vsprintf( string, fmt, argp ) )
    {
        CDC_Transmit_FS( ( uint8_t * ) string, strlen( string ) );
    }
}

void my_printf( const char *fmt, ... )
{
    return;
    va_list argp;
    va_start( argp, fmt );
    vprint( fmt, argp );
    va_end( argp );
}

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config( void )
{
    MPU_Region_InitTypeDef MPU_InitStruct = { 0 };

    /* Disables the MPU */
    HAL_MPU_Disable();

    /** Initializes and configures the Region and the memory to be protected
     */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress      = 0x0;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion( &MPU_InitStruct );
    /* Enables the MPU */
    HAL_MPU_Enable( MPU_PRIVILEGED_DEFAULT );
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler( void )
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while ( 1 )
    {
        HAL_GPIO_TogglePin( GPIOE, GPIO_PIN_3 );
        HAL_Delay( 1000 );
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed( uint8_t *file, uint32_t line )
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       //ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
