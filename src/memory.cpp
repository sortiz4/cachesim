#include <string>
#include "chars.hh"
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
                        sb << e.what() <<" in '" << path << "'";
                        throw FormatException(sb.str());
                    }
                    key.clear();
                    value.clear();
                }
            } else if(c == chars::COLON) {
                // The end of a key has been reached
                if(unit.is_valid() && buffer.compare(text::LEVEL) == 0) {
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

void Memory::exec(String &instr, String &addr) {
    u32 uaddr = 0;
    // Parse the address
    try {
        uaddr = (u32)stoul(addr);
    } catch(Exception &e) {
        throw FormatException("'address' could not be parsed");        
    }
    // Parse the instruction
    if(instr.compare(text::LOAD) == 0) {
        this->load(uaddr);
    } else if(instr.compare(text::STORE) == 0) {
        this->store(uaddr);        
    } else {
        throw FormatException("unrecognized instruction");        
    }
}

void Memory::load(u32) {
    // Load instruction
}

void Memory::store(u32) {
    // Store instruction
}
