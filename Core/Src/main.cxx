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
#include "ov2640.h"
#include "ov2640_interface.h"
}
#include "ov2640_basic.h"
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

auto width { WIDTH };
auto height { HEIGHT };
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
uint16_t frameBuffers[ 1 ][ WIDTH * HEIGHT ] __attribute__( ( section( ".RAM_D2" ) ) ) __attribute__( ( aligned( 32 ) ) );
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
    HAL_Delay( 200 );
    // auto d = 1;
    // HAL_DCMI_Stop( &hdcmi );

    // OV2640_Init( &hi2c1, &hdcmi );
    ov2640_basic_init();
    // sccb_config( OV2640_INIT, sizeof( OV2640_INIT ) / sizeof( OV2640_INIT[ 0 ] ) );
    // HAL_Delay( 300 );
    // sccb_config( OV2640_FORMAT_CIF, sizeof( OV2640_FORMAT_CIF ) / sizeof( OV2640_FORMAT_CIF[ 0 ] ) );
    // HAL_Delay( 300 );
    // sccb_config( OV2640_FORMAT_RGB565, sizeof( OV2640_FORMAT_RGB565 ) / sizeof( OV2640_FORMAT_RGB565[ 0 ] ) );
    // HAL_Delay( 300 );
    // sccb_config( OV2640_320x240, sizeof( OV2640_320x240 ) / sizeof( OV2640_320x240[ 0 ] ) );
    // ov2640_basic_set_rgb565_mode();
    // ov2640_basic_set_image_resolution( OV2640_IMAGE_RESOLUTION_QVGA );
    // ov2640_set_output_width( &gs_handle, WIDTH / 4 );
    // ov2640_set_output_height( &gs_handle, HEIGHT / 4 );
    ov2640_interface_delay_ms( 500 );

    // ov2640 init
    // OV2640_Init( &hi2c1, &hdcmi );

    // // configure RGB565 mode
    // ov2640_table_init( &ov2640_handle );
    // ov2640_table_rgb565_init( &ov2640_handle );

    // // configure resolution
    // ov2640_set_resolution( &ov2640_handle, OV2640_RESOLUTION_UXGA );

    // // configure image size
    // // set output size
    // ov2640_set_output_width( &ov2640_handle, WIDTH / 4 );
    // ov2640_set_output_height( &ov2640_handle, HEIGHT / 4 );

    // // set image size
    // ov2640_set_image_horizontal( &ov2640_handle, 800 );
    // ov2640_set_image_vertical( &ov2640_handle, 600 );

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
    // // SCCB_Write( HSIZE8, HSIZE8_SET( 800 / 4 ) );
    // // SCCB_Write( VSIZE8, VSIZE8_SET( 1200 / 4 ) );
    // // SCCB_Write( HSIZE, HSIZE_SET( 1600 / 4 / 2 ) );
    // // SCCB_Write( VSIZE, VSIZE_SET( 1200 / 4 / 2 ) );
    // // SCCB_Write( XOFFL, XOFFL_SET( 0 ) );
    // // SCCB_Write( YOFFL, YOFFL_SET( 0 ) );
    // // SCCB_Write( VHYX, VHYX_HSIZE_SET( WIDTH / 4 ) | VHYX_VSIZE_SET( HEIGHT / 4 ) | VHYX_XOFF_SET( 0 ) | VHYX_YOFF_SET( 0 ) );
    // // SCCB_Write( TEST, TEST_HSIZE_SET( WIDTH / 4 ) );
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
    uint8_t reg_value;

    // HAL_DMA_RegisterCallback( &hdma_dcmi, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_CpltCallback );
    memset( &frameBuffers, 0, width * height );
    HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_CONTINUOUS, ( uint32_t ) &frameBuffers, width * height / 2 );

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while ( 1 )
    {
        // uint8_t ret;
        // uint16_t value = 0;

        // my_printf( "========== OV2640 Resolution Characteristics ==========\n" );
        // my_printf( "\n--- Window Settings ---\n" );
        // my_printf( "RESOLUTION_MODE: " );
        // ret = ov2640_get_resolution( &gs_handle, reinterpret_cast<ov2640_resolution_t *>( &value ) );
        // if ( ret == 0 )
        // {
        //     switch ( value )
        //     {
        //         case 0:
        //             my_printf( "UXGA (1600x1200)\n" );
        //             break;
        //         case 1:
        //             my_printf( "CIF (400x296)\n" );
        //             break;
        //         case 4:
        //             my_printf( "SVGA (800x600)\n" );
        //             break;
        //         default:
        //             my_printf( "Unknown (0x%02X)\n", value );
        //             break;
        //     }
        // }
        // else
        // {
        //     my_printf( "Failed to read\n" );
        // }
        // // H_WINDOW_START
        // ret = ov2640_get_horizontal_window_start( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "H_WINDOW_START: %u (0x%04X)\n", value, value );
        // }
        // else
        // {
        //     my_printf( "H_WINDOW_START: Failed to read (error: %d)\n", ret );
        // }

        // // H_WINDOW_END
        // ret = ov2640_get_horizontal_window_end( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "H_WINDOW_END:   %u (0x%04X)\n", value, value );
        // }
        // else
        // {
        //     my_printf( "H_WINDOW_END:   Failed to read (error: %d)\n", ret );
        // }

        // // V_WINDOW_START
        // ret = ov2640_get_vertical_window_line_start( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "V_WINDOW_START: %u (0x%04X)\n", value, value );
        // }
        // else
        // {
        //     my_printf( "V_WINDOW_START: Failed to read (error: %d)\n", ret );
        // }

        // // V_WINDOW_END
        // ret = ov2640_get_vertical_window_line_end( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "V_WINDOW_END:   %u (0x%04X)\n", value, value );
        // }
        // else
        // {
        //     my_printf( "V_WINDOW_END:   Failed to read (error: %d)\n", ret );
        // }

        // my_printf( "\n--- DSP Image Size ---\n" );

        // // DSP_IMAGE_HORIZONTAL
        // ret = ov2640_get_image_horizontal( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_IMAGE_HORIZONTAL: %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_IMAGE_HORIZONTAL: Failed to read (error: %d)\n", ret );
        // }

        // // DSP_IMAGE_VERTICAL
        // ret = ov2640_get_image_vertical( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_IMAGE_VERTICAL:   %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_IMAGE_VERTICAL:   Failed to read (error: %d)\n", ret );
        // }

        // my_printf( "\n--- DSP Size (with offset) ---\n" );

        // // DSP_HORIZONTAL_SIZE
        // ret = ov2640_get_horizontal_size( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_HORIZONTAL_SIZE: %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_HORIZONTAL_SIZE: Failed to read (error: %d)\n", ret );
        // }

        // // DSP_VERTICAL_SIZE
        // ret = ov2640_get_vertical_size( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_VERTICAL_SIZE:   %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_VERTICAL_SIZE:   Failed to read (error: %d)\n", ret );
        // }

        // my_printf( "\n--- DSP Output Size (Zoom/Scale) ---\n" );

        // // DSP_OUTPUT_WIDTH
        // ret = ov2640_get_output_width( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_OUTPUT_WIDTH:  %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_OUTPUT_WIDTH:  Failed to read (error: %d)\n", ret );
        // }

        // // DSP_OUTPUT_HEIGHT
        // ret = ov2640_get_output_height( &gs_handle, &value );
        // if ( ret == 0 )
        // {
        //     my_printf( "DSP_OUTPUT_HEIGHT: %u (0x%04X) pixels\n", value, value );
        // }
        // else
        // {
        //     my_printf( "DSP_OUTPUT_HEIGHT: Failed to read (error: %d)\n", ret );
        // }
        // my_printf( "\n========== OV2640 Register Dump ==========\n\n" );

        // // Bank 0 - DSP Registers (0xFF = 0x00)
        // my_printf( "--- DSP Bank (0xFF=0x00) ---\n" );
        // SCCB_Write( 0xFF, 0x00 ); // Select DSP bank

        // uint8_t dsp_regs[] = { 0x05, 0x44, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        //                        0x5A, 0x5B, 0x5C, 0x7C, 0x7D, 0x86, 0x87, 0x8C, 0xC0, 0xC1,
        //                        0xC2, 0xC3, 0xD3, 0xDA, 0xE0, 0xF0, 0xF7, 0xF8, 0xF9, 0xFA,
        //                        0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

        // for ( int i = 0; i < sizeof( dsp_regs ); i++ )
        // {
        //     SCCB_Read( dsp_regs[ i ], &reg_value );
        //     my_printf( "REG[0x%02X] = 0x%02X\n", dsp_regs[ i ], reg_value );
        // }

        // my_printf( "\n--- Sensor Bank (0xFF=0x01) ---\n" );
        // SCCB_Write( 0xFF, 0x01 ); // Select Sensor bank

        // uint8_t sensor_regs[] = { 0x00, 0x03, 0x04, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x10,
        //                           0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18, 0x19, 0x1A, 0x1C,
        //                           0x1D, 0x24, 0x25, 0x26, 0x2A, 0x2B, 0x2D, 0x2E, 0x2F, 0x30,
        //                           0x31, 0x32, 0x34, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4B, 0x4E,
        //                           0x4F, 0x50, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62 };

        // for ( int i = 0; i < sizeof( sensor_regs ); i++ )
        // {
        //     SCCB_Read( sensor_regs[ i ], &reg_value );
        //     my_printf( "REG[0x%02X] = 0x%02X\n", sensor_regs[ i ], reg_value );
        // }

        // my_printf( "\n========== End of Register Dump ==========\n" );
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
    frameLen          = width * height * 2;
    curentFrameBuffer = reinterpret_cast<uint8_t *>( frameBuffers[ 0 ] );
    uint8_t spliter[] { "bgn" };
    auto d = CDC_Transmit_FS( spliter, 3 );
}

void HAL_DMA_CpltCallback( DMA_HandleTypeDef *hdcmi )
{
    frameLen          = width * height * 2;
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
    va_list argp;
    va_start( argp, fmt );
    vprint( fmt, argp );
    va_end( argp );
    HAL_Delay( 50 );
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
