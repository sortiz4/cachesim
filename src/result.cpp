#include "result.hh"
#include "types.hh"

Result::Result() {
    this->status = 0xff;
    this->address = 0xffffffff;
    this->time = 0;
}

Result::Result(u8 status) {
    this->status = status;
    this->address = 0xffffffff;
    this->time = 0;
}

Result::Result(u8 status, u32 address) {
    this->status = status;
    this->address = address;
    this->time = 0;
}

u8 Result::get_status() {
    return this->status;
}

u32 Result::get_address() {
    return this->address;
}

u32 Result::get_time() {
    return this->time;
}

void Result::add_time(u32 time) {
    this->time += time;
}
