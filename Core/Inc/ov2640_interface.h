#ifndef OV2640_INTERFACE_H
#define OV2640_INTERFACE_H

#include "main.h"
#include "driver_ov2640.h"
// SCCB (I2C)
uint8_t ov2640_interface_sccb_init( void );
uint8_t ov2640_interface_sccb_deinit( void );
uint8_t ov2640_interface_sccb_read( uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len );
uint8_t ov2640_interface_sccb_write( uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len );

// Power Down
uint8_t ov2640_interface_power_down_init( void );
uint8_t ov2640_interface_power_down_deinit( void );
uint8_t ov2640_interface_power_down_write( uint8_t level );

// Reset
uint8_t ov2640_interface_reset_init( void );
uint8_t ov2640_interface_reset_deinit( void );
uint8_t ov2640_interface_reset_write( uint8_t level );

//
void ov2640_interface_delay_ms( uint32_t ms );
void ov2640_interface_debug_print( const char *const fmt, ... );

#endif