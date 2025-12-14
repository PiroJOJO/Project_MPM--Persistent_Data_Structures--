#ifndef PERSISTENT_VECTOR_HPP
#define PERSISTENT_VECTOR_HPP

#include "persistent_data_structure.hpp"
#include <memory>
#include <vector>
#include <optional>
#include <cstdint>
#include <stdexcept>

// -----------------------------------------
// ---------------- Массив -----------------
// -----------------------------------------
// 
// Реализация массива через Bitmapped Vector Trie:
// - Каждый узел дерева хранит фиксированное число потомков;
// - Глубина дерева зависит от длины вектора;
// - Листья являются элементами вектора.

template<typename T>
class PersistentVector : public IPersistentStructure<T> {
private:
    // -----------------------------------------
    // ----------- Константы дерева ------------
    // -----------------------------------------
    static constexpr size_t BRANCHING_FACTOR = 32; // Количество потомков в узле
    static constexpr size_t BITS_PER_LEVEL = 5; // Уровень ветвления
    static constexpr size_t BIT_MASK = BRANCHING_FACTOR - 1; // Битовая маска

    // -----------------------------------------
    // ------------ Структура узла -------------
    // -----------------------------------------
    struct Node {
        std::shared_ptr<Node> children[BRANCHING_FACTOR]; // Потомки
        std::optional<T> values[BRANCHING_FACTOR];  // Значения
        size_t count = 0;

        Node() = default;
        // -----------------------------------------
        // ----------- Клонирование узла -----------
        // -----------------------------------------
        std::shared_ptr<Node> clone() const {
            auto new_node = std::make_shared<Node>();
            for (size_t i = 0; i < BRANCHING_FACTOR; ++i) {
                new_node->children[i] = children[i];
                new_node->values[i] = values[i];
            }
            new_node->count = count;
            return new_node;
        }
    };
    // -----------------------------------------
    // ----------- Структура дерева ------------
    // -----------------------------------------
    struct Data {
        std::shared_ptr<Node> root;
        size_t size; // Размер
        size_t shift; // Смещение

        Data() : root(std::make_shared<Node>()), size(0), shift(BITS_PER_LEVEL) {}
        Data(std::shared_ptr<Node> r, size_t s, size_t sh) : root(r), size(s), shift(sh) {}
    };

    std::shared_ptr<Data> data;

    // Клонирование метода с изменением данных
    std::shared_ptr<typename PersistentVector<T>::Node>
        assocNode(std::shared_ptr<typename PersistentVector<T>::Node> node,
            size_t shift, size_t index, const T& value) const;

    // Получение значения по индексу
    const T& getNodeValue(size_t index) const;
    // Добавление элемента в конец
    std::shared_ptr<Data> push(const T& value) const;

public:
    // -----------------------------------------
    // -------------- Конструкторы -------------
    // -----------------------------------------
    PersistentVector();
    PersistentVector(const std::vector<T>& values);

    // -----------------------------------------
    // ---------- IPersistentStructure ---------
    // -----------------------------------------
    size_t size() const override;
    bool empty() const override;
    std::shared_ptr<IPersistentStructure<T>> clear() const override;
    std::shared_ptr<IPersistentStructure<T>> clone() const override;

    // -----------------------------------------
    // ---------- Перекрытие операторов --------
    // -----------------------------------------
    const T& operator[](size_t index) const;
    const T& get(size_t index) const;

    // Установка по индексу нового элемента
    PersistentVector<T> set(size_t index, const T& value) const;
    // Добавление элемента в конец
    PersistentVector<T> append(const T& value) const;
    PersistentVector<T> push_back(const T& value) const {
        return append(value);
    }
    // Удаление элемента
    PersistentVector<T> pop_back() const;

    // -----------------------------------------
    // ----------- Итератор по дереву ----------
    // -----------------------------------------
    class Iterator {
    private:
        const PersistentVector* vec;  // Указатель на вектор
        size_t index; // Текущий индекс

    public:
        // -----------------------------------------
        // -------------- Конструктор --------------
        // -----------------------------------------
        Iterator(const PersistentVector* v, size_t i) : vec(v), index(i) {}
        // -----------------------------------------
        // ---------- Перекрытие операторов --------
        // -----------------------------------------
        // Разыменование указателя (значение)
        T operator*() const {
            return (*vec)[index];
        }
        // Следующий элемент
        Iterator& operator++() {
            ++index;
            return *this;
        }
        // Оператор неравенства
        bool operator!=(const Iterator& other) const {
            return vec != other.vec || index != other.index;
        }
    };
    // -----------------------------------------
    // ------------ Для работы цикла -----------
    // -----------------------------------------
    // Итератор на первый элемент
    Iterator begin() const {
        return Iterator(this, 0);
    }
    // Итератор за последний эелемент
    Iterator end() const {
        return Iterator(this, size());
    }
    // Преобразования
    std::vector<T> toStdVector() const;
};

#include "persistent_vector_impl.hpp"

#endif