#pragma once
#include "block.hh"
#include "types.hh"

class Unit {
    private:
        // Configuration properties
        u8 level;
        u8 write_hit_policy;
        u8 write_miss_policy;
        u16 block_size;
        u16 set_count;
        u16 way;
        u32 hit_time;
        u32 size;
        // Access properties
        u32 hits;
        u32 misses;
        // Cache types
        Deque<Block> mmap;
        HashMap<u32, Block> dmap;
        HashMap<u32, Deque<Block>> nmap;
        // Access methods
        bool access(bool, u32);
        bool access_mmap(bool, u32, u32);
        bool access_dmap(bool, u32, u32);
        bool access_nmap(bool, u32, u32);
        // Configuration methods
        void set_level(String&);
        void set_write_hit_policy(String&);
        void set_write_miss_policy(String&);
        void set_block_size(String&);
        void set_set_count();
        void set_way(String&);
        void set_hit_time(String&);
        void set_size(String&);
    public:
        Unit();
        bool load(u32);
        bool store(u32);
        bool is_valid();
        u8 get_level();
        u8 get_write_hit_policy();
        u8 get_write_miss_policy();
        void set(String&, String&);
        bool operator<(Unit&);        
};
