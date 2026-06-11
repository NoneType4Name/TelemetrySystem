/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @FatFsFile           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE FatFsFile
 * in the root directory of this software component.
 * If no LICENSE FatFsFile comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "stm32_hal_legacy.h"
#include "stm32h7xx_hal_rtc.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_ov2640.h"
#include <array>
#include <bitset>
#include <cstdlib>
#include <stdint.h>
#include "ov2640_basic.h"
#include "stm32h750xx.h"
#include "stm32h7xx_hal_gpio.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <vector>
// #include "TestImage.h"
extern "C"
{
#include "ESP8266.h"
#include "espConfig.h"
}

// #include <TestImage.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RGB565_R( p ) ( ( ( p ) >> 11 ) & 0x1F )
#define RGB565_G( p ) ( ( ( p ) >> 5 ) & 0x3F )
#define RGB565_B( p ) ( ( p ) & 0x1F )
#define GET_X( o )    o % WIDTH
#define GET_Y( o )    o / WIDTH
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint16_t frameBuffers[ 1 ][ WIDTH * HEIGHT + 8 / sizeof( uint16_t ) + 2 + 1 ] __attribute__( ( section( ".RAM_D2" ) ) ) __attribute__( ( aligned( 32 ) ) );
extern uint8_t UserRxBufferFS[ APP_RX_DATA_SIZE ];
extern uint8_t UserTxBufferFS[ APP_TX_DATA_SIZE ];
extern USBD_HandleTypeDef hUsbDeviceFS;
size_t frameLen { 0 };
uint8_t *curentFrameBuffer;
bool CDC_RxStatus { 0 };
bool CameraCountDownEnded { 1 };
bool sdCardPresented { 0 };
bool sdCardMounted { 0 };
bool newFrame { 0 };
bool usbConnected { 0 };
bool newConfigProcessed { 1 };
bool timeToCalibrateRTC { 0 };
bool nightMode { 0 };
bool debugCameraPattern { 0 };
uint8_t avg;
uint16_t aec;
uint16_t offsetWithZoom[ 2 ] { 0, 0 };
FATFS FatFs;
FIL FatFsFile;
extern char ESP_RX_buff[ ESP_RX_buff_size ];
std::bitset<WIDTH * HEIGHT> pixelVisited { 0 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( void );
static void MPU_Config( void );
static void MX_GPIO_Init( void );
static void MX_DMA_Init( void );
static void MX_I2C1_Init( void );
static void MX_USART3_UART_Init( void );
static void MX_TIM3_Init( void );
static void MX_TIM7_Init( void );
static void MX_RTC_Init( void );
/* USER CODE BEGIN PFP */

void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef *hrtc )
{
    // every day at 00:00
    timeToCalibrateRTC = true;
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
    if ( htim->Instance == TIM7 )
    {
        HAL_TIM_Base_Stop_IT( htim );
        CameraCountDownEnded = true;
    }
    else if ( htim->Instance == TIM3 )
    {
        HAL_TIM_Base_Stop_IT( htim );
        HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_RESET );
    }
}

void HAL_DCMI_FrameEventCallback( DCMI_HandleTypeDef *hdcmi )
{
    newFrame = true;
    if ( newConfigProcessed )
        return;
    ov2640_set_offset_x( &gs_handle, offsetWithZoom[ 0 ] );
    ov2640_set_offset_y( &gs_handle, offsetWithZoom[ 1 ] & 0xFFF );
    if ( aec == 0 )
    {
        ov2640_set_exposure_control( &gs_handle, OV2640_CONTROL_AUTO );
    }
    else
    {
        ov2640_set_exposure_control( &gs_handle, OV2640_CONTROL_MANUAL );
        ov2640_set_aec( &gs_handle, aec );
    }
    newConfigProcessed = true;
    // auto d = CDC_Transmit_FS( curentFrameBuffer, frameLen );
}

// void HAL_DMA_CpltCallback( DMA_HandleTypeDef *hdcmi )
// {
//     frameLen          = WIDTH * HEIGHT * 2;
//     curentFrameBuffer = reinterpret_cast<uint8_t *>( frameBuffers[ 0 ] );
//     uint8_t spliter[] { "bgn" };
//     CDC_Transmit_FS( spliter, 3 );
// }

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
    if ( GPIO_Pin == SDMMC1_SW_Pin )
    {
        if ( HAL_GPIO_ReadPin( SDMMC1_SW_GPIO_Port, SDMMC1_SW_Pin ) )
        {
            sdCardPresented = 1;
            HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_RESET );
        }
        else
        {
            sdCardPresented = 0;
            sdCardMounted   = 0;
            f_mount( 0, SDPath, 1 );
            MX_FATFS_DeInit();
            if ( HAL_SD_DeInit( &hsd1 ) == HAL_OK )
                HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_SET );
        }
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

