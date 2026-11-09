#include "tree.h"
#include <ostream>
#include <queue>
#include <sstream>
#include <utility>
#include "serialize.h"

Tree::Node::Node(int value) : data(value), left(nullptr), right(nullptr) {}
Tree::Tree() : root(nullptr), sz(0) {}
void Tree::destroy(Node* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}
Tree::~Tree() { destroy(root); }
Tree::Tree(const Tree& other) : Tree() {
    std::queue<const Node*> pending;
    if (other.root) pending.push(other.root);
    try {
        while (!pending.empty()) {
            const Node* node = pending.front(); pending.pop();
            insert(node->data);
            if (node->left) pending.push(node->left);
            if (node->right) pending.push(node->right);
        }
    } catch (...) { clear(); throw; }
}
Tree& Tree::operator=(const Tree& other) {
    if (this == &other) return *this;
    Tree copy(other);
    std::swap(root, copy.root);
    std::swap(sz, copy.sz);
    return *this;
}
bool Tree::insert(int value) {
    if (!root) { root = new Node(value); ++sz; return true; }
    std::queue<Node*> pending; pending.push(root);
    while (!pending.empty()) {
        Node* node = pending.front(); pending.pop();
        if (!node->left) { node->left = new Node(value); ++sz; return true; }
        if (!node->right) { node->right = new Node(value); ++sz; return true; }
        pending.push(node->left); pending.push(node->right);
    }
    return false;
}
bool Tree::is_member(int value) const {
    std::queue<const Node*> pending;
    if (root) pending.push(root);
    while (!pending.empty()) {
        const Node* node = pending.front(); pending.pop();
        if (node->data == value) return true;
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
    return false;
}
bool Tree::is_complete() const {
    std::queue<const Node*> pending; pending.push(root);
    bool gap = false;
    while (!pending.empty()) {
        const Node* node = pending.front(); pending.pop();
        if (!node) { gap = true; continue; }
        if (gap) return false;
        pending.push(node->left); pending.push(node->right);
    }
    return true;
}
void Tree::clear() { destroy(root); root = nullptr; sz = 0; }
std::size_t Tree::size() const { return sz; }
bool Tree::empty() const { return root == nullptr; }
int Tree::height() const {
    std::size_t n = sz; int result = 0;
    while (n) { ++result; n >>= 1U; }
    return result;
}
std::string Tree::read() const {
    std::ostringstream out;
    std::queue<const Node*> pending;
    if (root) pending.push(root);
    while (!pending.empty()) {
        const Node* node = pending.front(); pending.pop();
        out << ' ' << node->data;
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
    return out.str();
}
std::string Tree::structure() const {
    std::ostringstream out;
    std::queue<std::pair<const Node*, std::string>> pending;
    if (root) pending.push({root, "root"});
    while (!pending.empty()) {
        const auto item = pending.front(); pending.pop();
        out << item.second << " = " << item.first->data << '\n';
        if (item.first->left) pending.push({item.first->left, item.second + "->left"});
        if (item.first->right) pending.push({item.first->right, item.second + "->right"});
    }
    return out.str();
}
void Tree::print(std::ostream& out) const { out << structure(); }
void Tree::serialize_binary(std::ostream& out) const {
    ser::write_tag(out, "CTRE"); ser::write_size(out, sz);
    std::queue<const Node*> pending;
    if (root) pending.push(root);
    while (!pending.empty()) {
        const Node* node = pending.front(); pending.pop();
        ser::write_int(out, node->data);
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
}
void Tree::deserialize_binary(std::istream& in) {
    ser::read_tag(in, "CTRE"); const auto n = ser::read_size(in);
    Tree tmp;
    for (std::size_t i = 0; i < n; ++i) tmp.insert(ser::read_int(in));
    *this = tmp;
}
void Tree::serialize_text(std::ostream& out) const {
    ser::write_text_header(out, "CompleteTree", sz);
    std::queue<const Node*> pending;
    if (root) pending.push(root);
    while (!pending.empty()) {
        const Node* node = pending.front(); pending.pop();
        ser::write_text_int(out, node->data);
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
}
void Tree::deserialize_text(std::istream& in) {
    const auto n = ser::read_text_header(in, "CompleteTree"); Tree tmp;
    for (std::size_t i = 0; i < n; ++i) tmp.insert(ser::read_text_int(in));
    *this = tmp;
}
