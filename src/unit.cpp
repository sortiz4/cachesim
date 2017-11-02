#include <string>
#include "chars.hh"
#include "consts.hh"
#include "exceptions.hh"
#include "text.hh"
#include "types.hh"
#include "unit.hh"

Unit::Unit() {
    this->level = 0;
    this->line = 0;
    this->way = 0;
    this->write_policy = 0;
    this->alloc_policy = 0;
    this->size = 0;
    this->hit_time = 0;
}

bool Unit::is_valid() {
    if(this->level > 0) {
        return true;
    }
    return false;
}

void Unit::set(String &key, String &value) {
    if(key.compare(text::LEVEL) == 0) {
        // Evaluate the level
        this->set_level(value);
    } else if(key.compare(text::LINE) == 0) {
        // Evaluate the line
        this->set_line(value);
    } else if(key.compare(text::WAY) == 0) {
        // Evaluate the way
        this->set_way(value);
    } else if(key.compare(text::SIZE) == 0) {
        // Evaluate the size
        this->set_size(value);
    } else if(key.compare(text::HIT_TIME) == 0) {
        // Evaluate the hit time
        this->set_hit_time(value);
    } else if(key.compare(text::WRITE_POLICY) == 0) {
        // Evaluate the write policy
        this->set_write_policy(value);
    } else if(key.compare(text::ALLOC_POLICY) == 0) {
        // Evaluate the allocation policy
        this->set_alloc_policy(value);
    } else {
        throw FormatException("unrecognized key");
    }
}

void Unit::set_level(String &value) {
    if(value.compare(text::LEVEL_MAIN) == 0) {
        this->level = consts::LEVEL_MAIN;
    } else {
        try {
            this->level = stoul(value.substr(1, value.length()));
        } catch(Exception &e) {
            throw FormatException("'level' could not be parsed");        
        }
    }
}

void Unit::set_line(String &value) {
    try {
        this->line = (u8)stoul(value);
    } catch(Exception &e) {
        throw FormatException("'line' could not be parsed");        
    }
}

void Unit::set_way(String &value) {
    try {
        this->way = (u8)stoul(value);
    } catch(Exception &e) {
        throw FormatException("'way' could not be parsed");        
    }
}

void Unit::set_size(String &value) {
    u32 multipliers[] = {1, (u32)1e3, (u32)1e6, (u32)1e9};
    auto suffix = value[value.length() - 1];
    auto multiplier = multipliers[0];
    // Check the size multiplier (only supports 'K', 'M', 'G')
    if(suffix == chars::UPPER_K) {
        multiplier = multipliers[1];
        value.pop_back();
    } else if(suffix == chars::UPPER_M) {
        multiplier = multipliers[2];
        value.pop_back();
    } else if(suffix == chars::UPPER_G) {
        multiplier = multipliers[3];
        value.pop_back();
    }
    try {
        this->size = (u32)stoul(value) * multiplier;
    } catch(Exception &e) {
        throw FormatException("'size' could not be parsed");        
    }
}

void Unit::set_hit_time(String &value) {
    try {
        this->hit_time = (u32)stoul(value);
    } catch(Exception &e) {
        throw FormatException("'hit time' could not be parsed");        
    }
}

void Unit::set_write_policy(String &value) {
    if(value.compare(text::WRITE_BACK) == 0) {
        this->write_policy = consts::WRITE_BACK;
    } else if(value.compare(text::WRITE_THROUGH) == 0) {
        this->write_policy = consts::WRITE_THROUGH;
    } else {
        throw FormatException("unrecognized write policy");
    }
}

void Unit::set_alloc_policy(String &value) {
    if(value.compare(text::WRITE_ALLOCATE_ON) == 0) {
        this->alloc_policy = consts::WRITE_ALLOCATE_ON;
    } else if(value.compare(text::WRITE_ALLOCATE_OFF) == 0) {
        this->alloc_policy = consts::WRITE_ALLOCATE_OFF;
    } else {
        throw FormatException("unrecognized allocation policy");
    }
}
