#pragma once
#include "types.hh"
#include "unit.hh"

class Memory {
    private:
        Vector<Unit> hierarchy;
        void exec(String&, String&);
        void load(u32, u32);
        void store(u32, u32);
        f32 time(u32);
    public:
        Memory();
        void conf(String&);
        void access(String&);
        void score();
};
