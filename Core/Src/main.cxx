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
#define OV3640_AGC_H              0x3000
#define OV3640_AGC_L              0x3001
#define OV3640_AEC_H              0x3002
#define OV3640_AEC_L              0x3003
#define OV3640_AECL               0x3004
#define OV3640_RED                0x3005
#define OV3640_GREEN              0x3006
#define OV3640_BLUE               0x3007
#define OV3640_PIDH               0x300A
#define OV3640_PIDL               0x300B
#define OV3640_SCCB_ID            0x300C
#define OV3640_PCLK               0x300D
#define OV3640_PCLK_HREFQUAL_OUT  ( 1 << 5 )
#define OV3640_PCLK_REVERSE       ( 1 << 4 )
#define OV3640_PCLK_DIVBY4        ( 1 << 1 )
#define OV3640_PCLK_DIVBY2        1
#define OV3640_PLL_1              0x300E
#define OV3640_PLL_1_RXPLL_MASK   0x3F
#define OV3640_PLL_2              0x300F
#define OV3640_PLL_2_FREQDIV_MASK ( 0x3 << 6 )
#define OV3640_PLL_2_FREQDIV_1    ( 0x0 << 6 )
#define OV3640_PLL_2_FREQDIV_1_5  ( 0x1 << 6 )
#define OV3640_PLL_2_FREQDIV_2    ( 0x2 << 6 )
#define OV3640_PLL_2_FREQDIV_3    ( 0x3 << 6 )

#define OV3640_PLL_2_BIT8DIV_MASK ( 0x3 << 4 )
#define OV3640_PLL_2_BIT8DIV_1    ( 0x0 << 4 )
#define OV3640_PLL_2_BIT8DIV_1_2  ( 0x1 << 4 )
#define OV3640_PLL_2_BIT8DIV_4    ( 0x2 << 4 )
#define OV3640_PLL_2_BIT8DIV_5    ( 0x3 << 4 )

#define OV3640_PLL_2_BYPASS ( 1 << 3 )

#define OV3640_PLL_2_INDIV_MASK 0x3
#define OV3640_PLL_2_INDIV_1    0x0
#define OV3640_PLL_2_INDIV_1_5  0x1
#define OV3640_PLL_2_INDIV_2    0x2
#define OV3640_PLL_2_INDIV_3    0x3

#define OV3640_PLL_3             0x3010
#define OV3640_PLL_3_DVPDIV_MASK ( 0x3 << 6 )
#define OV3640_PLL_3_DVPDIV_1    ( 0x0 << 6 )
#define OV3640_PLL_3_DVPDIV_2    ( 0x1 << 6 )
#define OV3640_PLL_3_DVPDIV_8    ( 0x2 << 6 )
#define OV3640_PLL_3_DVPDIV_16   ( 0x3 << 6 )

#define OV3640_PLL_3_LANEDIV2LANES ( 1 << 5 )
#define OV3640_PLL_3_SENSORDIV2    ( 1 << 4 )

#define OV3640_PLL_3_SCALEDIV_MASK 0xF

#define OV3640_CLK           0x3011
#define OV3640_CLK_DFREQDBL  ( 1 << 7 )
#define OV3640_CLK_SLAVEMODE ( 1 << 6 )
#define OV3640_CLK_DIV_MASK  0x3F

#define OV3640_SYS              0x3012
#define OV3640_SYS_SRST         ( 1 << 7 )
#define OV3640_SYS_BASERES_MASK ( 0x7 << 4 )
#define OV3640_SYS_BASERES_QXGA ( 0x0 << 4 )
#define OV3640_SYS_BASERES_XGA  ( 0x1 << 4 )
#define OV3640_SYS_BASERES_SXGA ( 0x7 << 4 )

#define OV3640_AUTO_1                   0x3013
#define OV3640_AUTO_1_FASTAEC           ( 1 << 7 )
#define OV3640_AUTO_1_AECBIGSTEPS       ( 1 << 6 )
#define OV3640_AUTO_1_BANDINGFILTEREN   ( 1 << 5 )
#define OV3640_AUTO_1_AUTOBANDINGFILTER ( 1 << 4 )
#define OV3640_AUTO_1_EXTRBRIGHTEXPEN   ( 1 << 3 )
#define OV3640_AUTO_1_AGCEN             ( 1 << 2 )
#define OV3640_AUTO_1_AECEN             1

#define OV3640_AUTO_2                    0x3014
#define OV3640_AUTO_2_MANBANDING50       ( 1 << 7 )
#define OV3640_AUTO_2_AUTOBANDINGDETEN   ( 1 << 6 )
#define OV3640_AUTO_2_AGCADDLT1F         ( 1 << 5 )
#define OV3640_AUTO_2_FREEZEAECAGC       ( 1 << 4 )
#define OV3640_AUTO_2_NIGHTMODEEN        ( 1 << 3 )
#define OV3640_AUTO_2_BANDINGSMOOTHSW    ( 1 << 2 )
#define OV3640_AUTO_2_MANEXTRBRIGHTEXPEN ( 1 << 1 )
#define OV3640_AUTO_2_BANDINGFILTEREN    1

