#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "chars.hh"
#include "consts.hh"
#include "exceptions.hh"
#include "text.hh"
#include "types.hh"
#include "unit.hh"
using namespace std;

Unit::Unit() {
    // Configuration properties
    this->level = 0;
    this->write_policy = 0;
    this->alloc_policy = 0;
    this->line_count = 0;
    this->line_size = 0;
    this->way = 0;
    this->size = 0;
    this->hit_time = 0;
    // Access properties
    this->hits = 0;
    this->misses = 0;
    this->time = 0;
    // Cache types
    this->dmap = HashMap<u32, u32>();
    this->nmap = HashMap<u32, Deque<u32>>();
    this->mmap = Deque<u32>();
}

bool Unit::is_valid() {
    if(this->level > 0) {
        return true;
    }
    return false;
}

bool Unit::load(u32 addr) {
    // Compute bit widths
    u32 setw = (u32)log2(this->line_count);
    u32 offsw = (u32)log2(this->line_size);
    // Compute bit masks
    u32 tagm = 0xffffffff << (offsw + setw);
    u32 setm = ~tagm & (0xffffffff << offsw);
    // Break apart address
    u32 tag = tagm & addr;
    u32 set = setm & addr;
    // Access cache
    bool hit = access(tag, set);
    // Score keeping
    if(hit) {
        this->hits += 1;
    } else {
        this->misses += 1;
    }
    this->time += this->hit_time;
    return hit;
}

bool Unit::access(u32 tag, u32 set) {
    if(this->level == consts::LEVEL_MAIN) {
        // Main memory always hits
        return true;
    } else if(this->way == 0) {
        // Direct-mapped cache
        return this->access_dmap(tag, set);
    } else if(this->way == this->line_count) {
        // Fully associative cache (LRU)
        return this->access_mmap(tag, set);
    }
    // N-way associative cache (LRU)
    return this->access_nmap(tag, set);
}

bool Unit::access_dmap(u32 tag, u32 set) {
    // Direct-mapped cache
    if(this->dmap.find(set) == this->dmap.end()) {
        // The line is invalid (compulsory miss)
        this->dmap[set] = tag;
        return false;
    }
    // The line is valid
    if(this->dmap[set] == tag) {
        // Cache hit
        return true;
    }
    // Cache miss
    this->dmap[set] = tag;
    return false;
}

bool Unit::access_nmap(u32 tag, u32 set) {
    // N-way associative cache (LRU)
    auto itera = this->nmap.find(set);
    if(itera == this->nmap.end()) {
        // The line is invalid (compulsory miss)
        auto deque = Deque<u32>(this->way);
        deque.push_front(tag);
        this->nmap[set] = deque;
        return false;
    }
    // The line is valid
    auto deque = itera->second;
    auto iterb = find(deque.begin(), deque.end(), tag);
    if(iterb == deque.end()) {
        // The tag could not be found (miss)
        if(deque.size() < this->way) {
            // The set has room
            deque.push_front(tag);
        } else {
            // The set doesn't have room
            deque.pop_back();
            deque.push_front(tag);
        }
        return false;
    }
    // The tag was found (move to front)
    auto value = *iterb;
    deque.erase(iterb);
    deque.push_front(value);
    return true;
}

bool Unit::access_mmap(u32 tag, u32 set) {
    // Fully associative cache (LRU)
    auto iter = find(this->mmap.begin(), this->mmap.end(), tag);
    if(iter == this->mmap.end()) {
        // The tag could not be found (miss)
        if(this->mmap.size() < this->line_count) {
            // The cache has room
            this->mmap.push_front(tag);
        } else {
            // The cache doesn't have room
            this->mmap.pop_back();
            this->mmap.push_front(tag);
        }
        return false;
    }
    // The tag was found (move to front)
    auto value = *iter;
    this->mmap.erase(iter);
    this->mmap.push_front(value);
    return true;
}

void Unit::set(String &key, String &value) {
    if(key.compare(text::LEVEL) == 0) {
        // Evaluate the level
        this->set_level(value);
    } else if(key.compare(text::LINE) == 0) {
        // Evaluate the line size
        this->set_line_size(value);
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

void Unit::set_line_size(String &value) {
    try {
        this->line_size = (u16)stoul(value);
        if(this->line_size != 0) {
            this->line_count = this->size / this->line_size;
        }
    } catch(Exception &e) {
        throw FormatException("'line' could not be parsed");        
    }
}

void Unit::set_way(String &value) {
    try {
        this->way = (u16)stoul(value);
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
        if(this->line_size != 0) {
            this->line_count = this->size / this->line_size;
        }
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

bool Unit::operator<(Unit &rhs) {
    if(this->level < rhs.level) {
        return true;
    }
    return false;
}
