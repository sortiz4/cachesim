#pragma once
#include "types.hh"

class Block {
    private:
        u32 address;
        u32 tag;
        bool dirty;

    public:
        Block();
        Block(u32, u32);
        u32 get_tag() const;
        u32 get_address() const;
        bool get_dirty() const;
        void set_dirty(bool);
        bool operator==(const u32&);
        bool operator==(const Block&);
};
