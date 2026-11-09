#include "linked_list.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "serialize.h"

using namespace std;

Linked::Node::Node(string val, Node* nxt, Node* prv)
    : data(std::move(val)), next(nxt), prev(prv) {}

Linked::Linked() : head(nullptr), tail(nullptr), sz(0) {}

Linked::~Linked() {
    clear();
}

Linked::Linked(const Linked& other) : head(nullptr), tail(nullptr), sz(0) {
    copy_from(other);
}

Linked& Linked::operator=(const Linked& other) {
    if (this == &other) return *this;
    clear();
    copy_from(other);
    return *this;
}

void Linked::copy_from(const Linked& other) {
    for (const Node* node = other.head; node; node = node->next) {
        push_back(node->data);
    }
}

void Linked::clear() {
    while (head) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
    sz = 0;
}

Linked::Node* Linked::find_node(const string& value) const {
    Node* node = head;
    while (node && node->data != value) node = node->next;
    return node;
}

void Linked::unlink(Node* node) {
    if (node->prev) node->prev->next = node->next;
    else head = node->next;
    if (node->next) node->next->prev = node->prev;
    else tail = node->prev;
    delete node;
    sz--;
}

int Linked::find(const string& value) const {
    int count = 0;
    for (const Node* node = head; node; node = node->next, count++) {
        if (node->data == value) return count;
    }
    return -1;
}

void Linked::push_back(const string& data) {
    Node* node = new Node(data, nullptr, tail);
    if (tail) tail->next = node;
    else head = node;
    tail = node;
    sz++;
}

void Linked::push_front(const string& data) {
    Node* node = new Node(data, head, nullptr);
    if (head) head->prev = node;
    else tail = node;
    head = node;
    sz++;
}

void Linked::push_after(const string& prev_el, const string& data) {
    Node* node = find_node(prev_el);
    if (!node) throw invalid_argument("Linked::push_after: элемент не найден");
    if (node == tail) {
        push_back(data);
        return;
    }
    Node* new_node = new Node(data, node->next, node);
    node->next->prev = new_node;
    node->next = new_node;
    sz++;
}

void Linked::push_before(const string& next_el, const string& data) {
    Node* node = find_node(next_el);
    if (!node) throw invalid_argument("Linked::push_before: элемент не найден");
    if (node == head) {
        push_front(data);
        return;
    }
    Node* new_node = new Node(data, node, node->prev);
    node->prev->next = new_node;
    node->prev = new_node;
    sz++;
}

string Linked::pop_back() {
    if (!tail) throw out_of_range("Linked::pop_back: список пуст");
    string val = tail->data;
    unlink(tail);
    return val;
}

string Linked::pop_front() {
    if (!head) throw out_of_range("Linked::pop_front: список пуст");
    string val = head->data;
    unlink(head);
    return val;
}

string Linked::pop_before(const string& next_el) {
    Node* node = find_node(next_el);
    if (!node) throw invalid_argument("Linked::pop_before: элемент не найден");
    if (!node->prev) throw out_of_range("Linked::pop_before: перед элементом пусто");
    string val = node->prev->data;
    unlink(node->prev);
    return val;
}

string Linked::pop_after(const string& prev_el) {
    Node* node = find_node(prev_el);
    if (!node) throw invalid_argument("Linked::pop_after: элемент не найден");
    if (!node->next) throw out_of_range("Linked::pop_after: после элемента пусто");
    string val = node->next->data;
    unlink(node->next);
    return val;
}

bool Linked::del_by_value(const string& value) {
    Node* node = find_node(value);
    if (!node) return false;
    unlink(node);
    return true;
}

size_t Linked::size() const {
    return sz;
}

bool Linked::empty() const {
    return sz == 0;
}

string Linked::read() const {
    stringstream ss;
    for (const Node* node = head; node; node = node->next) {
        ss << " " << node->data;
    }
    return ss.str();
}

string Linked::read_reverse() const {
    stringstream ss;
    for (const Node* node = tail; node; node = node->prev) {
        ss << " " << node->data;
    }
    return ss.str();
}

void Linked::serialize_binary(ostream& out) const {
    ser::write_tag(out, "LIN2");
    ser::write_size(out, sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_string(out, node->data);
    }
}

void Linked::deserialize_binary(istream& in) {
    ser::read_tag(in, "LIN2");
    const size_t n = ser::read_size(in);
    Linked tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_string(in));
    }
    *this = tmp;
}

void Linked::serialize_text(ostream& out) const {
    ser::write_text_header(out, "LinkedList", sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_text_string(out, node->data);
    }
}

void Linked::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, "LinkedList");
    Linked tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_text_string(in));
    }
    *this = tmp;
}
