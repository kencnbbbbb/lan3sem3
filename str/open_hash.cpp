#include "open_hash.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "serialize.h"

using namespace std;

namespace {

size_t round_up_pow2(size_t n) {
    size_t c = OpenHash::MIN_CAP;
    while (c < n) c <<= 1U;
    return c;
}

}

OpenHash::Node::Node() : status(Status::EMPTY) {}

OpenHash::OpenHash(size_t capacity, ProbeMode method) : mode(method), sz(0), deleted(0), cap(0), nodes(nullptr) {
    if (capacity > MAX_CAP) throw length_error("Слишком большой capacity");
    cap = round_up_pow2(capacity);
    nodes = new Node[cap];
}

OpenHash::~OpenHash() {
    delete[] nodes;
}

OpenHash::OpenHash(const OpenHash& other) : mode(other.mode), sz(0), deleted(0), cap(0), nodes(nullptr) {
    copy_from(other);
}

OpenHash& OpenHash::operator=(const OpenHash& other) {
    if (this == &other) return *this;
    Node* fresh = new Node[other.cap];
    for (size_t i = 0; i < other.cap; i++) {
        fresh[i] = other.nodes[i];
    }
    delete[] nodes;
    nodes = fresh;
    mode = other.mode;
    cap = other.cap;
    sz = other.sz;
    deleted = other.deleted;
    return *this;
}

void OpenHash::copy_from(const OpenHash& other) {
    nodes = new Node[other.cap];
    mode = other.mode;
    cap = other.cap;
    sz = other.sz;
    deleted = other.deleted;
    for (size_t i = 0; i < cap; i++) {
        nodes[i] = other.nodes[i];
    }
}

size_t OpenHash::hash_djb2(const string& key) {
    size_t hash = 5381;
    for (unsigned char c : key) {
        hash = ((hash << 5U) + hash) + c;
    }
    return hash;
}

size_t OpenHash::hash_fnv1a(const string& key) {
    size_t hash = 14695981039346656037ULL;
    for (unsigned char c : key) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
}

size_t OpenHash::hash_step(const string& key) const {
    return mode == ProbeMode::Linear ? 1U : (hash_fnv1a(key) % cap) | 1U;
}

size_t OpenHash::probe(size_t start, size_t step, size_t i) const {
    return (start + (i * step)) & (cap - 1);
}

int OpenHash::find(const string& value) const {
    if (cap == 0) return -1;
    const size_t start = hash_djb2(value);
    const size_t step = hash_step(value);
    for (size_t i = 0; i < cap; i++) {
        const size_t idx = probe(start, step, i);
        if (nodes[idx].status == Status::EMPTY) return -1;
        if (nodes[idx].status == Status::USED && nodes[idx].val == value) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

bool OpenHash::contains(const string& value) const {
    return find(value) != -1;
}

void OpenHash::place(const string& value) {
    const size_t start = hash_djb2(value);
    const size_t step = hash_step(value);
    for (size_t i = 0; i < cap; i++) {
        Node& node = nodes[probe(start, step, i)];
        if (node.status != Status::USED) {
            if (node.status == Status::DELETED) deleted--;
            node.status = Status::USED;
            node.val = value;
            sz++;
            return;
        }
    }
    throw length_error("Таблица переполнена");
}

void OpenHash::rehash(size_t new_cap) {
    if (new_cap > MAX_CAP) throw length_error("Объём превысил максимум");
    Node* old_nodes = nodes;
    const size_t old_cap = cap;

    nodes = new Node[new_cap];
    cap = new_cap;
    sz = 0;
    deleted = 0;
    for (size_t i = 0; i < old_cap; i++) {
        if (old_nodes[i].status == Status::USED) place(old_nodes[i].val);
    }
    delete[] old_nodes;
}

bool OpenHash::insert(const string& value) {
    if (contains(value)) return false;
    if (cap == 0) rehash(MIN_CAP);
    if (static_cast<double>(sz + deleted + 1) > static_cast<double>(cap) * MAX_LOAD) {
        const bool grow = static_cast<double>(sz + 1) > static_cast<double>(cap) * MAX_LOAD / 2;
        rehash(grow ? cap * 2 : cap);
    }
    place(value);
    return true;
}

bool OpenHash::erase(const string& value) {
    const int idx = find(value);
    if (idx == -1) return false;
    nodes[idx].status = Status::DELETED;
    nodes[idx].val.clear();
    sz--;
    deleted++;
    return true;
}

void OpenHash::clear() {
    for (size_t i = 0; i < cap; i++) {
        nodes[i] = Node();
    }
    sz = 0;
    deleted = 0;
}

size_t OpenHash::size() const {
    return sz;
}

size_t OpenHash::capacity() const {
    return cap;
}

bool OpenHash::empty() const {
    return sz == 0;
}

string OpenHash::read() const {
    stringstream ss;
    for (size_t i = 0; i < cap; i++) {
        if (nodes[i].status == Status::USED) ss << " " << nodes[i].val;
    }
    return ss.str();
}

void OpenHash::serialize_binary(ostream& out) const {
    ser::write_tag(out, mode == ProbeMode::Linear ? "HLNR" : "HOPN");
    ser::write_size(out, sz);
    for (size_t i = 0; i < cap; i++) {
        if (nodes[i].status == Status::USED) ser::write_string(out, nodes[i].val);
    }
}

void OpenHash::deserialize_binary(istream& in) {
    ser::read_tag(in, mode == ProbeMode::Linear ? "HLNR" : "HOPN");
    const size_t n = ser::read_size(in);
    OpenHash tmp(MIN_CAP, mode);
    for (size_t i = 0; i < n; i++) {
        tmp.insert(ser::read_string(in));
    }
    *this = tmp;
}

void OpenHash::serialize_text(ostream& out) const {
    ser::write_text_header(out, mode == ProbeMode::Linear ? "LinearHash" : "OpenHash", sz);
    for (size_t i = 0; i < cap; i++) {
        if (nodes[i].status == Status::USED) ser::write_text_string(out, nodes[i].val);
    }
}

void OpenHash::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, mode == ProbeMode::Linear ? "LinearHash" : "OpenHash");
    OpenHash tmp(MIN_CAP, mode);
    for (size_t i = 0; i < n; i++) {
        tmp.insert(ser::read_text_string(in));
    }
    *this = tmp;
}

ProbeMode OpenHash::method() const { return mode; }
