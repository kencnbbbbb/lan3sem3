#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include "../../str/queue.h"

TEST(Queue, FifoOrder) {
    Queue q;
    EXPECT_TRUE(q.empty());
    q.push("a");
    q.push("b");
    q.push("c");
    EXPECT_EQ(q.size(), 3U);
    EXPECT_EQ(q.front(), "a");
    EXPECT_EQ(q.read(), " a b c");
    EXPECT_EQ(q.pop(), "a");
    EXPECT_EQ(q.pop(), "b");
    q.push("d");
    EXPECT_EQ(q.pop(), "c");
    EXPECT_EQ(q.pop(), "d");
    EXPECT_TRUE(q.empty());
    q.push("e");
    EXPECT_EQ(q.read(), " e");
}

TEST(Queue, EmptyThrows) {
    Queue q;
    EXPECT_THROW(q.pop(), out_of_range);
    EXPECT_THROW(q.front(), out_of_range);
}

TEST(Queue, Clear) {
    Queue q;
    q.push("a");
    q.push("b");
    q.clear();
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0U);
    q.push("c");
    EXPECT_EQ(q.read(), " c");
}

TEST(Queue, CopyAndAssign) {
    Queue q;
    q.push("a");
    q.push("b");
    Queue c(q);
    c.push("c");
    EXPECT_EQ(q.read(), " a b");
    EXPECT_EQ(c.read(), " a b c");
    Queue o;
    o.push("x");
    o = q;
    o.push("z");
    EXPECT_EQ(q.read(), " a b");
    EXPECT_EQ(o.read(), " a b z");
    Queue& ref = q;
    q = ref;
    EXPECT_EQ(q.read(), " a b");
    Queue e;
    Queue ec(e);
    EXPECT_TRUE(ec.empty());
    o = e;
    EXPECT_TRUE(o.empty());
}

TEST(Queue, SerializeBinaryRoundTrip) {
    Queue a;
    a.push("1");
    a.push("два слова");
    stringstream ss;
    a.serialize_binary(ss);
    Queue b;
    b.push("old");
    b.deserialize_binary(ss);
    EXPECT_EQ(b.read(), a.read());
    EXPECT_EQ(b.front(), "1");
}

TEST(Queue, SerializeTextRoundTrip) {
    Queue a;
    a.push("1");
    a.push("two words");
    stringstream ss;
    a.serialize_text(ss);
    Queue b;
    b.deserialize_text(ss);
    EXPECT_EQ(b.read(), a.read());
}
