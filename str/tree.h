#ifndef LAB3_TREE_H_
#define LAB3_TREE_H_
#include <cstddef>
#include <iosfwd>
#include <string>
#include "serializable.h"
// Complete Binary Tree из ЛР1: заполнение по уровням, повторные значения допустимы.
class Tree : public Serializable {
 public:
    Tree();
    ~Tree() override;
    Tree(const Tree& other);
    Tree& operator=(const Tree& other);
    bool insert(int value);
    bool is_member(int value) const;
    bool is_complete() const;
    void clear();
    std::size_t size() const;
    bool empty() const;
    int height() const;
    std::string read() const;
    std::string structure() const;
    void print(std::ostream& out) const;
    void serialize_binary(std::ostream& out) const override;
    void deserialize_binary(std::istream& in) override;
    void serialize_text(std::ostream& out) const override;
    void deserialize_text(std::istream& in) override;
 private:
    struct Node {
        int data;
        Node* left;
        Node* right;
        explicit Node(int value);
    };
    Node* root;
    std::size_t sz;
    static void destroy(Node* node);
};
#endif  // LAB3_TREE_H_
