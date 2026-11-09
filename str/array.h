#ifndef ARRAY_H
#define ARRAY_H

#include <cstddef>
#include <iosfwd>
#include <string>

using namespace std;

#include "serializable.h"

class Array : public Serializable {
public:
    static constexpr size_t MIN_CAP = 16;
    static constexpr size_t MAX_CAP = 65536;

    Array();
    explicit Array(size_t capacity);
    ~Array() override;

    Array(const Array& other);
    Array& operator=(const Array& other);

    void push_back(const string& value);
    void insert(size_t idx, const string& value);
    string get(size_t idx) const;
    void erase(size_t idx);
    void set(size_t idx, const string& value);
    void clear();

    size_t length() const;
    size_t capacity() const;
    bool empty() const;
    string read() const;

    void serialize_binary(ostream& out) const override;
    void deserialize_binary(istream& in) override;
    void serialize_text(ostream& out) const override;
    void deserialize_text(istream& in) override;

private:
    size_t sz;
    size_t cap;
    string* data;

    void increase_capacity();
    void test_capacity();
    void copy_from(const Array& other);
};

#endif
