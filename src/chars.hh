#pragma once
#include "types.hh"

namespace chars {
    const char LF = 0x0A;
    const char COLON = 0x3A;
    const char NUM_0 = 0x30;
    const char NUM_9 = 0x39;
    const char UPPER_A = 0x41;
    const char UPPER_G = 0x47;
    const char UPPER_K = 0x4B;
    const char UPPER_M = 0x4D;
    const char UPPER_Z = 0x5A;    
    const char LOWER_A = 0x61;
    const char LOWER_Z = 0x7A;
    bool is_alphanum(char);
    char normalize(char);
    void normalize(String&);
}
