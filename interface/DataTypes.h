#pragma once
#ifndef DATATYPES_H
#    define DATATYPES_H
#    include <cstdint>
#    include "constants.h"

// #    pragma pack( 1 )
struct RxData_T
{
  private:
    uint32_t begin : 24 { 0b011011100110011101100010 };

  public:
    uint32_t : 1;                        // space for future use
    uint32_t cameraEnable : 1 { 0 };     //
    uint32_t avgLuminance : 6 { 0 };     // 0-63
    uint16_t frame[ WIDTH * HEIGHT ] {}; //

    uint32_t time { 0 }; //

    uint64_t : 3; // space for future use
    uint64_t aec : 16 { 0 };
    uint64_t x : 11 { 0 }; // in range 0-2047
    uint64_t y : 10 { 0 }; // in range 0-1023

  private:
    uint64_t end : 24 { 0b011001000110111001100101 };
};
// #    pragma pack()

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