#define OV3640_AUTO_3                0x3015
#define OV3640_AUTO_3_DUMMYFC_MASK   ( 0x7 << 4 )
#define OV3640_AUTO_3_DUMMYFC_NONE   ( 0x0 << 4 )
#define OV3640_AUTO_3_DUMMYFC_1FRAME ( 0x1 << 4 )
#define OV3640_AUTO_3_DUMMYFC_2FRAME ( 0x2 << 4 )
#define OV3640_AUTO_3_DUMMYFC_3FRAME ( 0x3 << 4 )
#define OV3640_AUTO_3_DUMMYFC_7FRAME ( 0x7 << 4 )

#define OV3640_AUTO_3_AGCGAINCEIL_MASK   0x7
#define OV3640_AUTO_3_AGCGAINCEIL_2X     0x0
#define OV3640_AUTO_3_AGCGAINCEIL_4X     0x1
#define OV3640_AUTO_3_AGCGAINCEIL_8X     0x2
#define OV3640_AUTO_3_AGCGAINCEIL_16X    0x3
#define OV3640_AUTO_3_AGCGAINCEIL_32X    0x4
#define OV3640_AUTO_3_AGCGAINCEIL_64X    0x5
#define OV3640_AUTO_3_AGCGAINCEIL_128X   0x6
#define OV3640_AUTO_3_AGCGAINCEIL_128X_2 0x7

#define OV3640_AUTO_5                    0x3017
#define OV3640_AUTO_5_MANBANDINGCNT_MASK 0x3F

#define OV3640_WPT_HISH   0x3018
#define OV3640_BPT_HISL   0x3019
#define OV3640_VPT        0x301A
#define OV3640_YAVG       0x301B
#define OV3640_AECG_MAX50 0x301C
#define OV3640_AECG_MAX60 0x301D
#define OV3640_RZM_H      0x301E
#define OV3640_RZM_L      0x301F

#define OV3640_HS_H    0x3020
#define OV3640_HS_L    0x3021
#define OV3640_VS_H    0x3022
#define OV3640_VS_L    0x3023
#define OV3640_HW_H    0x3024
#define OV3640_HW_L    0x3025
#define OV3640_VH_H    0x3026
#define OV3640_VH_L    0x3027
#define OV3640_HTS_H   0x3028
#define OV3640_HTS_L   0x3029
#define OV3640_VTS_H   0x302A
#define OV3640_VTS_L   0x302B
#define OV3640_EXHTS   0x302C
#define OV3640_EXVTS_H 0x302D
#define OV3640_EXVTS_L 0x302E

#define OV3640_WEIGHT0 0x3030
#define OV3640_WEIGHT1 0x3031
#define OV3640_WEIGHT2 0x3032
#define OV3640_WEIGHT3 0x3033
#define OV3640_WEIGHT4 0x3034
#define OV3640_WEIGHT5 0x3035
#define OV3640_WEIGHT6 0x3036
#define OV3640_WEIGHT7 0x3037
#define OV3640_AHS_H   0x3038
#define OV3640_AHS_L   0x3039
#define OV3640_AVS_H   0x303A
#define OV3640_AVS_L   0x303B
#define OV3640_AHW_H   0x303C
#define OV3640_AHW_L   0x303D
#define OV3640_AVH_H   0x303E
#define OV3640_AVH_L   0x303F

#define OV3640_HISTO0 0x3040
#define OV3640_HISTO1 0x3041
#define OV3640_HISTO2 0x3042
#define OV3640_HISTO3 0x3043
#define OV3640_HISTO4 0x3044
#define OV3640_HISTO5 0x3045
#define OV3640_HISTO6 0x3046
#define OV3640_HISTO7 0x3047
#define OV3640_D56C1  0x3048

#define OV3640_BLC9 0x3069

#define OV3640_BD50_H            0x3070
#define OV3640_BD50_L            0x3071
#define OV3640_BD60_H            0x3072
#define OV3640_BD60_L            0x3073
#define OV3640_VSYNCOPT          0x3075
#define OV3640_TMC1              0x3077
#define OV3640_TMC1_CHSYNCSWAP   ( 1 << 7 )
#define OV3640_TMC1_HREFSWAP     ( 1 << 6 )
#define OV3640_TMC1_HREFPOL_NEG  ( 1 << 3 )
#define OV3640_TMC1_VSYNCPOL_NEG ( 1 << 1 )
#define OV3640_TMC1_HSYNCPOL_NEG 1

#define OV3640_TMC2               0x3078
#define OV3640_TMC2_VSYNCDROP     ( 1 << 1 )
#define OV3640_TMC2_FRAMEDATADROP 1

#define OV3640_TMC3         0x3079
#define OV3640_TMC3_VSLATCH ( 1 << 7 )

