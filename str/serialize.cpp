#include "serialize.h"

#include <cstring>
#include <cctype>
#include <limits>
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>

using namespace std;

namespace ser {

namespace {

void check_read(const istream& in, const char* what) {
    if (!in) {
        throw runtime_error(string("Ошибка чтения: ") + what);
    }
}

}

void write_tag(ostream& out, const char (&tag)[5]) {
    out.write(tag, 4);
    if (!out) throw runtime_error("Ошибка записи тега");
}

void read_tag(istream& in, const char (&tag)[5]) {
    char buf[4] = {};
    in.read(buf, 4);
    check_read(in, "тег");
    if (memcmp(buf, tag, 4) != 0) {
        throw runtime_error(string("Неверный тег, ожидался ") + tag);
    }
}

void write_size(ostream& out, size_t n) {
    if (n > MAX_ITEMS) throw runtime_error("Слишком большой размер");
    const auto value = static_cast<uint64_t>(n);
    for (unsigned i = 0; i < 8; ++i) out.put(static_cast<char>((value >> (i * 8U)) & 255U));
    if (!out) throw runtime_error("Ошибка записи размера");
}
size_t read_size(istream& in) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) {
        const int byte = in.get();
        check_read(in, "размер");
        value |= static_cast<uint64_t>(static_cast<unsigned char>(byte)) << (i * 8U);
    }
    if (value > MAX_ITEMS) throw runtime_error("Слишком большой размер в файле");
    return static_cast<size_t>(value);
}

void write_string(ostream& out, const string& s) {
    write_size(out, s.size());
    out.write(s.data(), static_cast<streamsize>(s.size()));
    if (!out) throw runtime_error("Ошибка записи строки");
}

string read_string(istream& in) {
    const size_t len = read_size(in);
    string s(len, '\0');
    in.read(s.data(), static_cast<streamsize>(len));
    check_read(in, "строка");
    return s;
}

void write_int(ostream& out, int value) {
    static_assert(sizeof(int) == sizeof(int32_t), "32-bit int required");
    const auto bits = static_cast<uint32_t>(value);
    for (unsigned i = 0; i < 4; ++i) out.put(static_cast<char>((bits >> (i * 8U)) & 255U));
    if (!out) throw runtime_error("Ошибка записи числа");
}
int read_int(istream& in) {
    uint32_t bits = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const int byte = in.get(); check_read(in, "число");
        bits |= static_cast<uint32_t>(static_cast<unsigned char>(byte)) << (i * 8U);
    }
    int32_t value = 0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void write_text_header(ostream& out, const string& tag, size_t n) {
    if (n > MAX_ITEMS) throw runtime_error("Слишком большой размер");
    out << tag << ' ' << n << '\n';
    if (!out) throw runtime_error("Ошибка записи заголовка");
}

size_t read_text_header(istream& in, const string& tag) {
    string got;
    uint64_t n = 0;
    in >> got >> n;
    check_read(in, "заголовок");
    if (got != tag) {
        throw runtime_error("Неверный заголовок, ожидался " + tag);
    }
    if (n > MAX_ITEMS) {
        throw runtime_error("Слишком большой размер в файле");
    }
    return static_cast<size_t>(n);
}

void write_text_string(ostream& out, const string& s) {
    out << quoted(s) << '\n';
    if (!out) throw runtime_error("Ошибка записи строки");
}

string read_text_string(istream& in) {
    in >> ws;
    if (in.get() != '"') throw runtime_error("Ожидалась строка в кавычках");
    string result;
    for (;;) {
        int byte = in.get(); check_read(in, "строка");
        if (byte == '"') return result;
        if (byte == '\\') { byte = in.get(); check_read(in, "экранирование"); }
        if (result.size() >= MAX_ITEMS) throw runtime_error("Слишком длинная строка");
        result.push_back(static_cast<char>(byte));
    }
}

void write_text_int(ostream& out, int value) {
    out << value << '\n';
    if (!out) throw runtime_error("Ошибка записи числа");
}

int read_text_int(istream& in) {
    int v = 0;
    in >> v;
    check_read(in, "число");
    return v;
}

}
