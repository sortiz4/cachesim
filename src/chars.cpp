#include "chars.hh"

bool chars::is_alphanum(char c) {
    return (
        (c >= chars::NUM_0 && c <= chars::NUM_9) ||
        (c >= chars::UPPER_A && c <= chars::UPPER_Z) ||
        (c >= chars::LOWER_A && c <= chars::LOWER_Z)
    );
}

char chars::normalize(char c) {
    if(c >= chars::LOWER_A && c <= chars::LOWER_Z) {
        return c - 32;
    }
    return c;
}

void chars::normalize(String &str) {
    for(auto i = 0; i < str.length(); i++) {
        str[i] = chars::normalize(str[i]);
    }
}