#define OV3640_TMC4                   0x307A
#define OV3640_TMC5                   0x307B
#define OV3640_TMC5_AWB_GAINWRITE_DIS ( 1 << 7 )
#define OV3640_TMC5_DCOLORBAREN       ( 1 << 3 )
#define OV3640_TMC5_DCOLORBARPAT_MASK 0x7

#define OV3640_TMC6         0x307C
#define OV3640_TMC6_DGAINEN ( 1 << 5 )
#define OV3640_TMC6_HMIRROR ( 1 << 1 )
#define OV3640_TMC6_VFLIP   ( 1 << 0 )

#define OV3640_TMC7                   0x307D
#define OV3640_TMC7_COLORBARTESTPATEN ( 1 << 7 )
#define OV3640_TMC7_AVGHIST_SENSOR    ( 1 << 5 )

#define OV3640_TMC8 0x307E

#define OV3640_TMCA               0x3080
#define OV3640_TMCB               0x3081
#define OV3640_TMCB_MIRROROPTEN   ( 1 << 7 )
#define OV3640_TMCB_OTPFASTMEMCLK ( 1 << 6 )
#define OV3640_TMCB_SWAPBYTESOUT  1

#define OV3640_TMCF              0x3085
#define OV3640_TMC10             0x3086
#define OV3640_TMC10_SYSRST      ( 1 << 3 )
#define OV3640_TMC10_REGSLEEPOPT ( 1 << 2 )
#define OV3640_TMC10_SLEEPOPT    ( 1 << 1 )
#define OV3640_TMC10_SLEEPEN     1

#define OV3640_TMC11      0x3087
#define OV3640_ISP_XOUT_H 0x3088
#define OV3640_ISP_XOUT_L 0x3089
#define OV3640_ISP_YOUT_H 0x308A
#define OV3640_ISP_YOUT_L 0x308B
#define OV3640_TMC13      0x308D
#define OV3640_5060       0x308E
#define OV3640_OTP        0x308F

#define OV3640_IO_CTRL0 0x30B0
#define OV3640_IO_CTRL1 0x30B1
#define OV3640_IO_CTRL2 0x30B2
#define OV3640_DVP0     0x30B4
#define OV3640_DVP1     0x30B5
#define OV3640_DVP2     0x30B6
#define OV3640_DVP3     0x30B7
#define OV3640_DSPC0    0x30B8
#define OV3640_DSPC1    0x30B9
#define OV3640_DSPC2    0x30BA
#define OV3640_DSPC3    0x30BB
#define OV3640_DSPC7    0x30BF
/*
 * END - System Control Registers
 */

/*
 * SC Registers
 */
#define OV3640_SC_CTRL0     0x3100
#define OV3640_SC_CTRL2     0x3102
#define OV3640_SC_SYN_CTRL0 0x3104
#define OV3640_SC_SYN_CTRL1 0x3105
#define OV3640_SC_SYN_CTRL2 0x3106
#define OV3640_SC_SYN_CTRL3 0x3107
/*
 * END - SC Registers
 */

/*
 * CIF Registers
 */
#define OV3640_CIF_CTRL0 0x3200
#define OV3640_CIF_CTRL4 0x3204
/*
 * END - CIF Registers
 */

/*
 * DSP Registers
 */
#define OV3640_DSP_CTRL_0 0x3300
#define OV3640_DSP_CTRL_1 0x3301
#define OV3640_DSP_CTRL_2 0x3302
#define OV3640_DSP_CTRL_4 0x3304
#define OV3640_AWB_CTRL_3 0x3308

#define OV3640_YST1 0x331B
#define OV3640_YST2 0x331C
#define OV3640_YST3 0x331D
#define OV3640_YST4 0x331E
#define OV3640_YST5 0x331F

#define OV3640_YST6         0x3320
#define OV3640_YST7         0x3321
#define OV3640_YST8         0x3322
#define OV3640_YST9         0x3323
#define OV3640_YST10        0x3324
#define OV3640_YST11        0x3325
#define OV3640_YST12        0x3326
#define OV3640_YST13        0x3327
#define OV3640_YST14        0x3328
#define OV3640_YST15        0x3329
#define OV3640_YSLP15       0x332A
#define OV3640_MISC_CTRL    0x332B
#define OV3640_DNS_TH       0x332C
#define OV3640_Y_EDGE_MT    0x332D
#define OV3640_Y_EDGE_TH_TM 0x332E
#define OV3640_BASE1        0x332F

#define OV3640_BASE2        0x3330
#define OV3640_OFFSET       0x3331
#define OV3640_CMXSIGN_MISC 0x333F

#define OV3640_CMX_1   0x3340
#define OV3640_CMX_2   0x3341
#define OV3640_CMX_3   0x3342
#define OV3640_CMX_4   0x3343
#define OV3640_CMX_5   0x3344
#define OV3640_CMX_6   0x3345
#define OV3640_CMX_7   0x3346
#define OV3640_CMX_8   0x3347
#define OV3640_CMX_9   0x3348
#define OV3640_CMXSIGN 0x3349

