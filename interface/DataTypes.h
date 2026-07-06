#pragma once
#ifndef DATATYPES_H
#    define DATATYPES_H
#    include <cstdint>
#    include "constants.h"

struct RxData_T
{
  private:
    uint8_t begin[ 3 ] = { 'b', 'g', 'n' };

  public:
    uint16_t frame[ WIDTH * HEIGHT ];
    uint16_t x : 8;
    uint16_t cameraEnable : 1;
    uint16_t y : 7;
    uint8_t avgLuminance;
    uint8_t aec;
    uint32_t time;

  private:
    uint8_t end[ 3 ] = { 'e', 'n', 'd' };
};

enum TxCommand : uint16_t
{
    noCommand  = 0,
    newXoffset = 1,
    newYoffset = 2,
    newAec     = 3,
    takeShoot  = 4,
    aimingMode = 5,
};

struct TxData_T
{
    uint16_t cameraEnabled : 1;
    TxCommand command : 3;
    uint16_t additionalData : 12;
};
#endif // DATATYPES_H