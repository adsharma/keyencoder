#pragma once

#include <stdint.h>
#include <string>
#include <rocksdb/slice.h>

// Serialize type T into a byte comprable
// string and append to out.
template<typename T>
void Serialize(T val, std::string& out);

template<typename T>
T Deserialize(rocksdb::Slice& in);

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

template<>
inline int64_t Deserialize(rocksdb::Slice& in) {
}

template<>
inline bool Deserialize(rocksdb::Slice& in) {
}

template<>
inline double Deserialize(rocksdb::Slice& in) {
}