#define OV3640_SGNSET       0x3354
#define OV3640_SDE_CTRL     0x3355
#define OV3640_HUE_COS      0x3356
#define OV3640_HUE_SIN      0x3357
#define OV3640_SAT_U        0x3358
#define OV3640_SAT_V        0x3359
#define OV3640_UREG         0x335A
#define OV3640_VREG         0x335B
#define OV3640_YOFFSET      0x335C
#define OV3640_YGAIN        0x335D
#define OV3640_YBRIGHT      0x335E
#define OV3640_SIZE_IN_MISC 0x335F

#define OV3640_HSIZE_IN_L    0x3360
#define OV3640_VSIZE_IN_L    0x3361
#define OV3640_SIZE_OUT_MISC 0x3362
#define OV3640_HSIZE_OUT_L   0x3363
#define OV3640_VSIZE_OUT_L   0x3364

#define OV3640_R_XY0   0x3367
#define OV3640_R_X0    0x3368
#define OV3640_R_Y0    0x3369
#define OV3640_R_A1    0x336A
#define OV3640_R_A2_B2 0x336B
#define OV3640_R_B1    0x336C
#define OV3640_G_XY0   0x336D
#define OV3640_G_X0    0x336E
#define OV3640_G_Y0    0x336F

#define OV3640_G_A1    0x3370
#define OV3640_G_A2_B2 0x3371
#define OV3640_G_B1    0x3372
#define OV3640_B_XY0   0x3373
#define OV3640_B_X0    0x3374
#define OV3640_B_Y0    0x3375
#define OV3640_B_A1    0x3376
#define OV3640_B_A2_B2 0x3377
#define OV3640_B_B1    0x3378

#define OV3640_MISC_DCW_SIZE 0x33A4
#define OV3640_DCW_OH        0x33A5
#define OV3640_DCW_OV        0x33A6
#define OV3640_R_GAIN_M      0x33A7
#define OV3640_G_GAIN_M      0x33A8
#define OV3640_B_GAIN_M      0x33A9
#define OV3640_OVLY_MISC1    0x33AA
#define OV3640_OVLY_LEFT     0x33AB
#define OV3640_OVLY_TOP      0x33AC
#define OV3640_OVLY_MISC2    0x33AD
#define OV3640_OVLY_RIGHT    0x33AE
#define OV3640_OVLY_BOTTEM   0x33AF

#define OV3640_OVLY_MISC3       0x33B0
#define OV3640_OVLY_EXT_WIDTH_H 0x33B1
#define OV3640_OVLY_EXT_WIDTH_L 0x33B2
#define OV3640_OVLY_Y           0x33B3
#define OV3640_OVLY_U           0x33B4
#define OV3640_OVLY_V           0x33B5

#define OV2640_BANK_SEL        0xFF
#define OV2640_BANK_SEL_DSP    0x00
#define OV2640_BANK_SEL_SENSOR 0x01

/* DSP bank selected (0xFF = 0x00) */
#define OV2640_DSP_R_BYPASS   0x05
#define OV2640_DSP_QS         0x44
#define OV2640_DSP_CTRL       0x50
#define OV2640_DSP_HSIZE      0x51
#define OV2640_DSP_VSIZE      0x52
#define OV2640_DSP_XOFFL      0x53
#define OV2640_DSP_YOFFL      0x54
#define OV2640_DSP_VHYX       0x55
#define OV2640_DSP_DPRP       0x56
#define OV2640_DSP_TEST       0x57
#define OV2640_DSP_ZMOW       0x5A
#define OV2640_DSP_ZMOH       0x5B
#define OV2640_DSP_ZMHH       0x5C
#define OV2640_DSP_BPADDR     0x7C
#define OV2640_DSP_BPDATA     0x7D
#define OV2640_DSP_CTRL2      0x86
#define OV2640_DSP_CTRL3      0x87
#define OV2640_DSP_SIZEL      0x8C
#define OV2640_DSP_HSIZE8     0xC0
#define OV2640_DSP_VSIZE8     0xC1
#define OV2640_DSP_CTRL0      0xC2
#define OV2640_DSP_CTRL1      0xC3
#define OV2640_DSP_R_DVP_SP   0xD3
#define OV2640_DSP_IMAGE_MODE 0xDA
#define OV2640_DSP_RESET      0xE0
#define OV2640_DSP_MS_SP      0xF0
#define OV2640_DSP_SS_ID      0x7F
#define OV2640_DSP_SS_CTRL    0xF8
#define OV2640_DSP_MC_BIST    0xF9
#define OV2640_DSP_MC_AL      0xFA
#define OV2640_DSP_MC_AH      0xFB
#define OV2640_DSP_MC_D       0xFC
#define OV2640_DSP_P_STATUS   0xFE
#define OV2640_DSP_RA_DLMT    0xFF

