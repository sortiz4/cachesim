#pragma once
#include "types.hh"
#include "unit.hh"

class Memory {
    private:
        Unit *unit;
        void exec(String&, String&);
        void load(u32);
        void store(u32);
    public:
        Memory();
        ~Memory();
        void conf(String&);
        void access(String&);
        void score();
};
