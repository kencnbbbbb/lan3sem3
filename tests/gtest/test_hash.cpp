#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../str/open_hash.h"

template <typename T>
class HashTest : public ::testing::Test {
protected:
    void SetUp() override {
        h.insert("a");
        h.insert("b");
        h.insert("c");
    }
    T h;
};

using HashTypes = ::testing::Types<OpenHash>;
TYPED_TEST_SUITE(HashTest, HashTypes);

TYPED_TEST(HashTest, InsertContains) {
    EXPECT_EQ(this->h.size(), 3U);
    EXPECT_TRUE(this->h.contains("a"));
    EXPECT_TRUE(this->h.contains("c"));
    EXPECT_FALSE(this->h.contains("zzz"));
    EXPECT_FALSE(this->h.empty());
}

TYPED_TEST(HashTest, DuplicateInsert) {
    EXPECT_FALSE(this->h.insert("a"));
    EXPECT_EQ(this->h.size(), 3U);
}

TYPED_TEST(HashTest, Erase) {
    EXPECT_TRUE(this->h.erase("b"));
    EXPECT_FALSE(this->h.erase("b"));
    EXPECT_FALSE(this->h.erase("zzz"));
    EXPECT_FALSE(this->h.contains("b"));
    EXPECT_EQ(this->h.size(), 2U);
    EXPECT_TRUE(this->h.insert("b"));
    EXPECT_TRUE(this->h.contains("b"));
}

TYPED_TEST(HashTest, Clear) {
    this->h.clear();
    EXPECT_TRUE(this->h.empty());
    EXPECT_FALSE(this->h.contains("a"));
    EXPECT_TRUE(this->h.insert("a"));
    EXPECT_EQ(this->h.read(), " a");
}

TYPED_TEST(HashTest, ManyInsertsRehash) {
    for (int i = 0; i < 500; i++) EXPECT_TRUE(this->h.insert("k" + to_string(i)));
    EXPECT_EQ(this->h.size(), 503U);
    for (int i = 0; i < 500; i++) EXPECT_TRUE(this->h.contains("k" + to_string(i)));
    for (int i = 0; i < 500; i += 2) EXPECT_TRUE(this->h.erase("k" + to_string(i)));
    EXPECT_EQ(this->h.size(), 253U);
    for (int i = 0; i < 500; i++) EXPECT_EQ(this->h.contains("k" + to_string(i)), i % 2 == 1);
}

TYPED_TEST(HashTest, EraseThenInsertSameBucket) {
    for (int i = 0; i < 200; i++) this->h.insert("x" + to_string(i));
    for (int i = 0; i < 200; i++) this->h.erase("x" + to_string(i));
    for (int i = 0; i < 200; i++) this->h.insert("y" + to_string(i));
    EXPECT_EQ(this->h.size(), 203U);
}

TYPED_TEST(HashTest, ReadContainsAll) {
    string r = this->h.read();
    EXPECT_NE(r.find(" a"), string::npos);
    EXPECT_NE(r.find(" b"), string::npos);
    EXPECT_NE(r.find(" c"), string::npos);
}

TYPED_TEST(HashTest, CopyIsIndependent) {
    TypeParam copy(this->h);
    copy.insert("d");
    copy.erase("a");
    EXPECT_TRUE(this->h.contains("a"));
    EXPECT_FALSE(this->h.contains("d"));
    EXPECT_TRUE(copy.contains("d"));
    EXPECT_FALSE(copy.contains("a"));
}

TYPED_TEST(HashTest, Assign) {
    TypeParam other;
    other.insert("q");
    other = this->h;
    other.insert("d");
    EXPECT_EQ(this->h.size(), 3U);
    EXPECT_EQ(other.size(), 4U);
    EXPECT_FALSE(other.contains("q"));
    TypeParam& ref = this->h;
    this->h = ref;
    EXPECT_EQ(this->h.size(), 3U);
}

