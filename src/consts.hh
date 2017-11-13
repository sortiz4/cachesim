#pragma once
#include "types.hh"

namespace consts {
    // Configuration values
    const u8 MAIN = 0xFF;
    const u8 WRITE_BACK = 0x01;
    const u8 WRITE_THROUGH = 0x02;
    const u8 WRITE_ALLOCATE_ON = 0x01;
    const u8 WRITE_ALLOCATE_OFF = 0x02;
    // Access states
    const u8 HIT = 0x01;
    const u8 MISS = 0x02;
    const u8 DIRTY = 0x03;
}
