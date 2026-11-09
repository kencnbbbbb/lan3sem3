#include "array.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "serialize.h"

using namespace std;

Array::Array() : sz(0), cap(MIN_CAP), data(new string[MIN_CAP]) {}

Array::Array(size_t capacity)
    : sz(0),
      cap(capacity < MIN_CAP ? MIN_CAP : capacity),
      data(nullptr) {
    if (capacity > MAX_CAP) {
        throw length_error("Объём превысил максимум");
    }
    data = new string[cap];
}

Array::~Array() {
    delete[] data;
}

Array::Array(const Array& other) : sz(0), cap(0), data(nullptr) {
    copy_from(other);
}

Array& Array::operator=(const Array& other) {
    if (this == &other) return *this;
    auto* fresh = new string[other.cap];
    for (size_t i = 0; i < other.sz; i++) {
        fresh[i] = other.data[i];
    }
    delete[] data;
    data = fresh;
    cap = other.cap;
    sz = other.sz;
    return *this;
}

void Array::copy_from(const Array& other) {
    data = new string[other.cap];
    cap = other.cap;
    sz = other.sz;
    for (size_t i = 0; i < sz; i++) {
        data[i] = other.data[i];
    }
}

void Array::increase_capacity() {
    size_t new_cap = (cap == 0) ? MIN_CAP : cap * 2;
    if (new_cap > MAX_CAP) {
        throw length_error("Объём превысил максимум");
    }
    auto* new_arr = new string[new_cap];
    for (size_t i = 0; i < sz; i++) {
        new_arr[i] = std::move(data[i]);
    }
    delete[] data;
    data = new_arr;
    cap = new_cap;
}

void Array::test_capacity() {
    if (sz >= cap) {
        increase_capacity();
    }
}

void Array::push_back(const string& value) {
    test_capacity();
    data[sz++] = value;
}

void Array::insert(size_t idx, const string& value) {
    if (idx > sz) throw out_of_range("Array::insert: индекс вне диапазона");
    test_capacity();
    for (size_t i = sz; i > idx; i--) {
        data[i] = std::move(data[i - 1]);
    }
    data[idx] = value;
    sz++;
}

string Array::get(size_t idx) const {
    if (idx >= sz) throw out_of_range("Array::get: индекс вне диапазона");
    return data[idx];
}

void Array::erase(size_t idx) {
    if (idx >= sz) throw out_of_range("Array::erase: индекс вне диапазона");
    for (size_t i = idx; i + 1 < sz; i++) {
        data[i] = std::move(data[i + 1]);
    }
    sz--;
    data[sz].clear();
}

void Array::set(size_t idx, const string& value) {
    if (idx >= sz) throw out_of_range("Array::set: индекс вне диапазона");
    data[idx] = value;
}

void Array::clear() {
    for (size_t i = 0; i < sz; i++) {
        data[i].clear();
    }
    sz = 0;
}

size_t Array::length() const {
    return sz;
}

size_t Array::capacity() const {
    return cap;
}

bool Array::empty() const {
    return sz == 0;
}

string Array::read() const {
    stringstream ss;
    for (size_t i = 0; i < sz; i++) {
        ss << " " << data[i];
    }
    return ss.str();
}

void Array::serialize_binary(ostream& out) const {
    ser::write_tag(out, "ARRY");
    ser::write_size(out, sz);
    for (size_t i = 0; i < sz; i++) {
        ser::write_string(out, data[i]);
    }
}

void Array::deserialize_binary(istream& in) {
    ser::read_tag(in, "ARRY");
    const size_t n = ser::read_size(in);
    Array tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_string(in));
    }
    *this = tmp;
}

void Array::serialize_text(ostream& out) const {
    ser::write_text_header(out, "Array", sz);
    for (size_t i = 0; i < sz; i++) {
        ser::write_text_string(out, data[i]);
    }
}

void Array::deserialize_text(istream& in) {
    const size_t n = ser::read_text_header(in, "Array");
    Array tmp;
    for (size_t i = 0; i < n; i++) {
        tmp.push_back(ser::read_text_string(in));
    }
    *this = tmp;
}
