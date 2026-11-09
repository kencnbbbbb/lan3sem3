#include "one_linked_list.h"

#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "serialize.h"

using namespace std;

OneLinked::Node::Node(string val, Node* nxt) : data(std::move(val)), next(nxt) {}

OneLinked::OneLinked() : head(nullptr), tail(nullptr), sz(0) {}

OneLinked::~OneLinked() {
    clear();
}

OneLinked::OneLinked(const OneLinked& other) : head(nullptr), tail(nullptr), sz(0) {
    copy_from(other);
}

OneLinked& OneLinked::operator=(const OneLinked& other) {
    if (this == &other) return *this;
    clear();
    copy_from(other);
    return *this;
}

void OneLinked::copy_from(const OneLinked& other) {
    for (const Node* node = other.head; node; node = node->next) {
        push_back(node->data);
    }
}

void OneLinked::clear() {
    while (head) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
    sz = 0;
}

OneLinked::Node* OneLinked::find_node(const string& value) const {
    Node* node = head;
    while (node && node->data != value) node = node->next;
    return node;
}

OneLinked::Node* OneLinked::find_prev(const string& value, bool& found) const {
    found = false;
    Node* prev = nullptr;
    for (Node* node = head; node; prev = node, node = node->next) {
        if (node->data == value) {
            found = true;
            return prev;
        }
    }
    return nullptr;
}

void OneLinked::remove_after(Node* prev) {
    Node* del = prev ? prev->next : head;
    if (prev) prev->next = del->next;
    else head = del->next;
    if (del == tail) tail = prev;
    delete del;
    sz--;
}

int OneLinked::find(const string& value) const {
    int count = 0;
    for (const Node* node = head; node; node = node->next, count++) {
        if (node->data == value) return count;
    }
    return -1;
}

void OneLinked::push_back(const string& data) {
    Node* node = new Node(data);
    if (tail) tail->next = node;
    else head = node;
    tail = node;
    sz++;
}

void OneLinked::push_front(const string& data) {
    head = new Node(data, head);
    if (!tail) tail = head;
    sz++;
}

void OneLinked::push_after(const string& prev_el, const string& data) {
    Node* node = find_node(prev_el);
    if (!node) throw invalid_argument("OneLinked::push_after: элемент не найден");
    node->next = new Node(data, node->next);
    if (node == tail) tail = node->next;
    sz++;
}

void OneLinked::push_before(const string& next_el, const string& data) {
    bool found = false;
    Node* prev = find_prev(next_el, found);
    if (!found) throw invalid_argument("OneLinked::push_before: элемент не найден");
    if (!prev) {
        push_front(data);
        return;
    }
    prev->next = new Node(data, prev->next);
    sz++;
}

string OneLinked::pop_front() {
    if (!head) throw out_of_range("OneLinked::pop_front: список пуст");
    string val = head->data;
    remove_after(nullptr);
    return val;
}

string OneLinked::pop_back() {
    if (!head) throw out_of_range("OneLinked::pop_back: список пуст");
    string val = tail->data;
    Node* prev = nullptr;
    for (Node* node = head; node != tail; node = node->next) prev = node;
    remove_after(prev);
    return val;
}

string OneLinked::pop_after(const string& prev_el) {
    Node* node = find_node(prev_el);
    if (!node) throw invalid_argument("OneLinked::pop_after: элемент не найден");
    if (!node->next) throw out_of_range("OneLinked::pop_after: после элемента пусто");
    string val = node->next->data;
    remove_after(node);
    return val;
}

string OneLinked::pop_before(const string& next_el) {
    Node* before_prev = nullptr;
    Node* prev = nullptr;
    for (Node* node = head; node; node = node->next) {
        if (node->data == next_el) {
            if (!prev) throw out_of_range("OneLinked::pop_before: перед элементом пусто");
            string val = prev->data;
            remove_after(before_prev);
            return val;
        }
        before_prev = prev;
        prev = node;
    }
    throw invalid_argument("OneLinked::pop_before: элемент не найден");
}

bool OneLinked::del_by_value(const string& value) {
    bool found = false;
    Node* prev = find_prev(value, found);
    if (!found) return false;
    remove_after(prev);
    return true;
}

size_t OneLinked::size() const {
    return sz;
}

bool OneLinked::empty() const {
    return sz == 0;
}

string OneLinked::read() const {
    stringstream ss;
    for (const Node* node = head; node; node = node->next) {
        ss << " " << node->data;
    }
    return ss.str();
}

string OneLinked::read_reverse() const {
    vector<const string*> items;
    items.reserve(sz);
    for (Node* node = head; node; node = node->next) {
        items.push_back(&node->data);
    }
    stringstream ss;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        ss << " " << **it;
    }
    return ss.str();
}

void OneLinked::serialize_binary(ostream& out) const {
    ser::write_tag(out, "LIN1");
    ser::write_size(out, sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_string(out, node->data);
    }
}

void OneLinked::deserialize_binary(istream& in) {
    ser::read_tag(in, "LIN1");
    const size_t n = ser::read_size(in);
    OneLinked tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_string(in));
    }
    *this = tmp;
}

void OneLinked::serialize_text(ostream& out) const {
    ser::write_text_header(out, "OneLinkedList", sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_text_string(out, node->data);
    }
}

void OneLinked::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, "OneLinkedList");
    OneLinked tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_text_string(in));
    }
    *this = tmp;
}
