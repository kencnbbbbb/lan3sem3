#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../str/array.h"
#include "../../str/linked_list.h"
#include "../../str/one_linked_list.h"
#include "../../str/open_hash.h"
#include "../../str/queue.h"
#include "../../str/stack.h"
#include "../../str/tree.h"

TEMPLATE_TEST_CASE("Lists push and read", "[list]", OneLinked, Linked) {
    TestType l;
    REQUIRE(l.empty());
    REQUIRE(l.read() == "");
    REQUIRE(l.read_reverse() == "");
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    REQUIRE(l.read() == " a b c");
    REQUIRE(l.read_reverse() == " c b a");
    REQUIRE(l.size() == 3U);
    REQUIRE(l.find("b") == 1);
    REQUIRE(l.find("zzz") == -1);
}

TEMPLATE_TEST_CASE("Lists keep neighbours", "[list]", OneLinked, Linked) {
    TestType l;
    for (const char* v : {"10", "20", "30", "40", "50"}) l.push_back(v);

    SECTION("push_after and push_before") {
        l.push_after("50", "60");
        l.push_after("10", "15");
        l.push_before("10", "05");
        l.push_before("30", "25");
        REQUIRE(l.read() == " 05 10 15 20 25 30 40 50 60");
        REQUIRE(l.read_reverse() == " 60 50 40 30 25 20 15 10 05");
        REQUIRE_THROWS_AS(l.push_after("zzz", "1"), invalid_argument);
        REQUIRE_THROWS_AS(l.push_before("zzz", "1"), invalid_argument);
    }

    SECTION("pops") {
        REQUIRE(l.pop_front() == "10");
        REQUIRE(l.pop_back() == "50");
        REQUIRE(l.pop_after("20") == "30");
        REQUIRE(l.pop_before("40") == "20");
        REQUIRE(l.read() == " 40");
        REQUIRE_THROWS_AS(l.pop_after("40"), out_of_range);
        REQUIRE_THROWS_AS(l.pop_before("40"), out_of_range);
        REQUIRE_THROWS_AS(l.pop_after("zzz"), invalid_argument);
        REQUIRE_THROWS_AS(l.pop_before("zzz"), invalid_argument);
        l.pop_back();
        REQUIRE_THROWS_AS(l.pop_front(), out_of_range);
        REQUIRE_THROWS_AS(l.pop_back(), out_of_range);
    }

    SECTION("del_by_value and clear") {
        REQUIRE(l.del_by_value("10"));
        REQUIRE(l.del_by_value("50"));
        REQUIRE(l.del_by_value("30"));
        REQUIRE_FALSE(l.del_by_value("zzz"));
        REQUIRE(l.read() == " 20 40");
        l.clear();
        REQUIRE(l.empty());
        l.push_front("n");
        REQUIRE(l.read() == " n");
    }

    SECTION("copy and assign") {
        TestType copy(l);
        copy.push_back("60");
        REQUIRE(l.read() == " 10 20 30 40 50");
        REQUIRE(copy.read() == " 10 20 30 40 50 60");
        TestType other;
        other.push_back("x");
        other = l;
        REQUIRE(other.read() == " 10 20 30 40 50");
        TestType& ref = l;
        l = ref;
        REQUIRE(l.read() == " 10 20 30 40 50");
        other = TestType();
        REQUIRE(other.empty());
        TestType empty_copy(other);
        REQUIRE(empty_copy.empty());
    }

    SECTION("serialization") {
        l.push_back("two words");
        stringstream bin;
        l.serialize_binary(bin);
        TestType b;
        b.push_back("old");
        b.deserialize_binary(bin);
        REQUIRE(b.read() == l.read());
        stringstream txt;
        l.serialize_text(txt);
        TestType t;
        t.deserialize_text(txt);
        REQUIRE(t.read() == l.read());
    }
}

