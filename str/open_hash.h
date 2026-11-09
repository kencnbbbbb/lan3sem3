#ifndef OPEN_HASH_H
#define OPEN_HASH_H

#include <cstddef>
#include <iosfwd>
#include <string>

using namespace std;

#include "serializable.h"

enum class ProbeMode { Linear, Double };

class OpenHash : public Serializable {
public:
    static constexpr size_t MIN_CAP = 16;
    static constexpr size_t MAX_CAP = 65536;

    explicit OpenHash(size_t capacity = MIN_CAP, ProbeMode mode = ProbeMode::Double);
    ProbeMode method() const;
    ~OpenHash() override;

    OpenHash(const OpenHash& other);
    OpenHash& operator=(const OpenHash& other);

    int find(const string& value) const;
    bool contains(const string& value) const;
    bool insert(const string& value);
    bool erase(const string& value);
    void clear();

    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    string read() const;

    void serialize_binary(ostream& out) const override;
    void deserialize_binary(istream& in) override;
    void serialize_text(ostream& out) const override;
    void deserialize_text(istream& in) override;

    static size_t hash_djb2(const string& key);
    static size_t hash_fnv1a(const string& key);

private:
    enum class Status { EMPTY, USED, DELETED };

    struct Node {
        Status status;
        string val;
        Node();
    };

    static constexpr double MAX_LOAD = 0.7;

    ProbeMode mode;
    size_t sz;
    size_t deleted;
    size_t cap;
    Node* nodes;

    size_t hash_step(const string& key) const;
    size_t probe(size_t start, size_t step, size_t i) const;
    void place(const string& value);
    void rehash(size_t new_cap);
    void copy_from(const OpenHash& other);
};

#endif
