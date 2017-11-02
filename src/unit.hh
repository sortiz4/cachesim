#pragma once
#include "types.hh"

class Unit {
    private:
        u8 level;
        u8 line;
        u8 way;
        u8 write_policy;
        u8 alloc_policy;
        u32 size;
        u32 hit_time;
        void set_level(String&);
        void set_line(String&);
        void set_way(String&);
        void set_size(String&);
        void set_hit_time(String&);
        void set_write_policy(String&);
        void set_alloc_policy(String&);
    public:
        Unit();
        bool is_valid();
        void set(String&, String&);
};