TEST_CASE("Stack", "[stack]") {
    Stack s;
    s.push("a");
    s.push("b");

    SECTION("lifo") {
        REQUIRE(s.top() == "b");
        REQUIRE(s.read() == " b a");
        REQUIRE(s.size() == 2U);
        REQUIRE(s.pop() == "b");
        REQUIRE(s.pop() == "a");
        REQUIRE(s.empty());
        REQUIRE_THROWS_AS(s.pop(), out_of_range);
        REQUIRE_THROWS_AS(s.top(), out_of_range);
    }

    SECTION("copy assign clear") {
        Stack c(s);
        c.push("c");
        REQUIRE(s.read() == " b a");
        Stack o;
        o.push("x");
        o = s;
        REQUIRE(o.read() == " b a");
        Stack& ref = s;
        s = ref;
        s.clear();
        REQUIRE(s.empty());
        Stack e;
        o = e;
        REQUIRE(o.empty());
    }

    SECTION("serialization") {
        stringstream bin;
        s.serialize_binary(bin);
        Stack b;
        b.deserialize_binary(bin);
        REQUIRE(b.read() == s.read());
        stringstream txt;
        s.serialize_text(txt);
        Stack t;
        t.deserialize_text(txt);
        REQUIRE(t.read() == s.read());
    }
}

TEST_CASE("Queue", "[queue]") {
    Queue q;
    q.push("a");
    q.push("b");
    q.push("c");

    SECTION("fifo") {
        REQUIRE(q.front() == "a");
        REQUIRE(q.read() == " a b c");
        REQUIRE(q.pop() == "a");
        REQUIRE(q.pop() == "b");
        q.push("d");
        REQUIRE(q.pop() == "c");
        REQUIRE(q.pop() == "d");
        REQUIRE(q.empty());
        REQUIRE_THROWS_AS(q.pop(), out_of_range);
        REQUIRE_THROWS_AS(q.front(), out_of_range);
    }

    SECTION("copy assign clear") {
        Queue c(q);
        c.push("d");
        REQUIRE(q.read() == " a b c");
        Queue o;
        o.push("x");
        o = q;
        REQUIRE(o.read() == " a b c");
        Queue& ref = q;
        q = ref;
        q.clear();
        REQUIRE(q.empty());
        Queue e;
        o = e;
        REQUIRE(o.empty());
    }

    SECTION("serialization") {
        stringstream bin;
        q.serialize_binary(bin);
        Queue b;
        b.deserialize_binary(bin);
        REQUIRE(b.read() == q.read());
        stringstream txt;
        q.serialize_text(txt);
        Queue t;
        t.deserialize_text(txt);
        REQUIRE(t.read() == q.read());
    }
}

TEST_CASE("Array", "[array]") {
    Array arr;
    arr.push_back("a");
    arr.push_back("b");
    arr.push_back("c");

    SECTION("get and set") {
        REQUIRE(arr.length() == 3U);
        REQUIRE(arr.get(2) == "c");
        REQUIRE(arr.read() == " a b c");
        REQUIRE_THROWS_AS(arr.get(3), out_of_range);
        arr.set(1, "B");
        REQUIRE(arr.read() == " a B c");
        REQUIRE_THROWS_AS(arr.set(3, "x"), out_of_range);
    }

    SECTION("insert and erase") {
        arr.insert(0, "x");
        arr.insert(2, "y");
        arr.insert(arr.length(), "z");
        REQUIRE(arr.read() == " x a y b c z");
        REQUIRE_THROWS_AS(arr.insert(arr.length() + 1, "bad"), out_of_range);
        arr.erase(0);
        arr.erase(arr.length() - 1);
        REQUIRE(arr.read() == " a y b c");
        REQUIRE_THROWS_AS(arr.erase(arr.length()), out_of_range);
        arr.clear();
        REQUIRE(arr.empty());
    }

    SECTION("capacity") {
        Array a;
        REQUIRE(a.capacity() == Array::MIN_CAP);
        for (size_t i = 0; i < Array::MIN_CAP + 1; i++) a.push_back("x");
        REQUIRE(a.capacity() == Array::MIN_CAP * 2);
        REQUIRE(Array(3).capacity() == Array::MIN_CAP);
        REQUIRE(Array(100).capacity() == 100U);
        REQUIRE_THROWS_AS(Array(Array::MAX_CAP + 1), length_error);
        Array big(Array::MAX_CAP);
        for (size_t i = 0; i < Array::MAX_CAP; i++) big.push_back("x");
        REQUIRE_THROWS_AS(big.push_back("x"), length_error);
    }

    SECTION("copy and assign") {
        Array c(arr);
        c.set(0, "X");
        REQUIRE(arr.read() == " a b c");
        Array o;
        o.push_back("q");
        o = arr;
        REQUIRE(o.read() == " a b c");
        Array& ref = arr;
        arr = ref;
        REQUIRE(arr.read() == " a b c");
    }

    SECTION("serialization") {
        stringstream bin;
        arr.serialize_binary(bin);
        Array b;
        b.deserialize_binary(bin);
        REQUIRE(b.read() == arr.read());
        stringstream txt;
        arr.serialize_text(txt);
        Array t;
        t.deserialize_text(txt);
        REQUIRE(t.read() == arr.read());
    }
}

