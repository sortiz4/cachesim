#include "result.hh"
#include "types.hh"

Result::Result() {
    this->status = 0xff;
    this->address = 0xffffffff;
}

Result::Result(u8 status) {
    this->status = status;
    this->address = 0xffffffff;
}

Result::Result(u8 status, u32 address) {
    this->status = status;
    this->address = address;
}

u8 Result::get_status() {
    return this->status;
}

u32 Result::get_address() {
    return this->address;
}
