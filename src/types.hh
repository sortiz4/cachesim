#pragma once
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using String = std::string;
using FileReader = std::ifstream;
using StringBuilder = std::ostringstream;
template <typename V, typename K> using HashMap = std::unordered_map<K, V>;
template <typename V> using Vector = std::vector<V>;
