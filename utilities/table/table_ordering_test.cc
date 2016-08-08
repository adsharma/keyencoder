#include <gtest/gtest.h>

#include "rocksdb/utilities/serialize.h"

using ::testing::InitGoogleTest;

TEST(Basic, Int64) {
  std::string out;

  Serialize<int64_t>(1234, out);
  auto s = rocksdb::Slice(out);
  auto i1 = Deserialize<int64_t>(s);
  EXPECT_EQ(1234, i1);
}

TEST(Basic, NegativeInt64) {
  std::string out;

  Serialize<int64_t>(-1234, out);
  auto s = rocksdb::Slice(out);
  auto i1 = Deserialize<int64_t>(s);
  EXPECT_EQ(-1234, i1);
}

TEST(Basic, Double) {
  std::string out;

  Serialize<double>(1.234, out);
  auto s = rocksdb::Slice(out);
  auto d1 = Deserialize<double>(s);
  EXPECT_EQ(1.234, d1);
}

TEST(Basic, NegativeDouble) {
  std::string out;

  Serialize<double>(-1.234, out);
  auto s = rocksdb::Slice(out);
  auto d1 = Deserialize<double>(s);
  EXPECT_EQ(-1.234, d1);
}

TEST(Basic, Bool) {
  std::string out;

  Serialize<bool>(true, out);
  Serialize<bool>(false, out);
  auto s = rocksdb::Slice(out);
  auto b1 = Deserialize<bool>(s);
  EXPECT_EQ(true, b1);
  auto b2 = Deserialize<bool>(s);
  EXPECT_EQ(false, b2);
}

TEST(Basic, String) {
  std::string out;

  Serialize<std::string>("hello", out);
  Serialize<std::string>("world", out);
  auto s = rocksdb::Slice(out);
  auto s1 = Deserialize<std::string>(s);
  EXPECT_EQ("hello", s1);
  auto s2 = Deserialize<std::string>(s);
  EXPECT_EQ("world", s2);
}

template<typename T>
std::string SerializeHelper(T t) {
  std::string out;
  Serialize<T>(t, out);
  return out;
}

TEST(Ordering, Int64) {
  EXPECT_LT(SerializeHelper<int64_t>(-2), SerializeHelper<int64_t>(-1));
  EXPECT_LT(SerializeHelper<int64_t>(-1), SerializeHelper<int64_t>(0));
  EXPECT_LT(SerializeHelper<int64_t>(0), SerializeHelper<int64_t>(1));
  EXPECT_LT(SerializeHelper<int64_t>(1), SerializeHelper<int64_t>(2));
}

TEST(Ordering, Double) {
  auto positive_infinity = std::numeric_limits<double>::infinity();
  auto negative_infinity = -std::numeric_limits<double>::infinity();
  EXPECT_LT(SerializeHelper<double>(negative_infinity), SerializeHelper<double>(-2.0));
  EXPECT_LT(SerializeHelper<double>(-2.0), SerializeHelper<double>(-1.0));
  EXPECT_LT(SerializeHelper<double>(-1.0), SerializeHelper<double>(0));
  EXPECT_LT(SerializeHelper<double>(0), SerializeHelper<double>(1.0));
  EXPECT_LT(SerializeHelper<double>(1.0), SerializeHelper<double>(2.0));
  EXPECT_LT(SerializeHelper<double>(2.0), SerializeHelper<double>(positive_infinity));
}

TEST(Ordering, String) {
  EXPECT_LT(SerializeHelper<std::string>("bar"), SerializeHelper<std::string>("foo"));
  EXPECT_LT(SerializeHelper<std::string>("foo"), SerializeHelper<std::string>("foobar"));
}

TEST(Ordering, bool) {
  EXPECT_LT(SerializeHelper<bool>(false), SerializeHelper<bool>(true));
}

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
