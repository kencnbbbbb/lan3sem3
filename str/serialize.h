#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

using namespace std;

namespace ser {

constexpr uint64_t MAX_ITEMS = 1ULL << 24U;

void write_tag(ostream& out, const char (&tag)[5]);
void read_tag(istream& in, const char (&tag)[5]);

void write_size(ostream& out, size_t n);
size_t read_size(istream& in);

void write_string(ostream& out, const string& s);
string read_string(istream& in);

void write_int(ostream& out, int value);
int read_int(istream& in);

void write_text_header(ostream& out, const string& tag, size_t n);
size_t read_text_header(istream& in, const string& tag);

void write_text_string(ostream& out, const string& s);
string read_text_string(istream& in);

void write_text_int(ostream& out, int value);
int read_text_int(istream& in);

}

#endif
