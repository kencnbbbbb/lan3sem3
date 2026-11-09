#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "str/array.h"
#include "str/linked_list.h"
#include "str/one_linked_list.h"
#include "str/open_hash.h"
#include "str/queue.h"
#include "str/stack.h"
#include "str/tree.h"

namespace {
std::unique_ptr<Serializable> create(const std::string& type) {
    if (type == "Array") return std::make_unique<Array>();
    if (type == "OneLinked") return std::make_unique<OneLinked>();
    if (type == "Linked") return std::make_unique<Linked>();
    if (type == "Stack") return std::make_unique<Stack>();
    if (type == "Queue") return std::make_unique<Queue>();
    if (type == "OpenHash") return std::make_unique<OpenHash>();
    if (type == "LinearHash") return std::make_unique<OpenHash>(16, ProbeMode::Linear);
    if (type == "Tree") return std::make_unique<Tree>();
    throw std::invalid_argument("Неизвестный тип структуры");
}
void write(const Serializable& obj, const std::string& format, const std::string& path) {
    if (format != "text" && format != "binary") throw std::invalid_argument("Формат: text или binary");
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Не удалось открыть файл записи");
    if (format == "binary") obj.serialize_binary(out); else obj.serialize_text(out);
    out.flush();
    if (!out) throw std::runtime_error("Ошибка записи файла");
}
void convert(const std::string& type, const std::string& from, const std::string& input,
             const std::string& to, const std::string& output) {
    if ((from != "text" && from != "binary") || (to != "text" && to != "binary")) {
        throw std::invalid_argument("Формат: text или binary");
    }
    auto obj = create(type);
    std::ifstream in(input, std::ios::binary);
    if (!in) throw std::runtime_error("Не удалось открыть исходный файл");
    if (from == "binary") obj->deserialize_binary(in); else obj->deserialize_text(in);
    if (from == "text") in >> std::ws;
    if (in.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Лишние данные после структуры");
    write(*obj, to, output);
}
void demo(const std::string& dir) {
    std::filesystem::create_directories(dir);
    Array a; OneLinked one; Linked two; Stack stack; Queue queue;
    OpenHash hash; OpenHash linear(16, ProbeMode::Linear); Tree tree;
    for (const std::string value : {"alpha", "two words", "", "q\"uote", "Привет"}) {
        a.push_back(value); one.push_back(value); two.push_back(value);
        stack.push(value); queue.push(value); hash.insert(value); linear.insert(value);
    }
    for (int value : {5, 3, 8, 3, -7, 0, 11, 5}) tree.insert(value);
    const std::pair<std::string, const Serializable*> objects[] = {
        {"Array", &a}, {"OneLinked", &one}, {"Linked", &two}, {"Stack", &stack},
        {"Queue", &queue}, {"OpenHash", &hash}, {"LinearHash", &linear}, {"Tree", &tree}};
    for (const auto& item : objects) {
        write(*item.second, "text", dir + "/" + item.first + ".txt");
        write(*item.second, "binary", dir + "/" + item.first + ".bin");
    }
    std::cout << "Созданы текстовые и бинарные примеры для 8 структур\n";
}
}  // namespace
int main(int argc, char* argv[]) {
    try {
        if (argc == 3 && std::string(argv[1]) == "--demo") { demo(argv[2]); return 0; }
        if (argc == 7 && std::string(argv[1]) == "--convert") {
            convert(argv[2], argv[3], argv[4], argv[5], argv[6]); return 0;
        }
        if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--help")) {
            std::cout << "lab3 --demo папка\nlab3 --convert Тип text|binary вход text|binary выход\n";
            return 0;
        }
        throw std::invalid_argument("Неверные параметры; используйте --help");
    } catch (const std::exception& e) { std::cerr << "Ошибка: " << e.what() << '\n'; return 1; }
}
