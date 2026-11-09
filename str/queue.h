#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <iosfwd>
#include <string>

using namespace std;

#include "serializable.h"

class Queue : public Serializable {
public:
    Queue();
    ~Queue() override;

    Queue(const Queue& other);
    Queue& operator=(const Queue& other);

    void push(const string& value);
    string pop();
    const string& front() const;
    void clear();

    size_t size() const;
    bool empty() const;
    string read() const;

    void serialize_binary(ostream& out) const override;
    void deserialize_binary(istream& in) override;
    void serialize_text(ostream& out) const override;
    void deserialize_text(istream& in) override;

private:
    struct Node {
        string data;
        Node* next;
        explicit Node(string val, Node* nxt = nullptr);
    };
    Node* head;
    Node* tail;
    size_t sz;

    void copy_from(const Queue& other);
};

#endif
