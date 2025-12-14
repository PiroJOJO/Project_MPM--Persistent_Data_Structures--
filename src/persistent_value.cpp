#include "persistent_value.hpp"
#include "persistent_vector.hpp"
#include "persistent_list.hpp"
#include "persistent_map.hpp"

#include <stdexcept>
#include <sstream>
#include <typeinfo>
#include <iostream>

using namespace std;

// -----------------------------------------
// -------------- Конструкторы -------------
// -----------------------------------------
template<typename T>
PersistentValue::PersistentValue(const PersistentVector<T>& vector)
    : data(make_shared<VectorHolder>(make_shared<PersistentVector<T>>(vector))) {
}

template<typename T>
PersistentValue::PersistentValue(const PersistentList<T>& list)
    : data(make_shared<ListHolder>(make_shared<PersistentList<T>>(list))) {
}

template<typename K, typename V>
PersistentValue::PersistentValue(const PersistentMap<K, V>& map)
    : data(make_shared<MapHolder>(make_shared<PersistentMap<K, V>>(map))) {
}

template<typename T>
PersistentValue::PersistentValue(shared_ptr<PersistentVector<T>> vector)
    : data(make_shared<VectorHolder>(vector)) {
}

template<typename T>
PersistentValue::PersistentValue(shared_ptr<PersistentList<T>> list)
    : data(make_shared<ListHolder>(list)) {
}

template<typename K, typename V>
PersistentValue::PersistentValue(shared_ptr<PersistentMap<K, V>> map)
    : data(make_shared<MapHolder>(map)) {
}

template PersistentValue::PersistentValue(const PersistentVector<int>&);
template PersistentValue::PersistentValue(const PersistentVector<double>&);
template PersistentValue::PersistentValue(const PersistentVector<string>&);
template PersistentValue::PersistentValue(const PersistentVector<PersistentValue>&);

template PersistentValue::PersistentValue(const PersistentList<int>&);
template PersistentValue::PersistentValue(const PersistentList<double>&);
template PersistentValue::PersistentValue(const PersistentList<string>&);
template PersistentValue::PersistentValue(const PersistentList<PersistentValue>&);

template PersistentValue::PersistentValue(const PersistentMap<string, int>&);
template PersistentValue::PersistentValue(const PersistentMap<string, double>&);
template PersistentValue::PersistentValue(const PersistentMap<string, string>&);
template PersistentValue::PersistentValue(const PersistentMap<string, PersistentValue>&);

// -----------------------------------------
// ---- Получение значений с проверкой -----
// -----------------------------------------
template<typename T>
shared_ptr<PersistentVector<T>> PersistentValue::asVector() const {
    if (!isVector()) {
        throw runtime_error("Not a vector");
    }
    auto holder = std::get<shared_ptr<VectorHolder>>(data);
    auto ptr = holder->get<T>();
    if (!ptr) {
        throw runtime_error("Vector type mismatch");
    }
    return ptr;
}

template<typename T>
shared_ptr<PersistentList<T>> PersistentValue::asList() const {
    if (!isList()) {
        throw runtime_error("Not a list");
    }
    auto holder = std::get<shared_ptr<ListHolder>>(data);
    auto ptr = holder->get<T>();
    if (!ptr) {
        throw runtime_error("List type mismatch");
    }
    return ptr;
}

template<typename K, typename V>
shared_ptr<PersistentMap<K, V>> PersistentValue::asMap() const {
    if (!isMap()) {
        throw runtime_error("Not a map");
    }
    auto holder = std::get<shared_ptr<MapHolder>>(data);
    auto ptr = holder->get<K, V>();
    if (!ptr) {
        throw runtime_error("Map type mismatch");
    }
    return ptr;
}

template shared_ptr<PersistentVector<int>> PersistentValue::asVector<int>() const;
template shared_ptr<PersistentVector<double>> PersistentValue::asVector<double>() const;
template shared_ptr<PersistentVector<string>> PersistentValue::asVector<string>() const;
template shared_ptr<PersistentVector<PersistentValue>> PersistentValue::asVector<PersistentValue>() const;

template shared_ptr<PersistentList<int>> PersistentValue::asList<int>() const;
template shared_ptr<PersistentList<double>> PersistentValue::asList<double>() const;
template shared_ptr<PersistentList<string>> PersistentValue::asList<string>() const;
template shared_ptr<PersistentList<PersistentValue>> PersistentValue::asList<PersistentValue>() const;

template shared_ptr<PersistentMap<string, int>> PersistentValue::asMap<string, int>() const;
template shared_ptr<PersistentMap<string, double>> PersistentValue::asMap<string, double>() const;
template shared_ptr<PersistentMap<string, string>> PersistentValue::asMap<string, string>() const;
template shared_ptr<PersistentMap<string, PersistentValue>> PersistentValue::asMap<string, PersistentValue>() const;

