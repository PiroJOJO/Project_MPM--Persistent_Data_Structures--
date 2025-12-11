#ifndef PERSISTENT_VECTOR_IMPL_HPP
#define PERSISTENT_VECTOR_IMPL_HPP

#include "persistent_vector.hpp"
#include <stack>
#include <iostream>  

// -----------------------------------------
// ---------- Реализация массива -----------
// -----------------------------------------

// -----------------------------------------
// -------------- Конструктор --------------
// -----------------------------------------
template<typename T>
PersistentVector<T>::PersistentVector() : data(std::make_shared<Data>()) {
    // Для пустого вектора shift должен быть 0
    data->shift = 0;
}

// -----------------------------------------
// ------ Конструктор из std::vector -------
// -----------------------------------------
template<typename T>
PersistentVector<T>::PersistentVector(const std::vector<T>& values) : data(std::make_shared<Data>()) {
    data->shift = 0;  // Начинаем с shift = 0

    for (const auto& value : values) {
        data = push(value);
    }
}

// -----------------------------------------
// ------ Методы IPersistentStructure ------
// -----------------------------------------
template<typename T>
size_t PersistentVector<T>::size() const {
    return data->size;
}

template<typename T>
bool PersistentVector<T>::empty() const {
    return data->size == 0;
}

template<typename T>
std::shared_ptr<IPersistentStructure<T>> PersistentVector<T>::clear() const {
    return std::make_shared<PersistentVector<T>>();
}

template<typename T>
std::shared_ptr<IPersistentStructure<T>> PersistentVector<T>::clone() const {
    auto result = std::make_shared<PersistentVector<T>>();
    result->data = data;
    return result;
}

// -----------------------------------------
// ----- Получение элемента по индексу -----
// -----------------------------------------
template<typename T>
const T& PersistentVector<T>::operator[](size_t index) const {
    return getNodeValue(index);
}

template<typename T>
const T& PersistentVector<T>::get(size_t index) const {
    if (index >= size()) {
        throw std::out_of_range("Index out of range");
    }
    return getNodeValue(index);
}

// Реализация получения элемента по индексу
template<typename T>
const T& PersistentVector<T>::getNodeValue(size_t index) const {
    // Проверка на корректность индекса
    if (index >= data->size) {
        throw std::out_of_range("Index out of range");
    }

    // Если вектор пустой 
    if (empty()) {
        throw std::runtime_error("Vector is empty");
    }

    auto node = data->root;
    size_t shift = data->shift;

    // Если shift = 0, значит все элементы в корневом узле
    if (shift == 0) {
        // Все элементы в корневом узле
        if (index >= BRANCHING_FACTOR) {
            throw std::out_of_range("Index out of range for leaf node");
        }
        if (!node->values[index]) {
            throw std::runtime_error("Value not found in leaf node");
        }
        return *node->values[index];
    }

    // Иначе спускаемся по дереву
    while (shift > 0) {
        // Поиск потомка, содержащего элемент
        size_t pos = (index >> shift) & BIT_MASK;

        // Проверяем, существует ли дочерний узел
        if (!node->children[pos]) {
            std::cerr << "DEBUG: getNodeValue - child not found at pos=" << pos
                << ", shift=" << shift << ", index=" << index << std::endl;
            throw std::runtime_error("Internal error: child node not found");
        }

        node = node->children[pos];
        shift -= BITS_PER_LEVEL;
    }

    // Извлечение значения из найденного узла
    size_t pos = index & BIT_MASK;

    if (!node->values[pos]) {
        std::cerr << "DEBUG: getNodeValue - value not found at leaf pos=" << pos
            << ", index=" << index << std::endl;
        throw std::runtime_error("Internal error: value not found in leaf");
    }

    return *node->values[pos];
}

// -----------------------------------------
// ------ Вставка элемента по индексу ------
// -----------------------------------------
template<typename T>
PersistentVector<T> PersistentVector<T>::set(size_t index, const T& value) const {
    if (index >= size()) {
        throw std::out_of_range("Index out of range");
    }

    PersistentVector result;
    result.data = assoc(index, value);
    return result;
}

