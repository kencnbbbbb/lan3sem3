#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include "../../str/array.h"

class ArrayTest : public ::testing::Test {
protected:
    void SetUp() override {
        arr.push_back("a");
        arr.push_back("b");
        arr.push_back("c");
    }
    Array arr;
};

TEST_F(ArrayTest, PushAndGet) {
    EXPECT_EQ(arr.length(), 3U);
    EXPECT_EQ(arr.get(0), "a");
    EXPECT_EQ(arr.get(2), "c");
    EXPECT_EQ(arr.read(), " a b c");
    EXPECT_FALSE(arr.empty());
}

TEST_F(ArrayTest, GetOutOfRange) {
    EXPECT_THROW(arr.get(3), out_of_range);
}

TEST_F(ArrayTest, Insert) {
    arr.insert(0, "x");
    arr.insert(2, "y");
    arr.insert(arr.length(), "z");
    EXPECT_EQ(arr.read(), " x a y b c z");
    EXPECT_THROW(arr.insert(arr.length() + 1, "bad"), out_of_range);
}

TEST_F(ArrayTest, Set) {
    arr.set(1, "B");
    EXPECT_EQ(arr.read(), " a B c");
    EXPECT_THROW(arr.set(3, "x"), out_of_range);
}

TEST_F(ArrayTest, Erase) {
    arr.erase(0);
    EXPECT_EQ(arr.read(), " b c");
    arr.erase(1);
    EXPECT_EQ(arr.read(), " b");
    EXPECT_THROW(arr.erase(1), out_of_range);
}

TEST_F(ArrayTest, Clear) {
    arr.clear();
    EXPECT_TRUE(arr.empty());
    EXPECT_EQ(arr.read(), "");
    arr.push_back("n");
    EXPECT_EQ(arr.read(), " n");
}

TEST(Array, CapacityGrows) {
    Array a;
    EXPECT_EQ(a.capacity(), Array::MIN_CAP);
    for (size_t i = 0; i < Array::MIN_CAP + 1; i++) a.push_back("x");
    EXPECT_EQ(a.capacity(), Array::MIN_CAP * 2);
    a.insert(0, "y");
    EXPECT_EQ(a.length(), Array::MIN_CAP + 2);
}

TEST(Array, ConstructorWithCapacity) {
    Array small(3);
    EXPECT_EQ(small.capacity(), Array::MIN_CAP);
    Array big(100);
    EXPECT_EQ(big.capacity(), 100U);
    EXPECT_THROW(Array(Array::MAX_CAP + 1), length_error);
}

TEST(Array, MaxCapacity) {
    Array a(Array::MAX_CAP);
    for (size_t i = 0; i < Array::MAX_CAP; i++) a.push_back("x");
    EXPECT_EQ(a.length(), Array::MAX_CAP);
    EXPECT_THROW(a.push_back("x"), length_error);
}

TEST_F(ArrayTest, CopyIsIndependent) {
    Array copy(arr);
    copy.set(0, "X");
    copy.push_back("d");
    EXPECT_EQ(arr.read(), " a b c");
    EXPECT_EQ(copy.read(), " X b c d");
}

TEST_F(ArrayTest, Assign) {
    Array other;
    other.push_back("q");
    other = arr;
    other.set(0, "X");
    EXPECT_EQ(arr.read(), " a b c");
    EXPECT_EQ(other.read(), " X b c");
    Array& ref = arr;
    arr = ref;
    EXPECT_EQ(arr.read(), " a b c");
}

TEST_F(ArrayTest, SerializeBinaryRoundTrip) {
    stringstream ss;
    arr.serialize_binary(ss);
    Array other;
    other.push_back("old");
    other.deserialize_binary(ss);
    EXPECT_EQ(other.read(), arr.read());
}

TEST_F(ArrayTest, SerializeTextRoundTrip) {
    arr.push_back("two words");
    stringstream ss;
    arr.serialize_text(ss);
    Array other;
    other.deserialize_text(ss);
    EXPECT_EQ(other.read(), arr.read());
}
