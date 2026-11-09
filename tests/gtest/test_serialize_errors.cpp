#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include "../../str/array.h"
#include "../../str/linked_list.h"
#include "../../str/one_linked_list.h"
#include "../../str/open_hash.h"
#include "../../str/queue.h"
#include "../../str/stack.h"
#include "../../str/tree.h"

template <typename T>
class SerializeErrors : public ::testing::Test {};

using AllTypes = ::testing::Types<Array, OneLinked, Linked, Stack, Queue, OpenHash, Tree>;
TYPED_TEST_SUITE(SerializeErrors, AllTypes);

TYPED_TEST(SerializeErrors, EmptyRoundTrip) {
    TypeParam a;
    stringstream bin;
    a.serialize_binary(bin);
    TypeParam b;
    b.deserialize_binary(bin);
    EXPECT_TRUE(b.empty());
    stringstream txt;
    a.serialize_text(txt);
    TypeParam c;
    c.deserialize_text(txt);
    EXPECT_TRUE(c.empty());
}

TYPED_TEST(SerializeErrors, BadBinaryTag) {
    stringstream ss("XXXXXXXXXXXX");
    TypeParam a;
    EXPECT_THROW(a.deserialize_binary(ss), runtime_error);
}

TYPED_TEST(SerializeErrors, TruncatedBinary) {
    TypeParam a;
    stringstream ss;
    a.serialize_binary(ss);
    stringstream cut(ss.str().substr(0, 6));
    TypeParam b;
    EXPECT_THROW(b.deserialize_binary(cut), runtime_error);
    stringstream none;
    EXPECT_THROW(b.deserialize_binary(none), runtime_error);
}

TYPED_TEST(SerializeErrors, HugeBinarySize) {
    TypeParam a;
    stringstream ss;
    a.serialize_binary(ss);
    string s = ss.str();
    for (size_t i = 4; i < 12; i++) s[i] = '\xFF';
    stringstream bad(s);
    TypeParam b;
    EXPECT_THROW(b.deserialize_binary(bad), runtime_error);
}

TYPED_TEST(SerializeErrors, BadTextHeader) {
    stringstream ss("Wrong 0\n");
    TypeParam a;
    EXPECT_THROW(a.deserialize_text(ss), runtime_error);
    stringstream none;
    EXPECT_THROW(a.deserialize_text(none), runtime_error);
}

TYPED_TEST(SerializeErrors, HugeTextSize) {
    TypeParam a;
    stringstream ss;
    a.serialize_text(ss);
    string s = ss.str();
    s = s.substr(0, s.find(' ')) + " 99999999999\n";
    stringstream bad(s);
    TypeParam b;
    EXPECT_THROW(b.deserialize_text(bad), runtime_error);
}

TYPED_TEST(SerializeErrors, TruncatedText) {
    TypeParam a;
    stringstream ss;
    a.serialize_text(ss);
    string s = ss.str();
    s = s.substr(0, s.find(' ')) + " 3\n";
    stringstream bad(s);
    TypeParam b;
    EXPECT_THROW(b.deserialize_text(bad), runtime_error);
}
