#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../../str/one_linked_list.h"
#include "../../str/linked_list.h"

template <typename T>
class ListTest:public ::testing::Test{
protected:
    void SetUp() override{
        list.push_back("10");
        list.push_back("20");
        list.push_back("30");
        list.push_back("40");
        list.push_back("50");
    }
    T list;
};


using ListTypes = ::testing::Types<OneLinked, Linked>;
TYPED_TEST_SUITE(ListTest, ListTypes);

TYPED_TEST(ListTest,operations){
    EXPECT_EQ(this->list.find("10"),0);
    TypeParam other;
    this->list.pop_back();
    this->list.pop_back();
    this->list.pop_back();
    this->list.pop_back();
    other.push_back("10");
    EXPECT_EQ(other.pop_back(),this->list.pop_back());
    this->list.push_back("10");
    EXPECT_TRUE(other.empty());
    EXPECT_FALSE(this->list.empty());
}

TYPED_TEST(ListTest,serdeser_text){
    ofstream out("linked_test.txt");
    this->list.serialize_text(out);
    TypeParam other;
    out.close();
    ifstream in("linked_test.txt");
    other.deserialize_text(in);
    in.close();
}

TYPED_TEST(ListTest,serdeser_binary){
    ofstream out("linked_test_binary.txt");
    this->list.serialize_binary(out);
    TypeParam other;
    out.close();
    ifstream in("linked_test_binary.txt");
    other.deserialize_binary(in);
    in.close();
}

TYPED_TEST(ListTest, PushFrontBackRead) {
    this->list.push_front("00");
    this->list.push_back("60");
    EXPECT_EQ(this->list.read(), " 00 10 20 30 40 50 60");
    EXPECT_EQ(this->list.read_reverse(), " 60 50 40 30 20 10 00");
    EXPECT_EQ(this->list.size(), 7U);
}

TYPED_TEST(ListTest, PushOnEmpty) {
    TypeParam a;
    a.push_front("x");
    EXPECT_EQ(a.read(), " x");
    TypeParam b;
    b.push_back("y");
    EXPECT_EQ(b.read(), " y");
    EXPECT_EQ(b.read_reverse(), " y");
    TypeParam c;
    EXPECT_EQ(c.read(), "");
    EXPECT_EQ(c.read_reverse(), "");
}

TYPED_TEST(ListTest, FindMissing) {
    EXPECT_EQ(this->list.find("30"), 2);
    EXPECT_EQ(this->list.find("50"), 4);
    EXPECT_EQ(this->list.find("zzz"), -1);
}

TYPED_TEST(ListTest, PushAfter) {
    this->list.push_after("30", "35");
    this->list.push_after("50", "60");
    EXPECT_EQ(this->list.read(), " 10 20 30 35 40 50 60");
    EXPECT_EQ(this->list.read_reverse(), " 60 50 40 35 30 20 10");
    EXPECT_THROW(this->list.push_after("zzz", "1"), invalid_argument);
}

TYPED_TEST(ListTest, PushBefore) {
    this->list.push_before("30", "25");
    this->list.push_before("10", "05");
    EXPECT_EQ(this->list.read(), " 05 10 20 25 30 40 50");
    EXPECT_EQ(this->list.read_reverse(), " 50 40 30 25 20 10 05");
    EXPECT_THROW(this->list.push_before("zzz", "1"), invalid_argument);
}

TYPED_TEST(ListTest, PopFrontBack) {
    EXPECT_EQ(this->list.pop_front(), "10");
    EXPECT_EQ(this->list.pop_back(), "50");
    EXPECT_EQ(this->list.read(), " 20 30 40");
    EXPECT_EQ(this->list.size(), 3U);
}

TYPED_TEST(ListTest, PopAll) {
    for (int i = 0; i < 5; i++) this->list.pop_front();
    EXPECT_TRUE(this->list.empty());
    EXPECT_THROW(this->list.pop_front(), out_of_range);
    EXPECT_THROW(this->list.pop_back(), out_of_range);
    this->list.push_back("a");
    EXPECT_EQ(this->list.read(), " a");
}

TYPED_TEST(ListTest, PopAfter) {
    EXPECT_EQ(this->list.pop_after("10"), "20");
    EXPECT_EQ(this->list.pop_after("40"), "50");
    EXPECT_EQ(this->list.read(), " 10 30 40");
    EXPECT_THROW(this->list.pop_after("40"), out_of_range);
    EXPECT_THROW(this->list.pop_after("zzz"), invalid_argument);
}

TYPED_TEST(ListTest, PopBefore) {
    EXPECT_EQ(this->list.pop_before("20"), "10");
    EXPECT_EQ(this->list.pop_before("40"), "30");
    EXPECT_EQ(this->list.read(), " 20 40 50");
    EXPECT_THROW(this->list.pop_before("20"), out_of_range);
    EXPECT_THROW(this->list.pop_before("zzz"), invalid_argument);
}

TYPED_TEST(ListTest, DelByValue) {
    EXPECT_TRUE(this->list.del_by_value("10"));
    EXPECT_TRUE(this->list.del_by_value("50"));
    EXPECT_TRUE(this->list.del_by_value("30"));
    EXPECT_FALSE(this->list.del_by_value("zzz"));
    EXPECT_EQ(this->list.read(), " 20 40");
    EXPECT_EQ(this->list.read_reverse(), " 40 20");
    EXPECT_TRUE(this->list.del_by_value("20"));
    EXPECT_TRUE(this->list.del_by_value("40"));
    EXPECT_TRUE(this->list.empty());
    this->list.push_back("n");
    EXPECT_EQ(this->list.read(), " n");
}

TYPED_TEST(ListTest, Clear) {
    this->list.clear();
    EXPECT_TRUE(this->list.empty());
    EXPECT_EQ(this->list.size(), 0U);
    this->list.push_front("a");
    EXPECT_EQ(this->list.read(), " a");
}

TYPED_TEST(ListTest, CopyIsIndependent) {
    TypeParam copy(this->list);
    copy.push_back("60");
    copy.pop_front();
    EXPECT_EQ(this->list.read(), " 10 20 30 40 50");
    EXPECT_EQ(copy.read(), " 20 30 40 50 60");
    TypeParam empty_src;
    TypeParam empty_copy(empty_src);
    EXPECT_TRUE(empty_copy.empty());
}

TYPED_TEST(ListTest, Assign) {
    TypeParam other;
    other.push_back("x");
    other.push_back("y");
    other = this->list;
    other.push_back("60");
    EXPECT_EQ(this->list.read(), " 10 20 30 40 50");
    EXPECT_EQ(other.read(), " 10 20 30 40 50 60");
    TypeParam& ref = this->list;
    this->list = ref;
    EXPECT_EQ(this->list.read(), " 10 20 30 40 50");
    other = TypeParam();
    EXPECT_TRUE(other.empty());
}

TYPED_TEST(ListTest, SerializeBinaryRoundTrip) {
    stringstream ss;
    this->list.serialize_binary(ss);
    TypeParam other;
    other.push_back("old");
    other.deserialize_binary(ss);
    EXPECT_EQ(other.read(), this->list.read());
    EXPECT_EQ(other.read_reverse(), this->list.read_reverse());
}

TYPED_TEST(ListTest, SerializeTextRoundTrip) {
    this->list.push_back("two words");
    stringstream ss;
    this->list.serialize_text(ss);
    TypeParam other;
    other.deserialize_text(ss);
    EXPECT_EQ(other.read(), this->list.read());
    EXPECT_EQ(other.size(), 6U);
}
