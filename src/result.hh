#pragma once
#include "types.hh"

class Result {
    private:
        u8 status;
        u32 address;
        u32 time;
    public:
        Result();
        Result(u8);
        Result(u8, u32);
        u8 get_status();
        u32 get_address();
        u32 get_time();
        void add_time(u32);
};
