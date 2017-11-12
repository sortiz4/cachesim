#pragma once
#include "types.hh"

class Unit {
    private:
        // Configuration properties
        u8 level;
        u8 write_policy;
        u8 alloc_policy;
        u16 line_count;
        u16 line_size;
        u16 way;
        u32 hit_time;
        u32 size;
        // Access properties
        u32 hits;
        u32 misses;
        u32 time;
        // Cache types
        HashMap<u32, u32> dmap;
        HashMap<u32, Deque<u32>> nmap;
        Deque<u32> mmap;
        // Access methods
        bool access(u32, u32);
        bool access_dmap(u32, u32);
        bool access_nmap(u32, u32);
        bool access_mmap(u32, u32);
        // Configuration methods
        void set_level(String&);
        void set_line_size(String&);
        void set_way(String&);
        void set_size(String&);
        void set_hit_time(String&);
        void set_write_policy(String&);
        void set_alloc_policy(String&);
    public:
        Unit();
        bool is_valid();
        bool load(u32);
        void set(String&, String&);
        bool operator<(Unit&);        
};
