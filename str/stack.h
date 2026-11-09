#ifndef STACK_H
#define STACK_H

#include <cstddef>
#include <iosfwd>
#include <string>

using namespace std;

#include "serializable.h"

class Stack : public Serializable {
public:
    Stack();
    ~Stack() override;

    Stack(const Stack& other);
    Stack& operator=(const Stack& other);

    void push(const string& value);
    string pop();
    const string& top() const;
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
    size_t sz;

    void copy_from(const Stack& other);
};

#endif