void CDC_TX_FRAME()
{
    if ( hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED )
        return;
    frameLen          = WIDTH * HEIGHT * 2 + 14;
    curentFrameBuffer = reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ] );
    auto p { curentFrameBuffer };
    p[ 0 ]                                       = 'b';
    p[ 1 ]                                       = 'g';
    p[ 2 ]                                       = 'n';
    p[ 3 ]                                       = 'n';
    ( *reinterpret_cast<uint16_t *>( &p[ 4 ] ) ) = offsetWithZoom[ 0 ];
    ( *reinterpret_cast<uint16_t *>( &p[ 6 ] ) ) = offsetWithZoom[ 1 ];

    p = reinterpret_cast<uint8_t *>( &curentFrameBuffer[ WIDTH * HEIGHT * 2 + 8 ] );
    ov2640_get_luminance_average( &gs_handle, &avg );
    uint16_t Taec;
    ov2640_get_aec( &gs_handle, &Taec );
    *reinterpret_cast<uint16_t *>( &p[ 0 ] ) = Taec;
    if ( avg > 10 && aec > 500 && !nightMode )
    {
        ov2640_set_exposure_control( &gs_handle, OV2640_CONTROL_MANUAL );
        ov2640_set_aec( &gs_handle, 200 );
        nightMode = true;
    }
    else if ( nightMode && avg > 4 )
    {
        ov2640_set_exposure_control( &gs_handle, OV2640_CONTROL_AUTO );
        nightMode = false;
    }
    // ( *reinterpret_cast<uint16_t *>( &p[ 0 ] ) ) = aec;
    p[ 2 ] = avg;
    p[ 3 ] = 'e';
    p[ 4 ] = 'n';
    p[ 5 ] = 'd';
    CDC_Transmit_FS( curentFrameBuffer, 1u << 12u );
}

// bool inline isRed( uint16_t pixel ) // todo: wrong
// {
//     uint8_t r = RGB565_R( pixel );
//     uint8_t g = RGB565_G( pixel );
//     uint8_t b = RGB565_B( pixel );

//     return true && ( r > ( 80 * 31 / 255 ) ) && ( r > ( g >> 1 ) ) && ( ( int ) r - ( g >> 1 ) > ( 40 * 31 / 255 ) ) && ( r > b ) && ( ( int ) r - b > ( 40 * 31 / 255 ) ); // r >> g;b (d = 50/255) r > 100/255
// }

bool inline isYellow( uint16_t pixel )
{
    uint8_t r = RGB565_R( pixel );
    uint8_t g = RGB565_G( pixel ) >> 1;
    uint8_t b = RGB565_B( pixel );
    return ( abs( r - g ) < 6 && b < g && g - b > 3 );
}

bool inline isLight( uint16_t pixel )
{
    uint8_t r = RGB565_R( pixel );
    uint8_t g = RGB565_G( pixel ) >> 1;
    uint8_t b = RGB565_B( pixel );
    // return ( b > ( 0 * 31 / 255 ) ) && ( b + ( 30 * 31 / 255 ) < r ) && ( abs( r - ( g >> 1 ) ) < ( 25 * 31 / 255 ) ); // b > 10, r > b + 50, r ~ g (d<25)
    uint8_t avg = ( r + b + g ) / 3;
    return avg > 6 && pixel != 0xf800; // (r+g+b) / 3 > 20 (rgb565)
}

bool isLightBlue( uint16_t pixel )
{
    uint8_t r = RGB565_R( pixel );
    uint8_t g = RGB565_G( pixel );
    uint8_t b = RGB565_B( pixel );
    return true && ( b > ( 80 * 31 / 255 ) ) && ( b > ( g >> 1 ) ) && ( ( int ) b - ( g >> 1 ) < ( 58 * 31 / 255 ) ) && ( ( int ) b - ( g >> 1 ) > 0 ) && ( ( g >> 1 ) > r ) && ( ( int ) ( g >> 1 ) - r > 0 ) && ( ( int ) ( g >> 1 ) - r < ( 66 * 31 / 255 ) ); // b > g >> r (80+, (80+)-58>0, (80+)-58-66>0)
}

bool inline isGrey( uint16_t pixel )
{
    uint8_t r = RGB565_R( pixel );
    uint8_t g = RGB565_G( pixel );
    uint8_t b = RGB565_B( pixel );
    return true && ( abs( ( int ) b - ( g >> 1 ) ) < ( 25 * 31 / 255 ) ) && ( abs( ( ( int ) g >> 1 ) - r ) < ( 25 * 31 / 255 ) ) && ( abs( ( int ) b - r ) < ( 25 * 31 / 255 ) ) && b > ( 150 * 31 / 255 ) && b < ( 210 * 31 / 255 ); // r -- g -- b < 10 && b in (150; 210)
}

