// Test-only command adapter for comparing the real C++ and Go containers.
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "../../str/array.h"
#include "../../str/linked_list.h"
#include "../../str/one_linked_list.h"
#include "../../str/stack.h"
#include "../../str/queue.h"
#include "../../str/open_hash.h"
#include "../../str/tree.h"

std::string hex(const std::string& value) {
    if (value.empty()) return "-";
    const char* digits = "0123456789abcdef";
    std::string out;
    for (unsigned char c : value) { out += digits[c >> 4]; out += digits[c & 15]; }
    return out;
}
std::string unhex(const std::string& value) {
    if (value == "-") return "";
    std::string out;
    for (std::size_t i = 0; i < value.size(); i += 2)
        out += static_cast<char>(std::stoi(value.substr(i, 2), nullptr, 16));
    return out;
}
std::string operation(Array& a, const std::string& op, int index,
                      const std::string& v, const std::string&) {
    const auto i = static_cast<std::size_t>(index);
    if (op == "push") a.push_back(v);
    else if (op == "insert") a.insert(i, v);
    else if (op == "get") return a.get(i);
    else if (op == "set") a.set(i, v);
    else if (op == "erase") a.erase(i);
    else if (op == "clear") a.clear();
    else if (op == "len") return std::to_string(a.length());
    else if (op == "cap") return std::to_string(a.capacity());
    else if (op == "empty") return a.empty() ? "1" : "0";
    else if (op == "clone") { Array copy(a); copy.push_back(v); return std::to_string(copy.length()); }
    else throw std::invalid_argument("operation");
    return "";
}
template<class List>
std::string list_operation(List& a, const std::string& op,
                           const std::string& v, const std::string& w) {
    if (op == "pushback") a.push_back(v);
    else if (op == "pushfront") a.push_front(v);
    else if (op == "after") a.push_after(v, w);
    else if (op == "before") a.push_before(v, w);
    else if (op == "popback") return a.pop_back();
    else if (op == "popfront") return a.pop_front();
    else if (op == "popafter") return a.pop_after(v);
    else if (op == "popbefore") return a.pop_before(v);
    else if (op == "find") return std::to_string(a.find(v));
    else if (op == "del") return a.del_by_value(v) ? "1" : "0";
    else if (op == "reverse") return a.read_reverse();
    else if (op == "clear") a.clear();
    else if (op == "len") return std::to_string(a.size());
    else if (op == "empty") return a.empty() ? "1" : "0";
    else if (op == "clone") { List copy(a); copy.push_back(v); return std::to_string(copy.size()); }
    else throw std::invalid_argument("operation");
    return "";
}
std::string operation(OneLinked& a, const std::string& op, int, const std::string& v, const std::string& w) {
    return list_operation(a, op, v, w);
}
std::string operation(Linked& a, const std::string& op, int, const std::string& v, const std::string& w) {
    return list_operation(a, op, v, w);
}
template<class Container>
std::string sequence_operation(Container& a, const std::string& op, const std::string& v) {
    if (op == "push") a.push(v);
    else if (op == "pop") return a.pop();
    else if (op == "clear") a.clear();
    else if (op == "len") return std::to_string(a.size());
    else if (op == "empty") return a.empty() ? "1" : "0";
    else if (op == "clone") { Container copy(a); copy.push(v); return std::to_string(copy.size()); }
    else throw std::invalid_argument("operation");
    return "";
}
std::string operation(Stack& a, const std::string& op, int, const std::string& v, const std::string&) {
    return op == "peek" ? a.top() : sequence_operation(a, op, v);
}
std::string operation(Queue& a, const std::string& op, int, const std::string& v, const std::string&) {
    return op == "peek" ? a.front() : sequence_operation(a, op, v);
}
std::string operation(OpenHash& a, const std::string& op, int, const std::string& v, const std::string&) {
    if (op == "insert") return a.insert(v) ? "1" : "0";
    if (op == "del") return a.erase(v) ? "1" : "0";
    if (op == "contains") return a.contains(v) ? "1" : "0";
    if (op == "find") return a.find(v) >= 0 ? "1" : "0";
    if (op == "len") return std::to_string(a.size());
    if (op == "empty") return a.empty() ? "1" : "0";
    if (op == "clone") { OpenHash copy(a); copy.insert(v); return std::to_string(copy.size()); }
    if (op == "clear") { a.clear(); return ""; }
    throw std::invalid_argument("operation");
}
std::string operation(Tree& a, const std::string& op, int i, const std::string&, const std::string&) {
    if (op == "insert") return a.insert(i) ? "1" : "0";
    if (op == "contains") return a.is_member(i) ? "1" : "0";
    if (op == "complete") return a.is_complete() ? "1" : "0";
    if (op == "height") return std::to_string(a.height());
    if (op == "len") return std::to_string(a.size());
    if (op == "empty") return a.empty() ? "1" : "0";
    if (op == "structure") return a.structure();
    if (op == "clone") { Tree copy(a); copy.insert(i); return std::to_string(copy.size()); }
    if (op == "clear") { a.clear(); return ""; }
    throw std::invalid_argument("operation");
}
template<class T> int execute(T& object) {
    std::string op, a, b;
    int index;
    while (std::cin >> op >> index >> a >> b) {
        std::string status = "OK", result;
        try { result = operation(object, op, index, unhex(a), unhex(b)); }
        catch (const std::exception&) { status = "ERROR"; }
        std::ostringstream state; object.serialize_text(state);
        std::cout << status << '\t' << hex(result) << '\t' << hex(state.str()) << '\n';
    }
    return 0;
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const std::string kind = argv[1];
    if (kind == "Array") { Array a; return execute(a); }
    if (kind == "OneLinked") { OneLinked a; return execute(a); }
    if (kind == "Linked") { Linked a; return execute(a); }
    if (kind == "Stack") { Stack a; return execute(a); }
    if (kind == "Queue") { Queue a; return execute(a); }
    if (kind == "Tree") { Tree a; return execute(a); }
    if (kind == "OpenHash" || kind == "LinearHash") {
        OpenHash a(16, kind == "LinearHash" ? ProbeMode::Linear : ProbeMode::Double);
        return execute(a);
    }
    return 2;
}
