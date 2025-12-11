#include "../include/persistent_value.hpp"
#include <stdexcept>

// -----------------------------------------
// ---- Определения внутренних структур ----
// -----------------------------------------
struct PersistentValue::VectorHolder {
    std::shared_ptr<void> vectorPtr;
    ValueType elementType;
};

struct PersistentValue::ListHolder {
    std::shared_ptr<void> listPtr;
    ValueType elementType;
};

struct PersistentValue::MapHolder {
    std::shared_ptr<void> mapPtr;
    ValueType keyType;
    ValueType valueType;
};

// -----------------------------------------
// -------------- Конструкторы -------------
// -----------------------------------------
PersistentValue::PersistentValue() : data(std::monostate{}) {}

PersistentValue::PersistentValue(int value) : data(value) {}

PersistentValue::PersistentValue(double value) : data(value) {}

PersistentValue::PersistentValue(bool value) : data(value) {}

PersistentValue::PersistentValue(const std::string& value) : data(value) {}

PersistentValue::PersistentValue(const char* value) : data(std::string(value)) {}

// -----------------------------------------
// ------------ Получение типа -------------
// -----------------------------------------
ValueType PersistentValue::type() const {
    return static_cast<ValueType>(data.index());
}

// -----------------------------------------
// ------------ Проверки типов -------------
// -----------------------------------------
bool PersistentValue::isNull() const { return data.index() == 0; }
bool PersistentValue::isInt() const { return data.index() == 1; }
bool PersistentValue::isDouble() const { return data.index() == 2; }
bool PersistentValue::isBool() const { return data.index() == 3; }
bool PersistentValue::isString() const { return data.index() == 4; }
bool PersistentValue::isVector() const { return data.index() == 5; }
bool PersistentValue::isList() const { return data.index() == 6; }
bool PersistentValue::isMap() const { return data.index() == 7; }

// -----------------------------------------
// ---- Получение значений с проверкой -----
// -----------------------------------------
int PersistentValue::asInt() const {
    if (!isInt()) throw std::runtime_error("Not an integer");
    return std::get<int>(data);
}

double PersistentValue::asDouble() const {
    if (!isDouble()) throw std::runtime_error("Not a double");
    return std::get<double>(data);
}

bool PersistentValue::asBool() const {
    if (!isBool()) throw std::runtime_error("Not a boolean");
    return std::get<bool>(data);
}

std::string PersistentValue::asString() const {
    if (!isString()) throw std::runtime_error("Not a string");
    return std::get<std::string>(data);
}

// -----------------------------------------
// --------------- Сравнение ---------------
// -----------------------------------------
bool PersistentValue::operator==(const PersistentValue& other) const {
    return data == other.data;
}

bool PersistentValue::operator!=(const PersistentValue& other) const {
    return !(*this == other);
}