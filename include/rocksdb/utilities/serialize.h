#pragma once

#include <stdint.h>
#include <string>

// Serialize type T into a byte comprable
// string and append to out.
template<typename T>
void Serialize(T val, std::string& out);

// Specializations
template<>
inline void Serialize(int64_t val, std::string& out) {
}

template<>
inline void Serialize(bool val, std::string& out) {
}

template<>
inline void Serialize(double val, std::string& out) {
}

template<>
inline void Serialize(std::string val, std::string& out) {
}
