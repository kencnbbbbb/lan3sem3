#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <cstddef>
#include <iosfwd>
#include <string>

using namespace std;

#include "serializable.h"

class Linked : public Serializable {
public:
    Linked();
    ~Linked() override;

    Linked(const Linked& other);
    Linked& operator=(const Linked& other);

    int find(const string& value) const;

    void push_back(const string& data);
    void push_front(const string& data);
    void push_after(const string& prev_el, const string& data);
    void push_before(const string& next_el, const string& data);

    string pop_back();
    string pop_front();
    string pop_before(const string& next_el);
    string pop_after(const string& prev_el);

    bool del_by_value(const string& value);
    void clear();

    size_t size() const;
    bool empty() const;
    string read() const;
    string read_reverse() const;

    void serialize_binary(ostream& out) const override;
    void deserialize_binary(istream& in) override;
    void serialize_text(ostream& out) const override;
    void deserialize_text(istream& in) override;

private:
    struct Node {
        string data;
        Node* next;
        Node* prev;
        explicit Node(string val, Node* nxt = nullptr, Node* prv = nullptr);
    };
    Node* head;
    Node* tail;
    size_t sz;

    Node* find_node(const string& value) const;
    void unlink(Node* node);
    void copy_from(const Linked& other);
};

#endif