TEST_CASE("OpenHash", "[hash]") {
    OpenHash h;

    SECTION("insert erase contains") {
        REQUIRE(h.insert("a"));
        REQUIRE(h.insert("b"));
        REQUIRE_FALSE(h.insert("a"));
        REQUIRE(h.contains("a"));
        REQUIRE_FALSE(h.contains("zzz"));
        REQUIRE(h.size() == 2U);
        REQUIRE(h.find("a") >= 0);
        REQUIRE(h.find("zzz") == -1);
        REQUIRE(h.erase("a"));
        REQUIRE_FALSE(h.erase("a"));
        REQUIRE(h.find("a") == -1);
        REQUIRE(h.insert("a"));
        h.clear();
        REQUIRE(h.empty());
        REQUIRE(h.read() == "");
    }

    SECTION("rehash and tombstones") {
        for (int i = 0; i < 500; i++) h.insert("k" + to_string(i));
        REQUIRE(h.size() == 500U);
        REQUIRE(h.capacity() > OpenHash::MIN_CAP);
        for (int i = 0; i < 500; i += 2) h.erase("k" + to_string(i));
        for (int i = 0; i < 500; i++) REQUIRE(h.contains("k" + to_string(i)) == (i % 2 == 1));
        OpenHash t;
        for (int i = 0; i < 50; i++) {
            t.insert("t" + to_string(i));
            t.erase("t" + to_string(i));
        }
        REQUIRE(t.empty());
    }

    SECTION("capacity limits") {
        REQUIRE(OpenHash(20).capacity() == 32U);
        REQUIRE_THROWS_AS(OpenHash(OpenHash::MAX_CAP + 1), length_error);
        OpenHash big(OpenHash::MAX_CAP);
        bool thrown = false;
        try {
            for (int i = 0; i < 60000; i++) big.insert("k" + to_string(i));
        } catch (const length_error&) {
            thrown = true;
        }
        REQUIRE(thrown);
    }

    SECTION("hash functions") {
        REQUIRE(OpenHash::hash_djb2("abc") == OpenHash::hash_djb2("abc"));
        REQUIRE(OpenHash::hash_djb2("abc") != OpenHash::hash_djb2("abd"));
        REQUIRE(OpenHash::hash_fnv1a("abc") == OpenHash::hash_fnv1a("abc"));
        REQUIRE(OpenHash::hash_fnv1a("abc") != OpenHash::hash_fnv1a("abd"));
    }

    SECTION("copy assign serialize") {
        h.insert("a");
        h.insert("two words");
        OpenHash c(h);
        c.insert("d");
        REQUIRE(h.size() == 2U);
        OpenHash o;
        o.insert("q");
        o = h;
        REQUIRE_FALSE(o.contains("q"));
        OpenHash& ref = h;
        h = ref;
        stringstream bin;
        h.serialize_binary(bin);
        OpenHash b;
        b.deserialize_binary(bin);
        REQUIRE(b.size() == 2U);
        REQUIRE(b.contains("two words"));
        stringstream txt;
        h.serialize_text(txt);
        OpenHash t;
        t.deserialize_text(txt);
        REQUIRE(t.size() == 2U);
        REQUIRE(t.contains("a"));
    }
}

