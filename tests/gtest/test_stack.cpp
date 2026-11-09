#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../../str/stack.h"


TEST(Stack, CopyIsIndependent) {
    Stack s;
    s.push("1");
    Stack f(s);
    EXPECT_EQ(f.pop(), "1");
    EXPECT_THROW(f.pop(), out_of_range);
    EXPECT_EQ(s.size(), 1U);
}

TEST(Stack, Assign) {
    Stack s, g;
    s.push("1");
    g.push("2");
    g = s;
    EXPECT_EQ(g.pop(), "1");
    EXPECT_TRUE(g.empty());
    g.push("11");
    EXPECT_EQ(g.top(), "11");
    EXPECT_EQ(g.read(), " 11");
    g.clear();
    EXPECT_TRUE(g.empty());
}

TEST(Stack, SerializeTextToFile) {
    Stack a;
    a.push("1");
    a.push("два слова");

    ofstream out("stack_test.txt");
    a.serialize_text(out);
    out.close();

    ifstream check("stack_test.txt");
    stringstream content;
    content << check.rdbuf();
    check.close();
    EXPECT_EQ(content.str(), "Stack 2\n\"1\"\n\"два слова\"\n");

    ifstream in("stack_test.txt");
    Stack b;
    b.deserialize_text(in);
    in.close();
    EXPECT_EQ(b.read(), a.read());
    EXPECT_EQ(b.top(), "два слова");
}

TEST(Stack, LifoOrder) {
    Stack s;
    EXPECT_TRUE(s.empty());
    s.push("a");
    s.push("b");
    s.push("c");
    EXPECT_EQ(s.size(), 3U);
    EXPECT_EQ(s.top(), "c");
    EXPECT_EQ(s.read(), " c b a");
    EXPECT_EQ(s.pop(), "c");
    EXPECT_EQ(s.pop(), "b");
    EXPECT_EQ(s.pop(), "a");
    EXPECT_TRUE(s.empty());
}

TEST(Stack, EmptyThrows) {
    Stack s;
    EXPECT_THROW(s.pop(), out_of_range);
    EXPECT_THROW(s.top(), out_of_range);
}

TEST(Stack, CopyManyAndSelfAssign) {
    Stack s;
    s.push("a");
    s.push("b");
    Stack c(s);
    EXPECT_EQ(c.read(), " b a");
    Stack& ref = s;
    s = ref;
    EXPECT_EQ(s.read(), " b a");
    Stack e;
    Stack ec(e);
    EXPECT_TRUE(ec.empty());
    c = e;
    EXPECT_TRUE(c.empty());
}

TEST(Stack, SerializeBinaryRoundTrip) {
    Stack a;
    a.push("1");
    a.push("два слова");
    stringstream ss;
    a.serialize_binary(ss);
    Stack b;
    b.push("old");
    b.deserialize_binary(ss);
    EXPECT_EQ(b.read(), a.read());
    EXPECT_EQ(b.top(), "два слова");
}

TEST(Stack, SerializeBinaryToFile) {
    Stack a;
    a.push("1");
    a.push("x");
    ofstream out("stack_test.bin", ios::binary);
    a.serialize_binary(out);
    out.close();
    ifstream in("stack_test.bin", ios::binary);
    ASSERT_TRUE(in.is_open());
    Stack b;
    b.deserialize_binary(in);
    in.close();
    remove("stack_test.bin");
    EXPECT_EQ(b.read(), a.read());
}

TEST(Stack, SerializeTextRoundTrip) {
    Stack a;
    a.push("q\"uote");
    a.push("two words");
    stringstream ss;
    a.serialize_text(ss);
    Stack b;
    b.deserialize_text(ss);
    EXPECT_EQ(b.read(), a.read());
}
