#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include "block.hh"
#include "chars.hh"
#include "consts.hh"
#include "exceptions.hh"
#include "result.hh"
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
    this->way = 0;
    this->set_count = 0;
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

Result Unit::load(u32 addr) {
    // NOTE: Load events are handled by the controller. A load hit will stop
    // immediately. A load miss will continue until the block is found. A dirty
    // event occurs when a dirty block is being evicted - this will trigger a
    // store of that block to the next level.
    auto result = access(false, addr);
    if(result.get_status() == consts::HIT) {
        this->hits += 1;
        // cerr << "load: level: " << (u16)this->level << ", hit: " << addr << endl;
    } else if(result.get_status() == consts::MISS) {
        this->misses += 1;
        // cerr << "load: level: " << (u16)this->level << ", miss: " << addr << endl;
    } else if(result.get_status() == consts::DIRTY) {
        this->misses += 1;
        // cerr << "load: level: " << (u16)this->level << ", dirty: " << addr << endl;
    }
    return result;
}

Result Unit::store(u32 addr) {
    // NOTE: Write miss events are handled by the controller. With write
    // allocate, a load is triggered followed by a write hit. Without write
    // allocate, nothing is written to the cache.
    //
    // NOTE: Write hit events are split into two categories. Write back behaves
    // like a load hit but a dirty bit is set. Write through behaves like a
    // load hit.
    auto result = access(true, addr);
    if(result.get_status() == consts::HIT) {
        this->hits += 1;
        // cerr << "store: level: " << (u16)this->level << ", hit: " << addr << endl;
    } else if(result.get_status() == consts::MISS) {
        this->misses += 1;
        // cerr << "store: level: " << (u16)this->level << ", miss: " << addr << endl;
    } else if(result.get_status() == consts::DIRTY) {
        this->misses += 1;
        // cerr << "store: level: " << (u16)this->level << ", dirty: " << addr << endl;
    }
    return result;
}

Result Unit::access(bool store, u32 addr) {
    // Main memory always hits
    if(this->level == consts::MAIN) {
        return Result(consts::HIT);
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
        return this->access_dmap(store, addr, tag, set);
    } else if(this->way == this->set_count) {
        return this->access_mmap(store, addr, tag, set);
    }
    return this->access_nmap(store, addr, tag, set);
}

Result Unit::access_mmap(bool store, u32 addr, u32 tag, u32 set) {
    // Fully associative cache algorithm (LRU)
    auto index = find(this->mmap.begin(), this->mmap.end(), tag);
    if(index == this->mmap.end()) {
        // The tag could not be found (miss)
        if(!store) {
            if(this->mmap.size() == this->set_count) {
                // The cache doesn't have room (eviction)
                auto &block = this->mmap[this->mmap.size() - 1];
                if(block.get_dirty()) {
                    // The block is dirty so the controller must
                    // write this block to the next memory unit
                    // and restart the current operation
                    block.set_dirty(false);
                    return Result(consts::DIRTY, block.get_address());
                }
                // Read only: pop the last block
                this->mmap.pop_back();
            }
            // Read only: push the new block
            this->mmap.push_front(Block(addr, tag));
        }
        return Result(consts::MISS);
    }

    // The tag was found (hit + move to front)
    auto value = *index;
    if(store && this->write_hit_policy == consts::WRITE_BACK) {
        // Write only: set the dirty bit on write hit + write back
        value.set_dirty(true);
    }

    // Read + write: move the block to the front
    this->mmap.erase(index);
    this->mmap.push_front(value);
    return Result(consts::HIT);
}

Result Unit::access_dmap(bool store, u32 addr, u32 tag, u32 set) {
    // Direct mapped cache algorithm
    if(this->dmap.find(set) == this->dmap.end()) {
        // The block is invalid (compulsory miss)
        if(!store) {
            // Read only: 'load' the block
            this->dmap[set] = Block(addr, tag);
        }
        return Result(consts::MISS);
    }

    // The block is valid and the tags match (hit)
    if(this->dmap[set] == tag) {
        if(store && this->write_hit_policy == consts::WRITE_BACK) {
            // Write only: set the dirty bit on write hit + write back
            this->dmap[set].set_dirty(true);
        }
        return Result(consts::HIT);
    }

    // The block is valid but the tags do not match (miss + eviction)
    if(!store) {
        if(this->dmap[set].get_dirty()) {
            // The block is dirty so the controller must
            // write this block to the next memory unit
            // and restart the current operation
            this->dmap[set].set_dirty(false);
            return Result(consts::DIRTY, this->dmap[set].get_address());
        }
        // Read only: 'load' the block
        this->dmap[set] = Block(addr, tag);
    }
    return Result(consts::MISS);
}

Result Unit::access_nmap(bool store, u32 addr, u32 tag, u32 set) {
    // Set associative cache algorithm (LRU)
    auto set_index = this->nmap.find(set);
    if(set_index == this->nmap.end()) {
        // The line is invalid (compulsory miss)
        if(!store) {
            // Read only: 'load' the block
            auto deque = Deque<Block>();
            deque.push_front(Block(addr, tag));
            this->nmap[set] = deque;
        }
        return Result(consts::MISS);
    }

    // The line is valid
    auto &deque = set_index->second;
    auto block_index = find(deque.begin(), deque.end(), tag);
    if(block_index == deque.end()) {
        // The tag could not be found (miss)
        if(!store) {
            if(deque.size() == this->way) {
                // The set doesn't have room (eviction)
                auto &block = deque[deque.size() - 1];
                if(block.get_dirty()) {
                    // The block is dirty so the controller must
                    // write this block to the next memory unit
                    // and restart the current operation
                    block.set_dirty(false);
                    return Result(consts::DIRTY, block.get_address());
                }
                // Read only: pop the last block
                deque.pop_back();
            }
            // Read only: push the new block
            deque.push_front(Block(addr, tag));
        }
        return Result(consts::MISS);
    }

    // The tag was found (hit + move to front)
    auto value = *block_index;
    if(store && this->write_hit_policy == consts::WRITE_BACK) {
        // Write only: set the dirty bit on write hit + write back
        value.set_dirty(true);
    }

    // Read + write: move the block to the front
    deque.erase(block_index);
    deque.push_front(value);
    return Result(consts::HIT);
}

bool Unit::is_valid() {
    if(this->level == consts::MAIN) {
        return true;
    }
    return this->level > 0
        && this->write_hit_policy > 0
        && this->write_miss_policy > 0
        && this->block_size > 0
        && this->way > 0
        && this->set_count > 0
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

u32 Unit::get_hits() {
    return this->hits;
}

u32 Unit::get_misses() {
    return this->misses;
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

void Unit::set_set_count() {
    if(this->set_count == 0 && this->way > 0 && this->size > 0 && this->block_size > 0) {
        this->set_count = this->size / (u32)this->block_size / (u32)this->way;
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
