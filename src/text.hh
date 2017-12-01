#pragma once
#include "types.hh"

namespace text {
    // Known configuration keys
    const String LEVEL = "LEVEL";
    const String LINE = "LINE";
    const String WAY = "WAY";
    const String FULL = "FULL";
    const String SIZE = "SIZE";
    const String HIT_TIME = "HITTIME";
    const String WRITE_POLICY = "WRITEPOLICY";
    const String ALLOC_POLICY = "ALLOCATIONPOLICY";

    // Known configuration values
    const String MAIN = "MAIN";
    const String WRITE_BACK = "WRITEBACK";
    const String WRITE_THROUGH = "WRITETHROUGH";
    const String WRITE_ALLOCATE_ON = "WRITEALLOCATE";
    const String WRITE_ALLOCATE_OFF = "NOWRITEALLOCATE";

    // Known instructions
    const String LOAD = "LD";
    const String STORE = "ST";
}