// -----------------------------------------
// -- Реализация вспомогательных методов ---
// -----------------------------------------
type_index PersistentValue::getElementType() const {
    if (isVector()) {
        auto holder = std::get<shared_ptr<VectorHolder>>(data);
        return holder->getElementType();
    }
    if (isList()) {
        auto holder = std::get<shared_ptr<ListHolder>>(data);
        return holder->getElementType();
    }
    throw runtime_error("Not a collection type");
}

type_index PersistentValue::getKeyType() const {
    if (isMap()) {
        auto holder = std::get<shared_ptr<MapHolder>>(data);
        return holder->getKeyType();
    }
    throw runtime_error("Not a map");
}

type_index PersistentValue::getValueType() const {
    if (isMap()) {
        auto holder = std::get<shared_ptr<MapHolder>>(data);
        return holder->getValueType();
    }
    throw runtime_error("Not a map");
}

PersistentValue PersistentValue::clone() const {
    if (isInt()) return PersistentValue(asInt());
    if (isDouble()) return PersistentValue(asDouble());
    if (isBool()) return PersistentValue(asBool());
    if (isString()) return PersistentValue(asString());

    if (isVector()) {
        throw runtime_error("Vector clone not implemented");
    }
    if (isList()) {
        throw runtime_error("List clone not implemented");
    }
    if (isMap()) {
        throw runtime_error("Map clone not implemented");
    }

    return PersistentValue();
}

string PersistentValue::toString() const {
    ostringstream oss;

    if (isNull()) {
        oss << "null";
    }
    else if (isInt()) {
        oss << asInt();
    }
    else if (isDouble()) {
        oss << asDouble();
    }
    else if (isBool()) {
        oss << (asBool() ? "true" : "false");
    }
    else if (isString()) {
        oss << '"' << asString() << '"';
    }
    else if (isVector()) {
        oss << "[Vector<" << getElementType().name() << ">]";
    }
    else if (isList()) {
        oss << "[List<" << getElementType().name() << ">]";
    }
    else if (isMap()) {
        oss << "[Map<" << getKeyType().name() << ", " << getValueType().name() << ">]";
    }

    return oss.str();
}

// -----------------------------------------
// ------------ Проверки типов -------------
// -----------------------------------------

PersistentValue::PersistentValue() : data(monostate{}) {}

PersistentValue::PersistentValue(int value) : data(value) {}

PersistentValue::PersistentValue(double value) : data(value) {}

PersistentValue::PersistentValue(bool value) : data(value) {}

PersistentValue::PersistentValue(const string& value) : data(value) {}

PersistentValue::PersistentValue(const char* value) : data(string(value)) {}

ValueType PersistentValue::type() const {
    return static_cast<ValueType>(data.index());
}

bool PersistentValue::isNull() const { return data.index() == 0; }
bool PersistentValue::isInt() const { return data.index() == 1; }
bool PersistentValue::isDouble() const { return data.index() == 2; }
bool PersistentValue::isBool() const { return data.index() == 3; }
bool PersistentValue::isString() const { return data.index() == 4; }
bool PersistentValue::isVector() const { return data.index() == 5; }
bool PersistentValue::isList() const { return data.index() == 6; }
bool PersistentValue::isMap() const { return data.index() == 7; }

int PersistentValue::asInt() const {
    if (!isInt()) throw runtime_error("Not an integer");
    return std::get<int>(data);
}

double PersistentValue::asDouble() const {
    if (!isDouble()) throw runtime_error("Not a double");
    return std::get<double>(data);
}

bool PersistentValue::asBool() const {
    if (!isBool()) throw runtime_error("Not a boolean");
    return std::get<bool>(data);
}

string PersistentValue::asString() const {
    if (!isString()) throw runtime_error("Not a string");
    return std::get<string>(data);
}

// -----------------------------------------
// --------------- Сравнение ---------------
// -----------------------------------------
bool PersistentValue::operator==(const PersistentValue& other) const {
    if (type() != other.type()) return false;

    switch (type()) {
    case ValueType::NULL_VALUE: return true;
    case ValueType::INT: return asInt() == other.asInt();
    case ValueType::DOUBLE: return asDouble() == other.asDouble();
    case ValueType::BOOL: return asBool() == other.asBool();
    case ValueType::STRING: return asString() == other.asString();
        // Для структур сравниваем указатели
    case ValueType::VECTOR:
        return std::get<shared_ptr<VectorHolder>>(data) ==
            std::get<shared_ptr<VectorHolder>>(other.data);
    case ValueType::LIST:
        return std::get<shared_ptr<ListHolder>>(data) ==
            std::get<shared_ptr<ListHolder>>(other.data);
    case ValueType::MAP:
        return std::get<shared_ptr<MapHolder>>(data) ==
            std::get<shared_ptr<MapHolder>>(other.data);
    }
    return false;
}

bool PersistentValue::operator!=(const PersistentValue& other) const {
    return !(*this == other);
}