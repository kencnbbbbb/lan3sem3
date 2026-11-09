#ifndef LAB3_SERIALIZABLE_H_
#define LAB3_SERIALIZABLE_H_
#include <iosfwd>
// Общий интерфейс; виртуальный деструктор обеспечивает корректное удаление через базовый указатель.
class Serializable {
 public:
    virtual ~Serializable() = default;
    virtual void serialize_binary(std::ostream& out) const = 0;
    virtual void deserialize_binary(std::istream& in) = 0;
    virtual void serialize_text(std::ostream& out) const = 0;
    virtual void deserialize_text(std::istream& in) = 0;
};
#endif  // LAB3_SERIALIZABLE_H_