std::array<uint16_t, 3> inline fillColorPattern( uint16_t leftUpPixelInd, bool nightMode ) // [left up corner, right down corner, count]
{
    std::vector<uint16_t> stack { leftUpPixelInd };
    // frameBuffers[ 0 ][ leftUpPixelInd ] = 0x07e0;

    uint16_t filled { 1 };
    uint8_t maxX = GET_X( leftUpPixelInd );
    uint8_t maxY = GET_Y( leftUpPixelInd );
    uint8_t minX = maxX;
    uint8_t minY = maxY;

    pixelVisited.set( leftUpPixelInd - 4 );

    while ( !stack.empty() )
    {
        uint16_t idx = stack.back();
        stack.pop_back();

        int16_t x = GET_X( idx );
        int16_t y = GET_Y( idx );

        for ( int8_t dy = -2; dy <= 2; dy++ )
        {
            for ( int8_t dx = -2; dx <= 2; dx++ )
            {
                if ( dx == 0 && dy == 0 ) continue;

                int16_t nx = x + dx;
                int16_t ny = y + dy;
                if ( nx < 4 || ny < 0 ) continue;

                if ( nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT )
                {
                    uint16_t nidx = ny * WIDTH + nx;
                    if ( !pixelVisited.test( nidx - 4 ) && ( nightMode ? ( isYellow( frameBuffers[ 0 ][ nidx ] ) || isLight( frameBuffers[ 0 ][ nidx ] ) ) : isLightBlue( frameBuffers[ 0 ][ nidx ] ) ) )
                    {
                        pixelVisited.set( nidx - 4 );
                        // frameBuffers[ 0 ][ nidx ] = 0x07e0;
                        ++filled;
                        uint8_t px = GET_X( nidx );
                        uint8_t py = GET_Y( nidx );
                        if ( px > maxX ) maxX = px;
                        if ( py > maxY ) maxY = py;
                        if ( px < minX ) minX = px;
                        if ( py < minY )
                            minY = py;
                        stack.emplace_back( nidx );
                    }
                }
            }
        }
    }

    return { static_cast<uint16_t>( minX + minY * WIDTH ), static_cast<uint16_t>( maxX + maxY * WIDTH ), filled };
}

bool inline dayTestForBus()
{
    pixelVisited.reset();
    uint8_t countLightBluePattern { 0 };
    for ( uint16_t h { 0 }; h < HEIGHT; ++h )
        for ( uint16_t w { 0 }; w < WIDTH; ++w )
        {
            uint16_t leftP = 4 + w + h * WIDTH;
            if ( true && !pixelVisited.test( w + h * WIDTH ) && isLightBlue( frameBuffers[ 0 ][ leftP ] ) )
            {
                auto rightP     = fillColorPattern( leftP, 0 );
                leftP           = rightP[ 0 ];
                uint8_t dw      = GET_X( rightP[ 1 ] ) - GET_X( leftP );
                uint8_t dh      = GET_Y( rightP[ 1 ] ) - GET_Y( leftP );
                uint16_t square = dw * dh;
                if ( square > 1000 )
                {
                    if ( dh > 12 )
                    {
                        float d = ( ( float ) ( dw ) / dh );
                        if ( ( square > 3000 && ( d > 2.f && d < 3.f ) ) || ( d > 4.f && d < 6.5f ) ) // (square > 3000 -> ratio in range 2-3 - bus | 1000 < square < 3000 -> ratio ~ 4-6.5f - roof of bus) #experemental
                        {
                            if ( ++countLightBluePattern > 1 )
                                debugCameraPattern = true;
                            for ( size_t x = ( GET_X( leftP ) > 0 ? GET_X( leftP ) - 1 : 0 ); x <= ( GET_X( rightP[ 1 ] ) + 1 < WIDTH + 4 ? GET_X( rightP[ 1 ] ) + 1 : WIDTH + 3 ); ++x )
                            {
                                if ( GET_Y( leftP ) > 0 )
                                    frameBuffers[ 0 ][ 4 + ( GET_Y( leftP ) - 1 ) * WIDTH + x ] = 0xf800;
                                if ( GET_Y( rightP[ 1 ] ) + 1 < HEIGHT )
                                    frameBuffers[ 0 ][ 4 + ( GET_Y( rightP[ 1 ] ) + 1 ) * WIDTH + x ] = 0xf800;
                            }

                            for ( size_t y = ( GET_Y( leftP ) > 0 ? GET_Y( leftP ) : 0 ); y <= ( GET_Y( rightP[ 1 ] ) + 1 < HEIGHT ? GET_Y( rightP[ 1 ] ) + 1 : HEIGHT - 1 ); ++y )
                            {
                                if ( GET_X( leftP ) > 0 )
                                    frameBuffers[ 0 ][ 4 + y * WIDTH + ( GET_X( leftP ) - 1 ) ] = 0xf800;
                                if ( GET_X( rightP[ 1 ] ) + 1 < WIDTH + 4 )
                                    frameBuffers[ 0 ][ 4 + y * WIDTH + ( GET_X( rightP[ 1 ] ) + 1 ) ] = 0xf800;
                            }
                        }
                        else
                            debugCameraPattern = true;
                    }
                }
            }
        }
    return countLightBluePattern > 0;
}

