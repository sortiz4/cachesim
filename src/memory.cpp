#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>
#include "chars.hh"
#include "consts.hh"
#include "exceptions.hh"
#include "memory.hh"
#include "text.hh"
#include "types.hh"
#include "unit.hh"
using namespace std;

Memory::Memory() {
    this->hierarchy = Vector<Unit>();
}

void Memory::conf(String &path) {
    // Load the configuration
    auto file = FileReader(path);
    if(file) {
        String key, value;
        auto buffer = String();
        auto unit = Unit();
        while(!file.eof()) {
            auto c = chars::normalize(file.get());
            if(c == chars::LF || file.eof()) {
                if(key.length() > 0) {
                    // The end of a key-value pair has been reached
                    value = buffer;
                    buffer.clear();
                    // Evaluate the key-value pair
                    try {
                        unit.set(key, value);
                    } catch(FormatException &e) {
                        auto sb = StringBuilder();
                        sb << e.what() << " in '" << path << "'";
                        throw FormatException(sb.str());
                    }
                    key.clear();
                    value.clear();
                }
            } else if(c == chars::COLON) {
                // The end of a key has been reached
                if(buffer.compare(text::LEVEL) == 0 && unit.is_valid()) {
                    this->hierarchy.push_back(unit);
                    unit = Unit();
                }
                key = buffer;
                buffer.clear();
            } else if(chars::is_alphanum(c)) {
                // Push alphabetic characters only
                buffer.push_back(c);
            }
        }
        this->hierarchy.push_back(unit);
        file.close();
    } else {
        auto sb = StringBuilder();
        sb << "'" << path << "' could not be opened";
        throw IoException(sb.str());
    }

    // Sort the hierarchy
    sort(this->hierarchy.begin(), this->hierarchy.end());
}

void Memory::access(String &path) {
    auto file = FileReader(path);
    if(file) {
        String instr, addr;
        while(!file.eof()) {
            file >> instr >> addr;
            chars::normalize(instr);
            if(instr.length() > 0) {
                // Evaluate the instruction
                try {
                    this->exec(instr, addr);
                } catch(FormatException &e) {
                    auto sb = StringBuilder();
                    sb << e.what() <<" in '" << path << "'";
                    throw FormatException(sb.str());
                }
                // Clear the buffers
                instr.clear();
                addr.clear();
            }
        }
        file.close();
    } else {
        auto sb = StringBuilder();
        sb << "'" << path << "' could not be opened";
        throw IoException(sb.str());
    }
}

void Memory::score() {
    auto length = this->hierarchy.size();
    for(auto i = 0; i < length; i++) {
        auto &unit = this->hierarchy[i];
        if(unit.get_level() == consts::MAIN) {
            cout << "Level: " << "Main" << endl;
        } else {
            cout << "Level: " << (u32)unit.get_level() << endl;
        }
        cout << "Hits: " << unit.get_hits() << endl;
        cout << "Misses: " << unit.get_misses() << endl;
        cout << "Total: " << unit.get_hits() + unit.get_misses() << endl;
        cout << "AccessTime: " << "???" << endl;
        if(i < length - 1) {
            cout << endl;
        }
    }
}

void Memory::exec(String &instr, String &saddr) {
    u32 uaddr = 0;

    // Parse the address
    try {
        uaddr = (u32)stoul(saddr);
    } catch(Exception &e) {
        throw FormatException("'address' could not be parsed");
    }

    // Parse the instruction
    if(instr.compare(text::LOAD) == 0) {
        this->load(uaddr, 0);
    } else if(instr.compare(text::STORE) == 0) {
        this->store(uaddr, 0);
    } else {
        throw FormatException("unrecognized instruction");
    }
}

void Memory::load(u32 addr, u32 from) {
    // Load instruction
    for(auto i = from; i < this->hierarchy.size(); i++) {
        auto &unit = this->hierarchy[i];
        auto result = unit.load(addr);
        if(result.get_status() == consts::HIT) {
            // Read hit stops immediately
            break;
        } else if(result.get_status() == consts::DIRTY) {
            // Dirty blocks must be written through before eviction
            this->store(result.get_address(), i + 1);
            i -= 1;
        }
    }
}

void Memory::store(u32 addr, u32 from) {
    // Store instruction
    for(auto i = from; i < this->hierarchy.size(); i++) {
        auto &unit = this->hierarchy[i];
        auto result = unit.store(addr);
        if(result.get_status() == consts::HIT) {
            if(unit.get_write_hit_policy() == consts::WRITE_BACK) {
                // Write back stops immediately
                break;
            }
        } else if(result.get_status() == consts::MISS) {
            if(unit.get_write_miss_policy() == consts::WRITE_ALLOCATE_ON) {
                // Write allocation must load the block before continuing
                this->load(addr, i);
                i -= 1;
            }
        }
    }
}
