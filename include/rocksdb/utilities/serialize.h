#pragma once

#include <rocksdb/slice.h>
#include <endian.h>
#include <stdint.h>
#include <string>

// Serialize type T into a byte comprable
// string and append to out.
template<typename T>
void Serialize(T val, std::string& out);

template<typename T>
T Deserialize(rocksdb::Slice& in);

// Specializations
template<>
inline void Serialize(int64_t val, std::string& out) {
  val ^= (1UL << 63);
  val = htobe64(val);
  out.append((const char *) &val, sizeof(val));
}

template<>
inline void Serialize(bool val, std::string& out) {
  out.append((const char *) &val, sizeof(val));
}

template<>
inline void Serialize(double val, std::string& out) {
  int64_t mask = -1;
  if (val >= 0) {
    mask = 1UL << 63;
  }
  int64_t buf;
  buf = *(int64_t *) &val;
  buf ^= mask;
  buf = htobe64(buf);
  out.append((const char *) &buf, sizeof(buf));
}

template<>
inline void Serialize(std::string val, std::string& out) {
  // +1 to include the null terminated char
  out.append((const char *) val.c_str(), val.length() + 1);
}

template<>
inline int64_t Deserialize(rocksdb::Slice& in) {
  auto tmp = *(int64_t *) in.data();
  tmp = be64toh(tmp);
  tmp ^= (1UL << 63);
  in.remove_prefix(sizeof(int64_t));
  return tmp;
}

template<>
inline bool Deserialize(rocksdb::Slice& in) {
  auto p = *(bool *) in.data();
  in.remove_prefix(sizeof(bool));
  return p;
}

template<>
inline double Deserialize(rocksdb::Slice& in) {
  auto tmp = *(int64_t *) in.data();
  int64_t mask = -1;
  tmp = be64toh(tmp);
  auto *tmpd = (double *) &tmp;
  if (*tmpd < 0) {
    mask = 1UL << 63;
  }
  tmp ^= mask;
  in.remove_prefix(sizeof(double));
  return *tmpd;
}

template<>
inline rocksdb::Slice Deserialize(rocksdb::Slice& in) {
  size_t len = strnlen(in.data(), in.size());
  rocksdb::Slice tmp(in.data(), len);
  in.remove_prefix(len + 1);
  return tmp;
}

template<>
inline std::string Deserialize(rocksdb::Slice& in) {
  return Deserialize<rocksdb::Slice>(in).ToString();
}