// Алгоритм вставки по индексу элемента (новый вектор)
template<typename T>
std::shared_ptr<typename PersistentVector<T>::Data>
PersistentVector<T>::assoc(size_t index, const T& value) const {
    // Если вектор пустой
    if (empty()) {
        throw std::runtime_error("Cannot set in empty vector");
    }

    // Если shift = 0 (все элементы в корне)
    if (data->shift == 0) {
        auto new_root = data->root->clone();

        if (index >= BRANCHING_FACTOR) {
            throw std::out_of_range("Index too large for leaf node");
        }
        new_root->values[index] = value;
        // Обновляем счетчик
        if (!data->root->values[index]) {
            // Добавляем новый элемент
            new_root->count = data->root->count + 1;
        }
        else {
            // Обновляем существующий
            new_root->count = data->root->count;
        }

        return std::make_shared<Data>(new_root, data->size, 0);
    }

    // Общий случай: клонируем путь
    auto new_root = data->root->clone();
    auto node = new_root;
    auto new_data = std::make_shared<Data>(new_root, data->size, data->shift);

    size_t shift = data->shift;
    while (shift > 0) {
        size_t pos = (index >> shift) & BIT_MASK;
        auto child = node->children[pos];

        if (!child) {
            // Если не существует - создаем новый
            child = std::make_shared<Node>();
            node->children[pos] = child;
        }
        else {
            // Иначе - клонируем в новый вектор
            child = child->clone();
            node->children[pos] = child;
        }

        node = child;
        shift -= BITS_PER_LEVEL;
    }

    // Устанавливаем значение в листе
    size_t pos = index & BIT_MASK;
    // Обновляем счетчик
    if (!node->values[pos]) {
        node->count++;
    }
    node->values[pos] = value;

    return new_data;
}

// -----------------------------------------
// ------ Добавление элемента в конец ------
// -----------------------------------------
template<typename T>
PersistentVector<T> PersistentVector<T>::append(const T& value) const {
    PersistentVector result;
    result.data = push(value);
    return result;
}

// Реализация добавления элемента в конец
template<typename T>
std::shared_ptr<typename PersistentVector<T>::Data>
PersistentVector<T>::push(const T& value) const {
    // Очевидный случай
    if (empty()) {
        auto root = std::make_shared<Node>();
        root->values[0] = value;
        root->count = 1;
        return std::make_shared<Data>(root, 1, 0);
    }

    // Очевидный случай: Есть место в текущем корне (shift = 0)
    if (data->shift == 0 && data->size < BRANCHING_FACTOR) {
        auto new_root = data->root->clone();
        new_root->values[data->size] = value;
        new_root->count = data->size + 1;
        return std::make_shared<Data>(new_root, data->size + 1, 0);
    }
    // Очевидный случай: Нужно увеличивать глубину
    // Упрощенная реализация - бросаем исключение
    throw std::runtime_error("push: vector too large, need to implement tree expansion");
}

// -----------------------------------------
// ----- Удаление последнего элемента ------
// -----------------------------------------
template<typename T>
PersistentVector<T> PersistentVector<T>::pop_back() const {
    if (empty()) {
        throw std::runtime_error("Cannot pop from empty vector");
    }

    PersistentVector result;
    result.data = pop();
    return result;
}
// Реализация удаления последнего элемента
template<typename T>
std::shared_ptr<typename PersistentVector<T>::Data>
PersistentVector<T>::pop() const {
    // Очевидный случай
    if (data->size == 0) {
        return std::make_shared<Data>();
    }
    if (data->size == 1) {
        return std::make_shared<Data>();
    }

    // Уменьшаем счетчик
    auto new_root = data->root->clone();
    return std::make_shared<Data>(new_root, data->size - 1, data->shift);
}

// -----------------------------------------
// -- Преобразование в встроенный вектор ---
// -----------------------------------------
template<typename T>
std::vector<T> PersistentVector<T>::toStdVector() const {
    std::vector<T> result;
    result.reserve(size());

    // Простой обход для небольших векторов
    for (size_t i = 0; i < size(); ++i) {
        try {
            result.push_back(get(i));
        }
        catch (const std::exception& e) {
            std::cerr << "Error getting element at index " << i << ": " << e.what() << std::endl;
            throw;
        }
    }

    return result;
}

#endif 