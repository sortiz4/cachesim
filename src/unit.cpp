#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "block.hh"
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
    this->write_hit_policy = 0;
    this->write_miss_policy = 0;
    this->block_size = 0;
    this->set_count = 0;
    this->way = 0;
    this->hit_time = 0;
    this->size = 0;
    // Access properties
    this->hits = 0;
    this->misses = 0;
    // Cache types
    this->mmap = Deque<Block>();
    this->dmap = HashMap<u32, Block>();
    this->nmap = HashMap<u32, Deque<Block>>();
}

bool Unit::load(u32 addr) {
    bool hit = access(false, addr);
    if(hit) {
        this->hits += 1;
        cerr << "load: level: " << (u16)this->get_level() << ", hit: " << addr << endl;
    } else {
        this->misses += 1;
        cerr << "load: level: " << (u16)this->get_level() << ", miss: " << addr << endl;
    }
    return hit;
}

bool Unit::store(u32 addr) {
    // TODO: Implement store
    // need three states
        // hit - cache read/write hit, no dirty state
        // miss - cache read/write miss, no dirty state
        // dirty - cache dirty state - trigger additional store
    // if writing
        // Need to check dirty bit on hit and eviction
        // On write hit
            // write back - set dirty bit and continue
            // write through - write to lower cache level - continue (as if loading)
        // On write miss
            // write alloc on - value will be written to cache level - continue (as if loading)
            // write allocate off - value will not be written to cache level - do nothing
    return false;
}

bool Unit::access(bool store, u32 addr) {
    // Main memory always hits
    if(this->level == consts::MAIN) {
        return true;
    }

    // Compute bit widths
    u32 setw = log2(this->set_count);
    u32 offsw = log2(this->block_size);
    // Compute bit masks
    u32 tagm = 0xffffffff << (offsw + setw);
    u32 setm = ~tagm & (0xffffffff << offsw);
    // Break apart address
    u32 tag = tagm & addr;
    u32 set = setm & addr;

    // Access the appropriate cache
    if(this->way == 1) {
        return this->access_dmap(store, tag, set);
    } else if(this->way != this->set_count) {
        return this->access_mmap(store, tag, set);
    }
    return this->access_nmap(store, tag, set);
}

bool Unit::access_mmap(bool store, u32 tag, u32 set) {
    // Fully associative cache algorithm (LRU)
    auto index = find(this->mmap.begin(), this->mmap.end(), tag);
    if(index == this->mmap.end()) {
        // The tag could not be found (miss)
        if(this->mmap.size() == this->set_count) {
            // The cache doesn't have room
            this->mmap.pop_back();
        }
        this->mmap.push_front(Block(tag));
        return false;
    }
    // The tag was found (hit - move to front)
    auto value = *index;
    this->mmap.erase(index);
    this->mmap.push_front(value);
    return true;
}

bool Unit::access_dmap(bool store, u32 tag, u32 set) {
    // Direct mapped cache algorithm
    if(this->dmap.find(set) == this->dmap.end()) {
        // The line is invalid (compulsory miss)
        this->dmap[set] = Block(tag);
        return false;
    }
    // The line is valid and the tag matches (hit)
    if(this->dmap[set] == tag) {
        return true;
    }
    // The line is valid but the tag does not match (miss)
    this->dmap[set] = Block(tag);
    return false;
}

bool Unit::access_nmap(bool store, u32 tag, u32 set) {
    // Set associative cache algorithm (LRU)
    auto set_index = this->nmap.find(set);
    if(set_index == this->nmap.end()) {
        // The line is invalid (compulsory miss)
        auto deque = Deque<Block>(this->way);
        deque.push_front(Block(tag));
        this->nmap[set] = deque;
        return false;
    }
    // The line is valid
    auto deque = set_index->second;
    auto block_index = find(deque.begin(), deque.end(), tag);
    if(block_index == deque.end()) {
        // The tag could not be found (miss)
        if(deque.size() == this->way) {
            // The set doesn't have room
            deque.pop_back();
        }
        deque.push_front(Block(tag));
        return false;
    }
    // The tag was found (hit - move to front)
    auto value = *block_index;
    deque.erase(block_index);
    deque.push_front(value);
    return true;
}

