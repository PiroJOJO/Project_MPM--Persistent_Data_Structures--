#ifndef PERSISTENT_VALUE_HPP
#define PERSISTENT_VALUE_HPP

#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

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
    // структуры для хранения типизированных указателей
    struct VectorHolder {
        std::shared_ptr<void> vectorPtr;
        std::type_index elementType;

        template<typename T>
        VectorHolder(std::shared_ptr<PersistentVector<T>> ptr)
            : vectorPtr(ptr), elementType(typeid(T)) {
        }

        template<typename T>
        std::shared_ptr<PersistentVector<T>> get() const {
            if (elementType == typeid(T)) {
                return std::static_pointer_cast<PersistentVector<T>>(vectorPtr);
            }
            return nullptr;
        }

        std::type_index getElementType() const { return elementType; }
    };

    struct ListHolder {
        std::shared_ptr<void> listPtr;
        std::type_index elementType;

        template<typename T>
        ListHolder(std::shared_ptr<PersistentList<T>> ptr)
            : listPtr(ptr), elementType(typeid(T)) {
        }

        template<typename T>
        std::shared_ptr<PersistentList<T>> get() const {
            if (elementType == typeid(T)) {
                return std::static_pointer_cast<PersistentList<T>>(listPtr);
            }
            return nullptr;
        }

        std::type_index getElementType() const { return elementType; }
    };

    struct MapHolder {
        std::shared_ptr<void> mapPtr;
        std::type_index keyType;
        std::type_index valueType;

        template<typename K, typename V>
        MapHolder(std::shared_ptr<PersistentMap<K, V>> ptr)
            : mapPtr(ptr), keyType(typeid(K)), valueType(typeid(V)) {
        }

        template<typename K, typename V>
        std::shared_ptr<PersistentMap<K, V>> get() const {
            if (keyType == typeid(K) && valueType == typeid(V)) {
                return std::static_pointer_cast<PersistentMap<K, V>>(mapPtr);
            }
            return nullptr;
        }

        std::type_index getKeyType() const { return keyType; }
        std::type_index getValueType() const { return valueType; }
    };

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

    template<typename T>
    PersistentValue(std::shared_ptr<PersistentVector<T>> vector);

    template<typename T>
    PersistentValue(std::shared_ptr<PersistentList<T>> list);

    template<typename K, typename V>
    PersistentValue(std::shared_ptr<PersistentMap<K, V>> map);

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
    // ------- Получение значений данных -------
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

    // для вложенных структур
    std::type_index getElementType() const;
    std::type_index getKeyType() const;
    std::type_index getValueType() const;

    // глубокая копия
    PersistentValue clone() const;

    // для отладки
    std::string toString() const;

    // -----------------------------------------
    // ---- Перегрузка операторов сравнения ----
    // -----------------------------------------
    bool operator==(const PersistentValue& other) const;
    bool operator!=(const PersistentValue& other) const;
};

#endif