/* sensor bank selected (0xFF = 0x01) */
#define OV2640_SENSOR_GAIN       0x00
#define OV2640_SENSOR_COM1       0x03
#define OV2640_SENSOR_REG04      0x04
#define OV2640_SENSOR_REG08      0x08
#define OV2640_SENSOR_COM2       0x09
#define OV2640_SENSOR_PIDH       0x0A
#define OV2640_SENSOR_PIDL       0x0B
#define OV2640_SENSOR_COM3       0x0C
#define OV2640_SENSOR_COM4       0x0D
#define OV2640_SENSOR_AEC        0x10
#define OV2640_SENSOR_CLKRC      0x11
#define OV2640_SENSOR_COM7       0x12
#define OV2640_SENSOR_COM8       0x13
#define OV2640_SENSOR_COM9       0x14
#define OV2640_SENSOR_COM10      0x15
#define OV2640_SENSOR_HREFST     0x17
#define OV2640_SENSOR_HREFEND    0x18
#define OV2640_SENSOR_VSTART     0x19
#define OV2640_SENSOR_VEND       0x1A
#define OV2640_SENSOR_MIDH       0x1C
#define OV2640_SENSOR_MIDL       0x1D
#define OV2640_SENSOR_AEW        0x24
#define OV2640_SENSOR_AEB        0x25
#define OV2640_SENSOR_W          0x26
#define OV2640_SENSOR_REG2A      0x2A
#define OV2640_SENSOR_FRARL      0x2B
#define OV2640_SENSOR_ADDVSL     0x2D
#define OV2640_SENSOR_ADDVHS     0x2E
#define OV2640_SENSOR_YAVG       0x2F
#define OV2640_SENSOR_REG32      0x32
#define OV2640_SENSOR_ARCOM2     0x34
#define OV2640_SENSOR_REG45      0x45
#define OV2640_SENSOR_FLL        0x46 /* Frame Length Adjustment LSBs */
#define OV2640_SENSOR_FLH        0x47 /* Frame Length Adjustment MSBs */
#define OV2640_SENSOR_COM19      0x48
#define OV2640_SENSOR_ZOOMS      0x49 /* Zoom: Vertical start point */
#define OV2640_SENSOR_COM22      0x4B
#define OV2640_SENSOR_COM25      0x4E
#define OV2640_SENSOR_BD50       0x4F
#define OV2640_SENSOR_BD60       0x50
#define OV2640_SENSOR_REG5D      0x5D
#define OV2640_SENSOR_REG5E      0x5E
#define OV2640_SENSOR_REG5F      0x5F
#define OV2640_SENSOR_REG60      0x60
#define OV2640_SENSOR_HISTO_LOW  0x61 /* Histogram Algorithm Low Level */
#define OV2640_SENSOR_HISTO_HIGH 0x62 /* Histogram Algorithm High Level */
/*
 * END - DSP Registers
 */

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