TEMPLATE_TEST_CASE("Serialization errors", "[serialize]", Array, OneLinked, Linked, Stack, Queue, OpenHash, Tree) {
    TestType a;

    SECTION("empty round trip") {
        stringstream bin;
        a.serialize_binary(bin);
        TestType b;
        b.deserialize_binary(bin);
        REQUIRE(b.empty());
        stringstream txt;
        a.serialize_text(txt);
        TestType c;
        c.deserialize_text(txt);
        REQUIRE(c.empty());
    }

    SECTION("bad binary") {
        stringstream bad_tag("XXXXXXXXXXXX");
        REQUIRE_THROWS_AS(a.deserialize_binary(bad_tag), runtime_error);
        stringstream ok;
        a.serialize_binary(ok);
        string s = ok.str();
        stringstream cut(s.substr(0, 6));
        REQUIRE_THROWS_AS(a.deserialize_binary(cut), runtime_error);
        stringstream none;
        REQUIRE_THROWS_AS(a.deserialize_binary(none), runtime_error);
        for (size_t i = 4; i < 12; i++) s[i] = '\xFF';
        stringstream huge(s);
        REQUIRE_THROWS_AS(a.deserialize_binary(huge), runtime_error);
    }

    SECTION("bad text") {
        stringstream bad_tag("Wrong 0\n");
        REQUIRE_THROWS_AS(a.deserialize_text(bad_tag), runtime_error);
        stringstream none;
        REQUIRE_THROWS_AS(a.deserialize_text(none), runtime_error);
        stringstream ok;
        a.serialize_text(ok);
        string head = ok.str().substr(0, ok.str().find(' '));
        stringstream huge(head + " 99999999999\n");
        REQUIRE_THROWS_AS(a.deserialize_text(huge), runtime_error);
        stringstream truncated(head + " 3\n");
        REQUIRE_THROWS_AS(a.deserialize_text(truncated), runtime_error);
    }
}

TEST_CASE("Complete tree personal variant", "[tree]") {

    Tree t;
    REQUIRE(t.empty()); REQUIRE(t.is_complete()); REQUIRE(t.height() == 0);
    REQUIRE(!t.is_member(-1)); REQUIRE(t.structure().empty());
    for (int v : {50, 30, 70, 20, 40, 60, 80}) REQUIRE(t.insert(v));
    REQUIRE(t.read() == " 50 30 70 20 40 60 80");
    REQUIRE(t.is_member(40)); REQUIRE(!t.is_member(45));
    REQUIRE(t.size() == 7U); REQUIRE(t.height() == 3); REQUIRE(t.is_complete());
    REQUIRE(t.insert(50)); REQUIRE(t.size() == 8U); REQUIRE(t.height() == 4);
    REQUIRE(t.structure().find("root->left->left->left = 50") != std::string::npos);
    std::stringstream printed; t.print(printed); REQUIRE(printed.str() == t.structure());
    Tree copy(t); copy.insert(999); REQUIRE(!t.is_member(999));
    Tree assigned; assigned.insert(-5); assigned = t; REQUIRE(assigned.read() == t.read());
    Tree& alias = assigned; assigned = alias; REQUIRE(assigned.size() == 8U);
    std::stringstream binary; t.serialize_binary(binary); assigned.deserialize_binary(binary);
    REQUIRE(assigned.structure() == t.structure());
    std::stringstream text; t.serialize_text(text); assigned.deserialize_text(text);
    REQUIRE(assigned.structure() == t.structure());
    const auto before = assigned.structure();
    std::stringstream cut("CompleteTree 2\n1\n");
    bool failed = false; try { assigned.deserialize_text(cut); } catch (const std::runtime_error&) { failed = true; }
    REQUIRE(failed); REQUIRE(before == assigned.structure());
    assigned.clear(); REQUIRE(assigned.empty()); REQUIRE(assigned.size() == 0U);
    Tree emptyCopy(assigned); REQUIRE(emptyCopy.empty());
    for (int i = 0; i < 100; ++i) { assigned.insert(i % 7); REQUIRE(assigned.is_complete()); }
    REQUIRE(assigned.height() == 7);

}

#include "../variant_checks.h"
TEST_CASE("Both hash methods personal variant", "[hash]") {
    check_hash_methods([](bool ok) { REQUIRE(ok); });
}

TEST_CASE("Serialization IO failures", "[serialize]") {

    auto check = [](bool ok) { REQUIRE(ok); };
    Array a; a.push_back("two words"); check_serialization_failures(a, check);
    OneLinked one; one.push_back("q\"uote"); check_serialization_failures(one, check);
    Linked two; two.push_back("Привет"); check_serialization_failures(two, check);
    Stack stack; stack.push(""); stack.push("x"); check_serialization_failures(stack, check);
    Queue queue; queue.push("x"); check_serialization_failures(queue, check);
    OpenHash hash; hash.insert("two words"); check_serialization_failures(hash, check);
    OpenHash linear(16, ProbeMode::Linear); linear.insert("x"); check_serialization_failures(linear, check);
    Tree tree; tree.insert(-7); tree.insert(0); check_serialization_failures(tree, check);

}
