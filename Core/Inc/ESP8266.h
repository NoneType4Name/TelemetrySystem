#ifndef ESP8266_H_
#define ESP8266_H_

#include "stdbool.h"
#include "stm32h7xx_hal.h"

#define ESP_RX_buff_size 1024
#define ESP_TX_buff_size 256

#define ESP_DEBUG

void ESP8266_ClearRecvBuff();

bool ESP8266_Send( const char *command );
bool ESP8266_Recv( const char *correctAnswer );

char *ESP8266_GetAcceessPoints();

bool ESP8266_Restart();

void ESP8266_SetConfig( UART_HandleTypeDef *huart, GPIO_TypeDef *pinPort, uint32_t pinNum );

void ESP8266_ON();
void ESP8266_OFF();

bool ESP8266_Test( void );

bool ESP8266_EnableEcho();
bool ESP8266_DisableEcho();

bool ESP8266_ConnectTo( const char *wifiName, const char *password );
bool ESP8266_DisconnectFromWifi();

char *ESP8266_SendRequest( const char *type, const char *ip, uint8_t port, const char *request );
bool ESP8266_AT_CIPSTART( const char *type, const char *ip, uint8_t port );
bool ESP8266_AT_CIPSEND( int requestLength );
bool ESP8266_AT_SendData( const char *request );

#endif /* ESP8266_H_ */