TYPED_TEST(HashTest, SerializeBinaryRoundTrip) {
    stringstream ss;
    this->h.serialize_binary(ss);
    TypeParam other;
    other.insert("old");
    other.deserialize_binary(ss);
    EXPECT_EQ(other.size(), 3U);
    EXPECT_TRUE(other.contains("a"));
    EXPECT_TRUE(other.contains("b"));
    EXPECT_TRUE(other.contains("c"));
    EXPECT_FALSE(other.contains("old"));
}

TYPED_TEST(HashTest, SerializeTextRoundTrip) {
    this->h.insert("two words");
    stringstream ss;
    this->h.serialize_text(ss);
    TypeParam other;
    other.deserialize_text(ss);
    EXPECT_EQ(other.size(), 4U);
    EXPECT_TRUE(other.contains("two words"));
}

TEST(OpenHash, CapacityRoundsUp) {
    EXPECT_EQ(OpenHash().capacity(), OpenHash::MIN_CAP);
    EXPECT_EQ(OpenHash(3).capacity(), OpenHash::MIN_CAP);
    EXPECT_EQ(OpenHash(20).capacity(), 32U);
    EXPECT_THROW(OpenHash(OpenHash::MAX_CAP + 1), length_error);
}

TEST(OpenHash, FindReturnsIndex) {
    OpenHash h;
    h.insert("a");
    int idx = h.find("a");
    EXPECT_GE(idx, 0);
    EXPECT_LT(idx, static_cast<int>(h.capacity()));
    EXPECT_EQ(h.find("zzz"), -1);
    h.erase("a");
    EXPECT_EQ(h.find("a"), -1);
}

TEST(OpenHash, GrowsAndOverflows) {
    OpenHash h;
    for (int i = 0; i < 100; i++) h.insert("k" + to_string(i));
    EXPECT_GT(h.capacity(), OpenHash::MIN_CAP);
    OpenHash big(OpenHash::MAX_CAP);
    bool thrown = false;
    try {
        for (int i = 0; i < 60000; i++) big.insert("k" + to_string(i));
    } catch (const length_error&) {
        thrown = true;
    }
    EXPECT_TRUE(thrown);
}

TEST(OpenHash, TombstonesTriggerRehash) {
    OpenHash h;
    for (int round = 0; round < 50; round++) {
        h.insert("t" + to_string(round));
        h.erase("t" + to_string(round));
    }
    EXPECT_TRUE(h.empty());
    h.insert("x");
    EXPECT_TRUE(h.contains("x"));
}

TEST(OpenHash, HashFunctions) {
    EXPECT_EQ(OpenHash::hash_djb2("abc"), OpenHash::hash_djb2("abc"));
    EXPECT_NE(OpenHash::hash_djb2("abc"), OpenHash::hash_djb2("abd"));
    EXPECT_EQ(OpenHash::hash_fnv1a("abc"), OpenHash::hash_fnv1a("abc"));
    EXPECT_NE(OpenHash::hash_fnv1a("abc"), OpenHash::hash_fnv1a("abd"));
}

#include "../variant_checks.h"
TEST(PersonalVariant, BothHashMethods) {
    check_hash_methods([](bool ok) { EXPECT_TRUE(ok); });
}

TEST(PersonalVariant, SerializationIOFailures) {

    auto check = [](bool ok) { EXPECT_TRUE(ok); };
    Array a; a.push_back("two words"); check_serialization_failures(a, check);
    OneLinked one; one.push_back("q\"uote"); check_serialization_failures(one, check);
    Linked two; two.push_back("Привет"); check_serialization_failures(two, check);
    Stack stack; stack.push(""); stack.push("x"); check_serialization_failures(stack, check);
    Queue queue; queue.push("x"); check_serialization_failures(queue, check);
    OpenHash hash; hash.insert("two words"); check_serialization_failures(hash, check);
    OpenHash linear(16, ProbeMode::Linear); linear.insert("x"); check_serialization_failures(linear, check);
    Tree tree; tree.insert(-7); tree.insert(0); check_serialization_failures(tree, check);

}
