#ifndef LAB3_VARIANT_CHECKS_H_
#define LAB3_VARIANT_CHECKS_H_
#include <algorithm>
#include <stdexcept>
#include <streambuf>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include "../str/open_hash.h"
#include "../str/array.h"
#include "../str/tree.h"
#include "../str/linked_list.h"
#include "../str/one_linked_list.h"
#include "../str/stack.h"
#include "../str/queue.h"
#include "../str/serialize.h"
// Один независимый эталон выполняется каждым настоящим тестовым фреймворком.
template <typename Check>
void check_hash_methods(Check check) {
    for (const auto mode : {ProbeMode::Linear, ProbeMode::Double}) {
        OpenHash hash(17, mode);
        std::unordered_set<std::string> reference;
        std::mt19937 rng(5);
        for (int i = 0; i < 5000; ++i) {
            const auto key = std::to_string(rng() % 300);
            switch (rng() % 3) {
                case 0: check(hash.insert(key) == reference.insert(key).second); break;
                case 1: check(hash.erase(key) == (reference.erase(key) != 0)); break;
                default: check(hash.contains(key) == (reference.count(key) != 0)); break;
            }
        }
        check(hash.size() == reference.size());
        for (const auto& key : reference) check(hash.contains(key));
        OpenHash copy(hash); check(copy.method() == mode);
        OpenHash assigned; assigned = hash; check(assigned.method() == mode);
        std::stringstream bin; hash.serialize_binary(bin);
        OpenHash restored(16, mode); restored.deserialize_binary(bin);
        check(restored.size() == hash.size()); check(restored.method() == mode);
        std::stringstream text; hash.serialize_text(text); restored.deserialize_text(text);
        for (const auto& key : reference) check(restored.contains(key));
        const auto before = restored.read();
        std::stringstream bad("Broken 3\n"); bool failed = false;
        try { restored.deserialize_text(bad); } catch (const std::runtime_error&) { failed = true; }
        check(failed); check(restored.read() == before);
    }
    // Ошибка записи должна распространяться вызывающему коду.
    Array array; array.push_back("two words");
    std::stringstream broken; broken.setstate(std::ios::badbit);
    bool failed = false;
    try { array.serialize_binary(broken); } catch (const std::runtime_error&) { failed = true; }
    check(failed);
    // Виртуальный интерфейс используется с разными конкретными объектами.
    const Serializable& object = array;
    std::stringstream data; object.serialize_text(data); check(!data.str().empty());
}

// Ошибки потока на каждой позиции, включая запись заголовка, длины и содержимого.
class LimitedBuffer : public std::streambuf {
 public:
    explicit LimitedBuffer(std::size_t n) : remaining(n) {}
 protected:
    std::streamsize xsputn(const char*, std::streamsize n) override {
        const auto count = std::min<std::size_t>(remaining, static_cast<std::size_t>(n));
        remaining -= count;
        return static_cast<std::streamsize>(count);
    }
    int_type overflow(int_type ch) override {
        if (traits_type::eq_int_type(ch, traits_type::eof())) return traits_type::not_eof(ch);
        if (!remaining) return traits_type::eof();
        --remaining; return ch;
    }
 private:
    std::size_t remaining;
};
template <typename T, typename Check>
void check_serialization_failures(const T& source, Check check) {
    std::stringstream binary, text;
    source.serialize_binary(binary); source.serialize_text(text);
    for (bool bin : {true, false}) {
        const auto raw = bin ? binary.str() : text.str();
        for (std::size_t limit = 0; limit < raw.size(); ++limit) {
            LimitedBuffer buffer(limit); std::ostream out(&buffer); bool failed = false;
            try { if (bin) source.serialize_binary(out); else source.serialize_text(out); }
            catch (const std::runtime_error&) { failed = true; }
            check(failed);
        }
    }
    for (std::size_t cut = 0; cut < binary.str().size(); ++cut) {
        T destination(source); const auto before = destination.read();
        std::stringstream input(binary.str().substr(0, cut)); bool failed = false;
        try { destination.deserialize_binary(input); } catch (const std::runtime_error&) { failed = true; }
        check(failed); check(destination.read() == before);
    }
}

#endif  // LAB3_VARIANT_CHECKS_H_