bool Unit::is_valid() {
    if(this->level == consts::MAIN) {
        return true;
    }
    return this->level > 0
        && this->write_hit_policy > 0
        && this->write_miss_policy > 0
        && this->block_size > 0
        && this->set_count > 0
        && this->way > 0
        && this->size > 0;
}

u8 Unit::get_level() {
    return this->level;
}

u8 Unit::get_write_hit_policy() {
    return this->write_hit_policy;
}

u8 Unit::get_write_miss_policy() {
    return this->write_miss_policy;
}

void Unit::set(String &key, String &value) {
    if(key.compare(text::LEVEL) == 0) {
        // Evaluate the level
        this->set_level(value);
    } else if(key.compare(text::LINE) == 0) {
        // Evaluate the block size
        this->set_block_size(value);
    } else if(key.compare(text::WAY) == 0) {
        // Evaluate the associativity
        this->set_way(value);
    } else if(key.compare(text::SIZE) == 0) {
        // Evaluate the cache size
        this->set_size(value);
    } else if(key.compare(text::HIT_TIME) == 0) {
        // Evaluate the hit time
        this->set_hit_time(value);
    } else if(key.compare(text::WRITE_POLICY) == 0) {
        // Evaluate the write hit policy
        this->set_write_hit_policy(value);
    } else if(key.compare(text::ALLOC_POLICY) == 0) {
        // Evaluate the write miss policy
        this->set_write_miss_policy(value);
    } else {
        throw FormatException("unrecognized key");
    }
}

void Unit::set_level(String &value) {
    if(value.compare(text::MAIN) == 0) {
        this->level = consts::MAIN;
    } else {
        try {
            this->level = stoul(value.substr(1, value.length()));
        } catch(Exception &e) {
            throw FormatException("'level' could not be parsed");        
        }
    }
}

void Unit::set_write_hit_policy(String &value) {
    if(value.compare(text::WRITE_BACK) == 0) {
        this->write_hit_policy = consts::WRITE_BACK;
    } else if(value.compare(text::WRITE_THROUGH) == 0) {
        this->write_hit_policy = consts::WRITE_THROUGH;
    } else {
        throw FormatException("unrecognized write policy");
    }
}

void Unit::set_write_miss_policy(String &value) {
    if(value.compare(text::WRITE_ALLOCATE_ON) == 0) {
        this->write_miss_policy = consts::WRITE_ALLOCATE_ON;
    } else if(value.compare(text::WRITE_ALLOCATE_OFF) == 0) {
        this->write_miss_policy = consts::WRITE_ALLOCATE_OFF;
    } else {
        throw FormatException("unrecognized allocation policy");
    }
}

void Unit::set_block_size(String &value) {
    try {
        this->block_size = (u16)stoul(value);
        this->set_set_count();
    } catch(Exception &e) {
        throw FormatException("'line' could not be parsed");        
    }
}

void Unit::set_set_count() {
    if(this->block_size != 0 && this->way != 0) {
        this->set_count = this->size / this->block_size / this->way;
    }
}

void Unit::set_way(String &value) {
    try {
        this->way = (u16)stoul(value);
        this->set_set_count();
    } catch(Exception &e) {
        throw FormatException("'way' could not be parsed");        
    }
}

void Unit::set_hit_time(String &value) {
    try {
        this->hit_time = (u32)stoul(value);
    } catch(Exception &e) {
        throw FormatException("'hit time' could not be parsed");        
    }
}

void Unit::set_size(String &value) {
    u32 multipliers[] = {1, 1024, 1024*1024, 1024*1024*1024};
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
        this->set_set_count();
    } catch(Exception &e) {
        throw FormatException("'size' could not be parsed");        
    }
}

bool Unit::operator<(Unit &rhs) {
    return this->level < rhs.level;
}
