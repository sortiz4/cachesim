#pragma once
#include "block.hh"
#include "result.hh"
#include "types.hh"

class Unit {
    private:
        // Configuration properties
        u8 level;
        u8 write_hit_policy;
        u8 write_miss_policy;
        u16 block_size;
        u16 way;
        u32 hit_time;
        u32 set_count;
        u32 size;
        bool full;

        // Access properties
        u32 access_time;
        u32 hit_count;
        u32 miss_count;
        Unit *next;

        // Cache types
        Deque<Block> mmap;
        HashMap<u32, Block> dmap;
        HashMap<u32, Deque<Block>> nmap;

        // Access methods
        Result access(bool, u32);
        Result access_mmap(bool, u32, u32, u32);
        Result access_dmap(bool, u32, u32, u32);
        Result access_nmap(bool, u32, u32, u32);

        // Configuration methods
        void set_level(String&);
        void set_write_hit_policy(String&);
        void set_write_miss_policy(String&);
        void set_block_size(String&);
        void set_way(String&);
        void set_hit_time(String&);
        void set_size(String&);

    public:
        Unit();
        ~Unit();
        Result load(u32);
        Result store(u32);
        bool is_valid();
        void score();
        void finalize();
        void add_unit(Unit*);
        void set(String&, String&);
        bool operator<(Unit&);
};