bool nightTestForBus() // by lights pattern
{
    pixelVisited.reset();
    uint8_t countLightsPattern { 0 };
    for ( uint16_t h { 0 }; h < HEIGHT; ++h )
    {
        for ( uint16_t w { 0 }; w < WIDTH; ++w )
        {
            uint16_t leftP = 4 + w + h * WIDTH;
            if ( !pixelVisited.test( w + h * WIDTH ) && ( isYellow( frameBuffers[ 0 ][ leftP ] ) || isLight( frameBuffers[ 0 ][ leftP ] ) ) )
            {
                auto rightP     = fillColorPattern( leftP, 1 );
                leftP           = rightP[ 0 ];
                uint8_t dw      = GET_X( rightP[ 1 ] ) - GET_X( leftP );
                uint8_t dh      = GET_Y( rightP[ 1 ] ) - GET_Y( leftP );
                uint16_t square = dw * dh;
                if ( square > 80 )
                {
                    if ( square > 200 ) debugCameraPattern = true;
                    if ( dh > 1 && dh < 9 )
                    {
                        float d = ( ( float ) ( dw ) / dh );
                        if ( d > 5.f )
                        {
                            if ( ++countLightsPattern > 1 )
                                debugCameraPattern = true;
                            for ( size_t x = ( GET_X( leftP ) > 0 ? GET_X( leftP ) - 1 : 0 ); x <= ( GET_X( rightP[ 1 ] ) + 1 < WIDTH ? GET_X( rightP[ 1 ] ) + 1 : WIDTH - 1 ); ++x )
                            {
                                if ( GET_Y( leftP ) > 0 )
                                    frameBuffers[ 0 ][ ( GET_Y( leftP ) - 1 ) * WIDTH + x ] = 0xf800;
                                if ( GET_Y( rightP[ 1 ] ) + 1 < HEIGHT )
                                    frameBuffers[ 0 ][ ( GET_Y( rightP[ 1 ] ) + 1 ) * WIDTH + x ] = 0xf800;
                            }

                            for ( size_t y = ( GET_Y( leftP ) > 0 ? GET_Y( leftP ) : 0 ); y <= ( GET_Y( rightP[ 1 ] ) + 1 < HEIGHT ? GET_Y( rightP[ 1 ] ) + 1 : HEIGHT - 1 ); ++y )
                            {
                                if ( GET_X( leftP ) > 0 )
                                    frameBuffers[ 0 ][ y * WIDTH + ( GET_X( leftP ) - 1 ) ] = 0xf800;
                                if ( GET_X( rightP[ 1 ] ) + 1 < WIDTH )
                                    frameBuffers[ 0 ][ y * WIDTH + ( GET_X( rightP[ 1 ] ) + 1 ) ] = 0xf800;
                            }
                        }
                    }
                }
            }
        }
    }
    return countLightsPattern > 0;
}

bool inline testForBus()
{
    if ( nightMode )
        return nightTestForBus();
    return dayTestForBus();
}

// bool inline getZoomed()
// {
//     return offsetWithZoom[ 1 ] & 1 << 15;
// }

// void inline setZoomed()
// {
//     offsetWithZoom[ 1 ] |= 1 << 15;
// }

bool inline getToggle()
{
    return offsetWithZoom[ 1 ] & 1 << 14;
}

void inline ReSetToggle()
{
    offsetWithZoom[ 1 ] ^= ( 1 << 14 );
}

