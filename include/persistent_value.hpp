#ifndef PERSISTENT_VALUE_HPP
#define PERSISTENT_VALUE_HPP

#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>

// -----------------------------------------
// ----------- Хранимые значения -----------
// -----------------------------------------
// 
// Вспомогательный класс, который реализует 
// хранение и утилиты для работы с значениями 
// и их типами


template<typename T>
class PersistentVector;

template<typename T>
class PersistentList;

template<typename K, typename V>
class PersistentMap;

enum class ValueType {
    NULL_VALUE,
    INT,
    DOUBLE,
    BOOL,
    STRING,
    VECTOR,
    LIST,
    MAP
};

class PersistentValue {
private:
    struct VectorHolder;
    struct ListHolder;
    struct MapHolder;

    std::variant<
        std::monostate,
        int,
        double,
        bool,
        std::string,
        std::shared_ptr<VectorHolder>,
        std::shared_ptr<ListHolder>,
        std::shared_ptr<MapHolder>
    > data;

public:
    // -----------------------------------------
    // -------------- Конструкторы -------------
    // -----------------------------------------
    PersistentValue(); 
    PersistentValue(int value);
    PersistentValue(double value);
    PersistentValue(bool value);
    PersistentValue(const std::string& value);
    PersistentValue(const char* value);
    
    // -----------------------------------------
    // -- Конструкторы для кастомных структур --
    // -----------------------------------------
    template<typename T>
    PersistentValue(const PersistentVector<T>& vector);
    
    template<typename T>
    PersistentValue(const PersistentList<T>& list);
    
    template<typename K, typename V>
    PersistentValue(const PersistentMap<K, V>& map);

    // -----------------------------------------
    // -------------- Тип данных ---------------
    // -----------------------------------------
    ValueType type() const;

    // -----------------------------------------
    // --------- Проверка типов данных ---------
    // -----------------------------------------
    bool isNull() const;
    bool isInt() const;
    bool isDouble() const;
    bool isBool() const;
    bool isString() const;
    bool isVector() const;
    bool isList() const;
    bool isMap() const;

    // -----------------------------------------
    // --------- Проверка типов данных ---------
    // -----------------------------------------
    int asInt() const;
    double asDouble() const;
    bool asBool() const;
    std::string asString() const;
    
    template<typename T>
    std::shared_ptr<PersistentVector<T>> asVector() const;
    
    template<typename T>
    std::shared_ptr<PersistentList<T>> asList() const;
    
    template<typename K, typename V>
    std::shared_ptr<PersistentMap<K, V>> asMap() const;

    // -----------------------------------------
    // ---- Перегрузка операторов сравнения ----
    // -----------------------------------------
    bool operator==(const PersistentValue& other) const;
    bool operator!=(const PersistentValue& other) const;
};

#endif
