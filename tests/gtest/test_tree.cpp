#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include "../../str/tree.h"
#define CHECK(x) EXPECT_TRUE(x)
TEST(CompleteTree, PersonalVariant) {

    Tree t;
    CHECK(t.empty()); CHECK(t.is_complete()); CHECK(t.height() == 0);
    CHECK(!t.is_member(-1)); CHECK(t.structure().empty());
    for (int v : {50, 30, 70, 20, 40, 60, 80}) CHECK(t.insert(v));
    CHECK(t.read() == " 50 30 70 20 40 60 80");
    CHECK(t.is_member(40)); CHECK(!t.is_member(45));
    CHECK(t.size() == 7U); CHECK(t.height() == 3); CHECK(t.is_complete());
    CHECK(t.insert(50)); CHECK(t.size() == 8U); CHECK(t.height() == 4);
    CHECK(t.structure().find("root->left->left->left = 50") != std::string::npos);
    std::stringstream printed; t.print(printed); CHECK(printed.str() == t.structure());
    Tree copy(t); copy.insert(999); CHECK(!t.is_member(999));
    Tree assigned; assigned.insert(-5); assigned = t; CHECK(assigned.read() == t.read());
    Tree& alias = assigned; assigned = alias; CHECK(assigned.size() == 8U);
    std::stringstream binary; t.serialize_binary(binary); assigned.deserialize_binary(binary);
    CHECK(assigned.structure() == t.structure());
    std::stringstream text; t.serialize_text(text); assigned.deserialize_text(text);
    CHECK(assigned.structure() == t.structure());
    const auto before = assigned.structure();
    std::stringstream cut("CompleteTree 2\n1\n");
    bool failed = false; try { assigned.deserialize_text(cut); } catch (const std::runtime_error&) { failed = true; }
    CHECK(failed); CHECK(before == assigned.structure());
    assigned.clear(); CHECK(assigned.empty()); CHECK(assigned.size() == 0U);
    Tree emptyCopy(assigned); CHECK(emptyCopy.empty());
    for (int i = 0; i < 100; ++i) { assigned.insert(i % 7); CHECK(assigned.is_complete()); }
    CHECK(assigned.height() == 7);

}
#undef CHECK
