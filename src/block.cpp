#include "block.hh"
#include "types.hh"

Block::Block() {
    this->tag = 0;
    this->dirty = false;
}

Block::Block(u32 tag) {
    this->tag = tag;
    this->dirty = false;
}

u32 Block::get_tag() const {
    return this->tag;
}

void Block::set_tag(u32 tag) {
    this->tag = tag;
}

bool Block::get_dirty() const {
    return this->dirty;
}

void Block::set_dirty() {
    this->dirty = true;
}

bool Block::operator==(const u32 &rhs) {
    return this->tag == rhs;
}

bool Block::operator==(const Block &rhs) {
    return this->tag == rhs.get_tag();
}
