#include <iostream>
#include "exceptions.hh"
#include "memory.hh"
#include "status.hh"
using namespace std;

int main(int argc, char *argv[]) {
    // Parse the arguments
    if (argc < 3) {
        cerr << "usage: cachesim conf access" << endl;
        return status::USAGE;
    }
    auto conf = String(argv[1]);
    auto access = String(argv[2]);
    auto memory = Memory();

    // Parse the configuration file
    try {
        memory.conf(conf);
    } catch (RuntimeException &e) {
        cerr << e.what() << endl;
        return status::CONF;
    }

    // Parse and execute the access file
    try {
        memory.access(access);
    } catch (RuntimeException &e) {
        cerr << e.what() << endl;
        return status::ACCESS;
    }
    memory.score();
    return status::OKAY;
}
