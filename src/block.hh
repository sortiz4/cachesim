#pragma once
#include "types.hh"

class Block {
    private:
        u32 tag;
        bool dirty;
    public:
        Block();
        Block(u32);
        u32 get_tag() const;
        void set_tag(u32);
        bool get_dirty() const;
        void set_dirty();
        bool operator==(const u32&);
        bool operator==(const Block&);
};
