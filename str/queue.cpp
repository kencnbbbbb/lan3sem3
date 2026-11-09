#include "queue.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "serialize.h"

using namespace std;

Queue::Node::Node(string val, Node* nxt) : data(std::move(val)), next(nxt) {}

Queue::Queue() : head(nullptr), tail(nullptr), sz(0) {}

Queue::~Queue() {
    clear();
}

Queue::Queue(const Queue& other) : head(nullptr), tail(nullptr), sz(0) {
    copy_from(other);
}

Queue& Queue::operator=(const Queue& other) {
    if (this == &other) return *this;
    clear();
    copy_from(other);
    return *this;
}

void Queue::copy_from(const Queue& other) {
    for (const Node* node = other.head; node; node = node->next) {
        push(node->data);
    }
}

void Queue::clear() {
    while (head) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
    sz = 0;
}

void Queue::push(const string& value) {
    Node* node = new Node(value);
    if (tail) tail->next = node;
    else head = node;
    tail = node;
    sz++;
}

string Queue::pop() {
    if (!head) throw out_of_range("Queue::pop: очередь пуста");
    Node* node = head;
    string val = std::move(node->data);
    head = head->next;
    if (!head) tail = nullptr;
    delete node;
    sz--;
    return val;
}

const string& Queue::front() const {
    if (!head) throw out_of_range("Queue::front: очередь пуста");
    return head->data;
}

size_t Queue::size() const {
    return sz;
}

bool Queue::empty() const {
    return head == nullptr;
}

string Queue::read() const {
    stringstream ss;
    for (const Node* node = head; node; node = node->next) {
        ss << " " << node->data;
    }
    return ss.str();
}

void Queue::serialize_binary(ostream& out) const {
    ser::write_tag(out, "QUEU");
    ser::write_size(out, sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_string(out, node->data);
    }
}

void Queue::deserialize_binary(istream& in) {
    ser::read_tag(in, "QUEU");
    const size_t n = ser::read_size(in);
    Queue tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push(ser::read_string(in));
    }
    *this = tmp;
}

void Queue::serialize_text(ostream& out) const {
    ser::write_text_header(out, "Queue", sz);
    for (const Node* node = head; node; node = node->next) {
        ser::write_text_string(out, node->data);
    }
}

void Queue::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, "Queue");
    Queue tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push(ser::read_text_string(in));
    }
    *this = tmp;
}
