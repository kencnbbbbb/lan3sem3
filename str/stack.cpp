#include "stack.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "serialize.h"

using namespace std;

Stack::Node::Node(string val, Node* nxt) : data(std::move(val)), next(nxt) {}

Stack::Stack() : head(nullptr), sz(0) {}

Stack::~Stack() {
    clear();
}

Stack::Stack(const Stack& other) : head(nullptr), sz(0) {
    copy_from(other);
}

Stack& Stack::operator=(const Stack& other) {
    if (this == &other) return *this;
    clear();
    copy_from(other);
    return *this;
}

void Stack::copy_from(const Stack& other) {
    Node** last = &head;
    for (const Node* node = other.head; node; node = node->next) {
        *last = new Node(node->data);
        last = &(*last)->next;
        sz++;
    }
}

void Stack::clear() {
    while (head) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    sz = 0;
}

void Stack::push(const string& value) {
    head = new Node(value, head);
    sz++;
}

string Stack::pop() {
    if (!head) throw out_of_range("Stack::pop: стек пуст");
    Node* node = head;
    string val = std::move(node->data);
    head = head->next;
    delete node;
    sz--;
    return val;
}

const string& Stack::top() const {
    if (!head) throw out_of_range("Stack::top: стек пуст");
    return head->data;
}

size_t Stack::size() const {
    return sz;
}

bool Stack::empty() const {
    return head == nullptr;
}

string Stack::read() const {
    stringstream ss;
    for (const Node* node = head; node; node = node->next) {
        ss << " " << node->data;
    }
    return ss.str();
}

void Stack::serialize_binary(ostream& out) const {
    ser::write_tag(out, "STAK");
    ser::write_size(out, sz);
    Stack reversed;
    for (const Node* node = head; node; node = node->next) reversed.push(node->data);
    for (const Node* node = reversed.head; node; node = node->next) {
        ser::write_string(out, node->data);
    }
}

void Stack::deserialize_binary(istream& in) {
    ser::read_tag(in, "STAK");
    const size_t n = ser::read_size(in);
    Stack tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push(ser::read_string(in));
    }
    *this = tmp;
}

void Stack::serialize_text(ostream& out) const {
    ser::write_text_header(out, "Stack", sz);
    Stack reversed;
    for (const Node* node = head; node; node = node->next) reversed.push(node->data);
    for (const Node* node = reversed.head; node; node = node->next) {
        ser::write_text_string(out, node->data);
    }
}

void Stack::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, "Stack");
    Stack tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push(ser::read_text_string(in));
    }
    *this = tmp;
}