void SaveImageBMP( const char *filename, const uint8_t *buffer, UINT len )
{
    if ( !sdCardMounted )
        return;
    FRESULT result;
    UINT bytes_written;
    uint32_t row_size  = ( ( 16 * WIDTH + 31 ) / 32 ) * 4;
    uint32_t data_size = row_size * HEIGHT;

#pragma pack( push, 1 )

    struct
    {
        uint16_t file_type; // "BM" = 0x4D42
        uint32_t file_size; // Size of the entire FatFsFile
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t offset_data; // Headers (54) + 3 masks (12)
    } BMPFileHeader;

    struct
    {
        uint32_t size; // Size of this header (40)
        int32_t width;
        int32_t height;
        uint16_t planes;      // Must be 1
        uint16_t bit_count;   // 16 for RGB565
        uint32_t compression; // 3 = BI_BITFIELDS
        uint32_t size_image;
        int32_t x_pixels_per_meter;
        int32_t y_pixels_per_meter;
        uint32_t colors_used;
        uint32_t colors_important;
    } BMPInfoHeader;

#pragma pack( pop )
    BMPFileHeader = {
        .file_type   = 0x4D42,
        .file_size   = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader ) + 12 + data_size,
        .reserved1   = 0,
        .reserved2   = 0,
        .offset_data = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader ) + 12 };

    BMPInfoHeader = {
        .size               = sizeof( BMPInfoHeader ),
        .width              = WIDTH,
        .height             = HEIGHT,
        .planes             = 1,
        .bit_count          = 16,
        .compression        = 3,
        .size_image         = data_size,
        .x_pixels_per_meter = 0,
        .y_pixels_per_meter = 0,
        .colors_used        = 0,
        .colors_important   = 0 };

    result = f_open( &FatFsFile, filename, FA_WRITE | FA_CREATE_ALWAYS );
    if ( result != FR_OK )
    {
        return;
    }

    // Запись File Header
    result = f_write( &FatFsFile, &BMPFileHeader, sizeof( BMPFileHeader ), &bytes_written );
    if ( result != FR_OK || bytes_written != sizeof( BMPFileHeader ) )
    {
        f_close( &FatFsFile );
        return;
    }

    // Запись Info Header
    result = f_write( &FatFsFile, &BMPInfoHeader, sizeof( BMPInfoHeader ), &bytes_written );
    if ( result != FR_OK || bytes_written != sizeof( BMPInfoHeader ) )
    {
        f_close( &FatFsFile );
        return;
    }
    uint32_t rgb565_masks[ 3 ] = {
        0xF800, // Red mask (5 bits)
        0x07E0, // Green mask (6 bits)
        0x001F  // Blue mask (5 bits)
    };
    result = f_write( &FatFsFile, rgb565_masks, sizeof( rgb565_masks ), &bytes_written );
    if ( result != FR_OK || bytes_written != sizeof( rgb565_masks ) )
    {
        f_close( &FatFsFile );
        return;
    }

    uint32_t padding_size = row_size - ( WIDTH * 2 );
    uint8_t padding[ 4 ]  = { 0, 0, 0, 0 };

    for ( int y { HEIGHT - 1 }; y >= 0; y-- )
    {
        result = f_write( &FatFsFile, &frameBuffers[ 0 ][ 4 + y * WIDTH ], WIDTH * 2, &bytes_written );
        if ( result != FR_OK || bytes_written != ( UINT ) ( WIDTH * 2 ) )
        {
            f_close( &FatFsFile );
            return;
        }

        if ( padding_size > 0 )
        {
            result = f_write( &FatFsFile, padding, padding_size, &bytes_written );
            if ( result != FR_OK || bytes_written != ( UINT ) padding_size )
            {
                f_close( &FatFsFile );
                return;
            }
        }
    }
    f_sync( &FatFsFile );
    f_close( &FatFsFile );
}

// zero, if error
uint32_t IncrementLastPhotoNumber()
{
    if ( !sdCardMounted )
        return 0;
    FRESULT result;
    UINT bytes_read;
    uint32_t temp_value;

    result = f_open( &FatFsFile, "last", FA_READ | FA_WRITE );
    if ( result != FR_OK )
        return 0;

    result = f_read( &FatFsFile, &temp_value, sizeof( uint32_t ), &bytes_read );
    f_lseek( &FatFsFile, 0 );
    f_write( &FatFsFile, &++temp_value, sizeof( uint32_t ), &bytes_read );
    f_sync( &FatFsFile );
    f_close( &FatFsFile );

    if ( result == FR_OK && bytes_read == sizeof( uint32_t ) )
    {
        return temp_value;
    }

    return 0;
}

void StartCountdown()
{
    HAL_TIM_Base_Start_IT( &htim7 );
}

void enableLed2ms()
{
    HAL_TIM_Base_Start_IT( &htim3 );
    HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_SET );
}

