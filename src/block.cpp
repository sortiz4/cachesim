#include "block.hh"
#include "types.hh"

Block::Block() {
    this->address = 0xffffffff;
    this->tag = 0xffffffff;
    this->dirty = true;
}

Block::Block(u32 address, u32 tag) {
    this->address = address;
    this->tag = tag;
    this->dirty = false;
}

u32 Block::get_tag() const {
    return this->tag;
}

u32 Block::get_address() const {
    return this->address;
}

bool Block::get_dirty() const {
    return this->dirty;
}

void Block::set_dirty(bool dirty) {
    this->dirty = dirty;
}

bool Block::operator==(const u32 &rhs) {
    return this->tag == rhs;
}

bool Block::operator==(const Block &rhs) {
    return this->tag == rhs.get_tag();
}
