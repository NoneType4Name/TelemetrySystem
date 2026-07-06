#pragma once
#ifndef DATATYPES_H
#    define DATATYPES_H
#    include <cstdint>
#    include "constants.h"

struct RxData_T
{
  private:
    uint8_t begin[ 4 ] = { 'b', 'g', 'n', '\0' };

  public:
    uint16_t frame[ WIDTH * HEIGHT ] {};
    uint16_t x : 8 { 0 };
    uint16_t cameraEnable : 1 { 0 };
    uint16_t y : 7 { 0 };
    uint8_t avgLuminance { 0 };
    uint8_t aec { 0 };
    uint32_t time { 0 };

  private:
    uint8_t end[ 4 ] = { 'e', 'n', 'd', '\0' };
};

enum TxCommand : uint16_t
{
    newXoffset = 1,
    newYoffset = 2,
    newAec     = 3,
    takeShoot  = 4,
    aimingMode = 5,
    noCommand  = 6
};

struct TxData_T
{
    uint16_t cameraEnabled : 1;
    TxCommand command : 3;
    uint16_t additionalData : 12;
};
#endif // DATATYPES_H