struct sensor_reg
{
    uint8_t reg;
    uint8_t val;
};
const struct sensor_reg OV2640_INIT[] =
    {
        // DSP
        { OV2640_BANK_SEL, 0x0 },
        { 0x2c, 0xff },
        { 0x2e, 0xdf },

        // SENSOR
        { OV2640_BANK_SEL, 0x1 },
        { 0x3c, 0x32 },                // ok
        { OV2640_SENSOR_CLKRC, 0x0 },  // ok vs 0x01
        { OV2640_SENSOR_COM2, 0x02 },  // ok
        { OV2640_SENSOR_REG04, 0xa8 }, // vs 0x28, horizontal mirror ?
                                       //{OV2640_SENSOR_REG04, 0x28},
        { OV2640_SENSOR_COM8, 0xe5 },  // ok
        { OV2640_SENSOR_COM9, 0x48 },
        //{OV2640_SENSOR_COM9, 0x40},	// 0x20 or 0x40 ?
        { 0x2c, 0x0c },                 // ok
        { 0x33, 0x78 },                 // ok
        { 0x3a, 0x33 },                 // ok
        { 0x3b, 0xfb },                 // ok
        { 0x3e, 0x00 },                 // ok
        { 0x43, 0x11 },                 // ok
        { 0x16, 0x10 },                 // ok
        { 0x39, 0x02 },                 // vs 0x92
        { 0x35, 0x88 },                 // vs 0xda
        { 0x22, 0x0a },                 // vs 0x1a
        { 0x37, 0x40 },                 // vs c3
        { 0x23, 0x00 },                 // ok
        { OV2640_SENSOR_ARCOM2, 0xa0 }, // 0xa0 or 0xc0 ? defaul is 0x20 - reserved...
        { OV2640_SENSOR_ARCOM2, 0x20 }, // 0xa0 or 0xc0 ? defaul is 0x20 - reserved...
        { 0x06, 0x88 },                 // ok
        { 0x07, 0xc0 },                 // ok
        { OV2640_SENSOR_COM4, 0xb7 },   // 0x87 or 0xb7, reserved
        { 0x0E, 0x01 },                 // vs 0x41
        { 0x4c, 0x00 },                 // ok
        { 0x4a, 0x81 },                 // ok
        { 0x21, 0x99 },                 // ok
        { OV2640_SENSOR_AEW, 0x40 },    // ok
        { OV2640_SENSOR_AEB, 0x38 },    // ok
        { OV2640_SENSOR_W, 0x82 },      // ok
        { 0x5c, 0x00 },                 // ok
        { 0x63, 0x00 },                 // ok

        // start diff
        { OV2640_SENSOR_HISTO_LOW, 0x70 },
        { OV2640_SENSOR_HISTO_HIGH, 0x80 },
        { 0x7c, 0x05 },
        { 0x20, 0x80 },
        { 0x28, 0x30 },
        { 0x6c, 0x00 },
        { 0x6d, 0x80 },
        { 0x6e, 0x00 },
        { 0x70, 0x02 },
        { 0x71, 0x94 },
        { 0x73, 0xc1 },
        { 0x3d, 0x34 },
        { 0x5a, 0x57 },
        { OV2640_SENSOR_BD50, 0xbb },
        { OV2640_SENSOR_BD60, 0x9c },
        // end diff

        //{OV2640_SENSOR_FLL, 0x22}, // frame length ? - same with or without
        //{OV2640_SENSOR_COM3, 0x3a}, // not present -
        { OV2640_SENSOR_COM3, 0x38 },       // not present - this is the default. works.
        { OV2640_SENSOR_REG5D, 0x55 },      // not present
        { OV2640_SENSOR_REG5E, 0x7d },      // not present
        { OV2640_SENSOR_REG5F, 0x7d },      // not present
        { OV2640_SENSOR_REG60, 0x55 },      // not present
        { OV2640_SENSOR_HISTO_LOW, 0x70 },  // ok
        { OV2640_SENSOR_HISTO_HIGH, 0x80 }, // ok
        { 0x7c, 0x05 },                     // ok
        { 0x20, 0x80 },                     // ok
        { 0x28, 0x30 },                     // ok
        { 0x6c, 0x00 },                     // ok
        { 0x6d, 0x80 },                     // ok
        { 0x6e, 0x00 },                     // ok
        { 0x70, 0x02 },                     // ok
        { 0x71, 0x94 },                     // ok
        { 0x73, 0xc1 },                     // ok
        { 0x3d, 0x34 },                     // ok
        { OV2640_SENSOR_COM7, 0x04 },
        //{OV2640_SENSOR_COM7, 0x24},

        //{OV2640_SENSOR_COM7, 0x24},
        //{OV2640_SENSOR_HREFST, 0x11},
        //{OV2640_SENSOR_HREFEND, 0x43},
        //{OV2640_SENSOR_VSTART, 0x00},
        //{OV2640_SENSOR_VEND, 0x25},
        //{OV2640_SENSOR_REG32, 0x89},

        { 0x5a, 0x57 }, // ok
        { 0x4f, 0xbb }, // not present
        { 0x50, 0x9c }, // not present

        // DSP
        { OV2640_BANK_SEL, 0x00 },
        { 0xe5, 0x7f },
        { 0xf9, 0xc0 },
        { 0x41, 0x24 },
        { OV2640_DSP_RESET, 0x14 },
        { 0x76, 0xff },
        { 0x33, 0xa0 },
        { 0x42, 0x20 },
        { 0x43, 0x18 },
        { 0x4c, 0x0 },
        { 0x87, 0xd0 },
        { 0x88, 0x3f },
        { 0xd7, 0x3 },
        { 0xd9, 0x10 },
        { OV2640_DSP_R_DVP_SP, 0x82 },
        { 0xc8, 0x8 },
        { 0xc9, 0x80 },
        { 0x7c, 0x0 },
        { 0x7d, 0x0 },
        { 0x7c, 0x3 },
        { 0x7d, 0x48 },
        { 0x7d, 0x48 },
        { 0x7c, 0x8 },
        { 0x7d, 0x20 },
        { 0x7d, 0x10 },
        { 0x7d, 0xe },
        { 0x90, 0x0 },
        { 0x91, 0xe },
        { 0x91, 0x1a },
        { 0x91, 0x31 },
        { 0x91, 0x5a },
        { 0x91, 0x69 },
        { 0x91, 0x75 },
        { 0x91, 0x7e },
        { 0x91, 0x88 },
        { 0x91, 0x8f },
        { 0x91, 0x96 },
        { 0x91, 0xa3 },
        { 0x91, 0xaf },
        { 0x91, 0xc4 },
        { 0x91, 0xd7 },
        { 0x91, 0xe8 },
        { 0x91, 0x20 },
        { 0x92, 0x0 },

        { 0x93, 0x6 },
        { 0x93, 0xe3 },
        { 0x93, 0x3 },
        { 0x93, 0x3 },
        { 0x93, 0x0 },
        { 0x93, 0x2 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x93, 0x0 },
        { 0x96, 0x0 },
        { 0x97, 0x8 },
        { 0x97, 0x19 },
        { 0x97, 0x2 },
        { 0x97, 0xc },
        { 0x97, 0x24 },
        { 0x97, 0x30 },
        { 0x97, 0x28 },
        { 0x97, 0x26 },
        { 0x97, 0x2 },
        { 0x97, 0x98 },
        { 0x97, 0x80 },
        { 0x97, 0x0 },
        { 0x97, 0x0 },
        { 0xa4, 0x0 },
        { 0xa8, 0x0 },
        { 0xc5, 0x11 },
        { 0xc6, 0x51 },
        { 0xbf, 0x80 },
        { 0xc7, 0x10 },
        { 0xb6, 0x66 },
        { 0xb8, 0xa5 },
        { 0xb7, 0x64 },
        { 0xb9, 0x7c },
        { 0xb3, 0xaf },
        { 0xb4, 0x97 },
        { 0xb5, 0xff },
        { 0xb0, 0xc5 },
        { 0xb1, 0x94 },
        { 0xb2, 0xf },
        { 0xc4, 0x5c },
        { 0xa6, 0x0 },
        { 0xa7, 0x20 },
        { 0xa7, 0xd8 },
        { 0xa7, 0x1b },
        { 0xa7, 0x31 },
        { 0xa7, 0x0 },
        { 0xa7, 0x18 },
        { 0xa7, 0x20 },
        { 0xa7, 0xd8 },
        { 0xa7, 0x19 },
        { 0xa7, 0x31 },
        { 0xa7, 0x0 },
        { 0xa7, 0x18 },
        { 0xa7, 0x20 },
        { 0xa7, 0xd8 },
        { 0xa7, 0x19 },
        { 0xa7, 0x31 },
        { 0xa7, 0x0 },
        { 0xa7, 0x18 },
        { 0x7f, 0x0 },
        { 0xe5, 0x1f },
        { 0xe1, 0x77 },
        { 0xdd, 0x7f },
        { 0xc2, 0xe },
        // DSP
        { OV2640_BANK_SEL, 0x0 },
        { OV2640_DSP_RESET, 0x4 },
        { OV2640_DSP_HSIZE8, 0xc8 }, // 0xc8 << 3 => 1600
        { OV2640_DSP_VSIZE8, 0x96 }, // 0x96 << 3 = > 1200
                                     //{OV2640_DSP_HSIZE8, 0x32}, // 0x32 << 32 => 400
                                     //{OV2640_DSP_VSIZE8, 0x25}, // 0x25 << 32 => 296

        // {OV2640_DSP_CTRL2, 0x3d}, // old
        { OV2640_DSP_CTRL2, 0x35 },

        { OV2640_DSP_HSIZE, 0x90 }, // 0x90 | 0x100 => 0x190 = 400 *4 = 1600
        { OV2640_DSP_VSIZE, 0x2c }, // 0x2c | 0x100 => 0x12C = 300 * 3 = 1200
        { OV2640_DSP_XOFFL, 0x0 },
        { OV2640_DSP_YOFFL, 0x0 },
        { OV2640_DSP_VHYX, 0x88 }, // for 1600x1200 and offset 0

        //{OV2640_DSP_HSIZE, 0x64}, // 0x64 = 100 * 4 = 400
        //{OV2640_DSP_VSIZE, 0x4a}, // 0x4A = 74 * 4 = 296
        //{OV2640_DSP_VHYX, 0x00},

        { OV2640_DSP_TEST, 0x00 },

        { OV2640_DSP_CTRL, 0x92 },
        { OV2640_DSP_ZMOW, 0x50 },
        { OV2640_DSP_ZMOH, 0x3c },
        { OV2640_DSP_ZMHH, 0x0 },

        { OV2640_DSP_R_DVP_SP, 0x04 },
        { OV2640_DSP_RESET, 0x00 },
        // DSP
        { OV2640_BANK_SEL, 0x00 },
        { OV2640_DSP_R_BYPASS, 0x00 },
        { OV2640_DSP_IMAGE_MODE, 0x80 }, // RGB 565
        { 0xd7, 0x30 },
        { OV2640_DSP_RESET, 0x00 },
        { OV2640_DSP_R_BYPASS, 0x00 },
};