void updateTime()
{
    if ( ESP8266_SendRequest( "SSL", "smartapp-code.sberdevices.ru", 443, "GET /tools/api/now?tz=Europe/Moscow&format=dd,MM,yy,HH,mm,ss,u HTTP/1.1\r\n"
                                                                          "Host: smartapp-code.sberdevices.ru\r\n"
                                                                          "User-Agent: ESP8266\r\n"
                                                                          "Accept: application/json\r\n"
                                                                          "Connection: close\r\n" ) )
    {
        auto d = strstr( strstr( ESP8266_GetResponse( 500 ), "\r\n\r\n" ) + 4, "\r\n" ) + 2;
        if ( d )
        {
            uint8_t day, month, year, hour, minute, second, dayOfWeek;

            if ( sscanf( d, "{\"timezone\":\"Europe/Moscow\",\"formatted\":\"%d,%d,%d,%d,%d,%d,%d\"", ( int * ) &day, ( int * ) &month, ( int * ) &year, ( int * ) &hour, ( int * ) &minute, ( int * ) &second, ( int * ) &dayOfWeek ) == 7 )
            {
                RTC_TimeTypeDef cTime { hour, minute, second };
                RTC_DateTypeDef cData { dayOfWeek, month, day, year };
                HAL_RTC_SetTime( &hrtc, &cTime, FORMAT_BIN );
                HAL_RTC_SetDate( &hrtc, &cData, FORMAT_BIN );
            }
        }
    }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
    MX_USART3_UART_Init();
    MX_TIM3_Init();
    MX_TIM7_Init();
    MX_RTC_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay( 400 );
    if ( HAL_GPIO_ReadPin( SDMMC1_SW_GPIO_Port, SDMMC1_SW_Pin ) )
    {
        sdCardPresented = 1;
        sdCardMounted   = f_mount( &FatFs, SDPath, 1 ) == FR_OK;
    }
    // init camera
    ov2640_basic_init();

    ov2640_set_awb( &gs_handle, OV2640_BOOL_TRUE );
    ov2640_set_awb_gain( &gs_handle, OV2640_BOOL_TRUE );

    // HAL_DMA_RegisterCallback( &hdma_dcmi, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_CpltCallback );
    memset( &frameBuffers, 0, ( WIDTH * HEIGHT + 6 ) * sizeof( uint16_t ) );

    // init ESP
    ESP8266_SetConfig( &huart3, ESP_PW_GPIO_Port, ESP_PW_Pin );
    ESP8266_ON();
    ESP8266_Send( "AT+CWMODE=1\r\n" );
    if ( !ESP8266_Recv( "OK" ) )
        Error_Handler();

    ESP8266_Send( "AT+CIPSSLSIZE=4096\r\n" );
    if ( !ESP8266_Recv( "OK" ) )
        Error_Handler();

    ESP8266_Send( "AT+CIPMUX=1\r\n" );
    if ( !ESP8266_Recv( "OK" ) )
        Error_Handler();

    while ( !ESP8266_ConnectTo( ESP_SSID, ESP_SSID_PASSWORD ) )
    {
    }
    updateTime();

    // if ( ESP8266_SendRequest( "TCP", "moscowtransport.app", 80, "GET /api/stop_v2/7fce7321-a3ac-4648-8919-3f728cc166c7 HTTP/1.1\r\n"
    //                                                             "Host: moscowtransport.app\r\n"
    //                                                             "User-Agent: ESP8266\r\n"
    //                                                             "Accept: application/json\r\n"
    //                                                             "Connection: close\r\n" ) )

    HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_SNAPSHOT, ( uint32_t ) ( ( reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ] ) + 8 ) ), WIDTH * HEIGHT / 2 );

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while ( 1 )
    {
        RTC_TimeTypeDef sTime;
        RTC_DateTypeDef sDate;
        HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
        HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
        char timeStr[ 20 ];
        snprintf( timeStr, 20, "Time %d:%d:%d\n", sTime.Hours, sTime.Minutes, sTime.Seconds );
        if ( CDC_RxStatus )
        {
            if ( UserRxBufferFS[ 0 ] == 'x' ) // x offset
            {
                offsetWithZoom[ 0 ] = ( *reinterpret_cast<uint16_t *>( &UserRxBufferFS[ 1 ] ) );
                newConfigProcessed  = false;
            }
            else if ( UserRxBufferFS[ 0 ] == 'y' ) // y offset
            {
                offsetWithZoom[ 1 ] = ( ( offsetWithZoom[ 1 ] & ~0xFFF ) | ( *reinterpret_cast<uint16_t *>( &UserRxBufferFS[ 1 ] ) & 0xFFF ) );
                newConfigProcessed  = false;
            }
            else if ( UserRxBufferFS[ 0 ] == 'e' ) // y offset
            {
                aec                = *reinterpret_cast<uint16_t *>( &UserRxBufferFS[ 1 ] );
                newConfigProcessed = false;
            }
            else if ( UserRxBufferFS[ 0 ] == 's' ) // shoot
            {
                char name[ 18 ];
                uint32_t photoNum { IncrementLastPhotoNumber() };
                if ( photoNum )
                {
                    sprintf( name, "/shots/img%d.bmp", photoNum );
                    SaveImageBMP( name, reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ][ 4 ] ), WIDTH * HEIGHT * 2 );
                    enableLed2ms();
                }
            }
            else if ( UserRxBufferFS[ 0 ] == 't' ) // toggle
            {
                ReSetToggle();
            }
            CDC_RxStatus = 0;
        }
        if ( newFrame )
        {
            newFrame = false;
            if ( CameraCountDownEnded && getToggle() && sdCardPresented )
            {
                {
                    auto testResult = testForBus();
                    HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_SNAPSHOT, ( uint32_t ) ( ( reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ] ) + 8 ) ), WIDTH * HEIGHT / 2 );
                    if ( testResult )
                    {
                        char name[ 18 ];
                        uint32_t photoNum { IncrementLastPhotoNumber() };
                        if ( photoNum )
                        {
                            sprintf( name, "/data/img%d-%s-%d.bmp", photoNum, nightMode ? "n" : "d", avg );
                            SaveImageBMP( name, reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ][ 4 ] ), WIDTH * HEIGHT * 2 );
                            enableLed2ms();
                        }
                        CameraCountDownEnded = false;
                        StartCountdown();
                    }
                    if ( debugCameraPattern )
                    {
                        char name[ 20 ];
                        uint32_t photoNum { IncrementLastPhotoNumber() };
                        if ( photoNum )
                        {
                            sprintf( name, "/debug/d%d-%c-%d.bmp", photoNum, nightMode ? 'n' : 'd', avg );
                            SaveImageBMP( name, reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ][ 4 ] ), WIDTH * HEIGHT * 2 );
                            enableLed2ms();
                        }
                        debugCameraPattern = false;
                    }
                }
            }
            else
            {
                HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_SNAPSHOT, ( uint32_t ) ( ( reinterpret_cast<uint8_t *>( &frameBuffers[ 0 ] ) + 8 ) ), WIDTH * HEIGHT / 2 );
            }
            CDC_TX_FRAME();
        }
        if ( sdCardPresented && !sdCardMounted )
        {
            HAL_SD_DeInit( &hsd1 );
            if ( HAL_SD_Init( &hsd1 ) == HAL_OK )
            {
                MX_FATFS_Init();
                if ( f_mount( &FatFs, SDPath, 1 ) != FR_OK )
                {
                    HAL_SD_DeInit( &hsd1 );
                }
                else
                    sdCardMounted = 1;
            }
        }
        if ( timeToCalibrateRTC )
        {
            updateTime();
            timeToCalibrateRTC = false;
        }
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
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE0 );

    while ( !__HAL_PWR_GET_FLAG( PWR_FLAG_VOSRDY ) )
    {
    }

    /** Configure LSE Drive Capability
     */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState       = RCC_LSE_ON;
    RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 6;
    RCC_OscInitStruct.PLL.PLLN       = 240;
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
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

    if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
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
    hi2c1.Init.Timing           = 0x107075B0;
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
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init( void )
{
    /* USER CODE BEGIN RTC_Init 0 */

    /* USER CODE END RTC_Init 0 */

    RTC_TimeTypeDef sTime   = { 0 };
    RTC_DateTypeDef sDate   = { 0 };
    RTC_AlarmTypeDef sAlarm = { 0 };

    /* USER CODE BEGIN RTC_Init 1 */

    /* USER CODE END RTC_Init 1 */

    /** Initialize RTC Only
     */
    hrtc.Instance            = RTC;
    hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv   = 127;
    hrtc.Init.SynchPrediv    = 255;
    hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
    hrtc.Init.OutPutRemap    = RTC_OUTPUT_REMAP_NONE;
    if ( HAL_RTC_Init( &hrtc ) != HAL_OK )
    {
        Error_Handler();
    }

    /* USER CODE BEGIN Check_RTC_BKUP */

    /* USER CODE END Check_RTC_BKUP */

    /** Initialize RTC and set the Time and Date
     */
    sTime.Hours          = 0;
    sTime.Minutes        = 0;
    sTime.Seconds        = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if ( HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN ) != HAL_OK )
    {
        Error_Handler();
    }
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month   = RTC_MONTH_JANUARY;
    sDate.Date    = 1;
    sDate.Year    = 0;
    if ( HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Enable the Alarm A
     */
    sAlarm.AlarmTime.Hours          = 0;
    sAlarm.AlarmTime.Minutes        = 0;
    sAlarm.AlarmTime.Seconds        = 0;
    sAlarm.AlarmTime.SubSeconds     = 0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask                = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask       = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel      = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay         = 1;
    sAlarm.Alarm                    = RTC_ALARM_A;
    if ( HAL_RTC_SetAlarm_IT( &hrtc, &sAlarm, RTC_FORMAT_BIN ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN RTC_Init 2 */

    /* USER CODE END RTC_Init 2 */
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
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    hsd1.Init.ClockDiv            = 4;
    if ( HAL_SD_Init( &hsd1 ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SDMMC1_Init 2 */

    /* USER CODE END SDMMC1_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init( void )
{
    /* USER CODE BEGIN TIM3_Init 0 */

    /* USER CODE END TIM3_Init 0 */

    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC          = { 0 };

    /* USER CODE BEGIN TIM3_Init 1 */

    /* USER CODE END TIM3_Init 1 */
    htim3.Instance               = TIM3;
    htim3.Init.Prescaler         = 1199;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 99;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if ( HAL_TIM_OC_Init( &htim3 ) != HAL_OK )
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if ( HAL_TIMEx_MasterConfigSynchronization( &htim3, &sMasterConfig ) != HAL_OK )
    {
        Error_Handler();
    }
    sConfigOC.OCMode     = TIM_OCMODE_TIMING;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if ( HAL_TIM_OC_ConfigChannel( &htim3, &sConfigOC, TIM_CHANNEL_1 ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM3_Init 2 */

    /* USER CODE END TIM3_Init 2 */
    HAL_TIM_MspPostInit( &htim3 );
}

/**
 * @brief TIM7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM7_Init( void )
{
    /* USER CODE BEGIN TIM7_Init 0 */

    /* USER CODE END TIM7_Init 0 */

    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    /* USER CODE BEGIN TIM7_Init 1 */

    /* USER CODE END TIM7_Init 1 */
    htim7.Instance               = TIM7;
    htim7.Init.Prescaler         = 59999;
    htim7.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim7.Init.Period            = 14999;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if ( HAL_TIM_Base_Init( &htim7 ) != HAL_OK )
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if ( HAL_TIMEx_MasterConfigSynchronization( &htim7, &sMasterConfig ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM7_Init 2 */

    /* USER CODE END TIM7_Init 2 */
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init( void )
{
    /* USER CODE BEGIN USART3_Init 0 */

    /* USER CODE END USART3_Init 0 */

    /* USER CODE BEGIN USART3_Init 1 */

    /* USER CODE END USART3_Init 1 */
    huart3.Instance                    = USART3;
    huart3.Init.BaudRate               = 115200;
    huart3.Init.WordLength             = UART_WORDLENGTH_8B;
    huart3.Init.StopBits               = UART_STOPBITS_1;
    huart3.Init.Parity                 = UART_PARITY_NONE;
    huart3.Init.Mode                   = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if ( HAL_UART_Init( &huart3 ) != HAL_OK )
    {
        Error_Handler();
    }
    if ( HAL_UARTEx_SetTxFifoThreshold( &huart3, UART_TXFIFO_THRESHOLD_1_8 ) != HAL_OK )
    {
        Error_Handler();
    }
    if ( HAL_UARTEx_SetRxFifoThreshold( &huart3, UART_RXFIFO_THRESHOLD_1_8 ) != HAL_OK )
    {
        Error_Handler();
    }
    if ( HAL_UARTEx_DisableFifoMode( &huart3 ) != HAL_OK )
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART3_Init 2 */

    /* USER CODE END USART3_Init 2 */
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET );

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( DCMI_PWDN_GPIO_Port, DCMI_PWDN_Pin, GPIO_PIN_RESET );

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( ESP_PW_GPIO_Port, ESP_PW_Pin, GPIO_PIN_SET );

    /*Configure GPIO pin : LED_Pin */
    GPIO_InitStruct.Pin   = LED_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( LED_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : BUTTON_Pin */
    GPIO_InitStruct.Pin  = BUTTON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init( BUTTON_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : DCMI_PWDN_Pin */
    GPIO_InitStruct.Pin   = DCMI_PWDN_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( DCMI_PWDN_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : ESP_PW_Pin */
    GPIO_InitStruct.Pin   = ESP_PW_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( ESP_PW_GPIO_Port, &GPIO_InitStruct );

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
    HAL_NVIC_SetPriority( BUTTON_EXTI_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( BUTTON_EXTI_IRQn );

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
    /* User can add his own implementation to report the FatFsFile name and line number,
       //ex: printf("Wrong parameters value: FatFsFile %s on line %d\r\n", FatFsFile, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
