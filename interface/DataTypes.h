#include <cstdint>
#include "mainwindow.h"

struct RxData_T
{
  private:
    uint8_t begin[ 3 ] = { 'b', 'g', 'n' };

  public:
    uint16_t frame[ WIDTH * HEIGHT ];
    uint16_t cameraEnable : 1;
    uint32_t x : 8;
    uint32_t y : 7;
    uint8_t avgLuminance;
    uint8_t aec;
    uint32_t time;

  private:
    uint8_t end[ 3 ] = { 'e', 'n', 'd' };
};