const struct sensor_reg OV2640_320x240[] =
    {
        { OV2640_BANK_SEL, 0x01 },
        { 0x12, 0x40 },
        { 0x17, 0x11 },
        { 0x18, 0x43 },
        { 0x19, 0x00 },
        { 0x1a, 0x4b },
        { 0x32, 0x09 },
        { 0x4f, 0xca },
        { 0x50, 0xa8 },
        { 0x5a, 0x23 },
        { 0x6d, 0x00 },
        { 0x39, 0x12 },
        { 0x35, 0xda },
        { 0x22, 0x1a },
        { 0x37, 0xc3 },
        { 0x23, 0x00 },
        { 0x34, 0xc0 },
        { 0x36, 0x1a },
        { 0x06, 0x88 },
        { 0x07, 0xc0 },
        { 0x0d, 0x87 },
        { 0x0e, 0x41 },
        { 0x4c, 0x00 },
        { OV2640_BANK_SEL, 0x00 },
        { OV2640_DSP_RESET, 0x04 },
        { 0xc0, 0x64 },
        { 0xc1, 0x4b },
        { 0x86, 0x35 },
        { 0x50, 0x89 },
        { 0x51, 0xc8 },
        { 0x52, 0x96 },
        { 0x53, 0x00 },
        { 0x54, 0x00 },
        { 0x55, 0x00 },
        { 0x57, 0x00 },
        { 0x5a, 0x50 },
        { 0x5b, 0x3c },
        { 0x5c, 0x00 },
        { OV2640_DSP_RESET, 0x00 },
};
const struct sensor_reg OV2640_FORMAT_RGB565[] =
    {
        { OV2640_BANK_SEL, OV2640_BANK_SEL_DSP }, // DSP

        { OV2640_DSP_RESET, 0x04 },
        { OV2640_DSP_IMAGE_MODE, 0x08 },
        { 0xD7, 0x03 }, // same
        { 0xDF, 0x02 },
        { 0x33, 0xa0 },
        { 0x3C, 0x00 },
        { 0xe1, 0x77 }, // same
        { OV2640_DSP_RESET, 0x00 },

        { OV2640_BANK_SEL, OV2640_BANK_SEL_SENSOR }, // sensor
        { 0xe0, 0x00 },
        { 0xe1, 0x00 },
        { 0xe5, 0x00 },
        { 0xd7, 0x00 },
        { 0xda, 0x00 },
        { 0xe0, 0x00 },
};
const struct sensor_reg OV2640_FORMAT_CIF[] =
    {
        { OV2640_BANK_SEL, OV2640_BANK_SEL_SENSOR },
        { OV2640_SENSOR_COM7, 0x20 },

        // Set the sensor output window
        { OV2640_SENSOR_COM1, 0x0A },    // SVGA ?
        { OV2640_SENSOR_REG32, 0x89 },   // ????
        { OV2640_SENSOR_HREFST, 0x11 },  // ==> 0x45, 69 dec
        { OV2640_SENSOR_HREFEND, 0x43 }, // ==> 0x10E, 270 dec
        { OV2640_SENSOR_VSTART, 0x00 },  // 2
        { OV2640_SENSOR_VEND, 0x25 },    // 0x96, 150

        { OV2640_SENSOR_BD50, 0xca }, // ?
        { OV2640_SENSOR_BD60, 0xa8 }, // ?
        { 0x5a, 0x23 },
        { 0x6d, 0x00 },
        { 0x3d, 0x38 },
        { 0x39, 0x92 },
        { 0x35, 0xda },
        { 0x22, 0x1a },
        { 0x37, 0xc3 },
        { 0x23, 0x00 },
        { OV2640_SENSOR_ARCOM2, 0xc0 }, // dunno
        { 0x06, 0x88 },
        { 0x07, 0xc0 },
        { OV2640_SENSOR_COM4, 0x87 }, // reserved
        { 0x0e, 0x41 },
        { 0x4c, 0x00 },

        { OV2640_BANK_SEL, OV2640_BANK_SEL_DSP },
        { OV2640_DSP_RESET, 0x04 },

        // Set the sensor resolution (UXGA, SVGA, CIF)
        { OV2640_DSP_HSIZE8, 0x32 }, // raw 200
        { OV2640_DSP_VSIZE8, 0x25 }, // raw 148
        { OV2640_DSP_SIZEL, 0x00 },

        // Set the image window size >= output size
        { OV2640_DSP_HSIZE, 0x64 }, // 100*4 = 400
        { OV2640_DSP_VSIZE, 0x4a }, // 74*4 = 296
        { OV2640_DSP_XOFFL, 0x00 }, //
        { OV2640_DSP_YOFFL, 0x00 }, //
        { OV2640_DSP_VHYX, 0x00 },
        { OV2640_DSP_TEST, 0x00 },

        { OV2640_DSP_CTRL2, 0x20 | 0x1D },
        { OV2640_DSP_CTRL, 0x80 },
};

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
int sccb_config( const struct sensor_reg reglist[], uint16_t size )
{
    int err = 0;
    // int written = 0;

    for ( uint16_t i = 0; i < size; i++ )
    {
        err += gs_handle.sccb_write( 0x60, reglist[ i ].reg, &reglist[ i ].val, 1 );
    }

    return err;
}
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
    memset( &frameBuffers, 0, WIDTH * HEIGHT );
    HAL_DCMI_Start_DMA( &hdcmi, DCMI_MODE_CONTINUOUS, ( uint32_t ) &frameBuffers, WIDTH * HEIGHT / 2 );

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
    frameLen          = WIDTH * HEIGHT * 2;
    curentFrameBuffer = reinterpret_cast<uint8_t *>( frameBuffers[ 0 ] );
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
