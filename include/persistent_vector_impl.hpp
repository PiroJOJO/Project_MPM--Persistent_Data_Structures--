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
    data->shift = 0;
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

        // Проверяем, существует ли узел потомка
        if (!node->children[pos]) {
            throw std::runtime_error("Internal error: child node not found");
        }

        node = node->children[pos];
        shift -= BITS_PER_LEVEL;
    }
    // Извлечение значения из найденного узла
    size_t pos = index & BIT_MASK;

    if (!node->values[pos]) {
        throw std::runtime_error("Internal error: value not found in leaf");
    }

    return *node->values[pos];
}

// -----------------------------------------
// ------ Вставка элемента по индексу ------
// -----------------------------------------
// Алгоритм вставки по индексу элемента 
template<typename T>
std::shared_ptr<typename PersistentVector<T>::Node>
PersistentVector<T>::assocNode(std::shared_ptr<typename PersistentVector<T>::Node> node,
    size_t shift, size_t index, const T& value) const {
    // Если shift = 0 (все элементы в корне)
    if (shift == 0) {
        auto newNode = node->clone();
        size_t pos = index & BIT_MASK;
        // Обновляем счетчик
        if (!node->values[pos].has_value()) {
            // Добавляем новый элемент
            newNode->count = node->count + 1;
        }
        // Обновляем существующий
        newNode->values[pos] = value;

        return newNode;
    }
    // Общий случай: клонируем путь
    auto newNode = node->clone();
    // Устанавливаем значение в листе
    size_t pos = (index >> shift) & BIT_MASK;

    if (!node->children[pos]) {
        // Если не существует - создаем новый
        auto child = std::make_shared<Node>();
        newNode->children[pos] = assocNode(child, shift - BITS_PER_LEVEL, index, value);
    }
    else {
        // Иначе - клонируем в новый вектор
        newNode->children[pos] = assocNode(node->children[pos], shift - BITS_PER_LEVEL, index, value);
    }
    // Обновляем счетчик
    newNode->count = 0;
    for (size_t i = 0; i < BRANCHING_FACTOR; ++i) {
        if (newNode->children[i]) {
            newNode->count += newNode->children[i]->count;
        }
    }

    return newNode;
}

// Алгоритм вставки по индексу элемента (новый вектор)
template<typename T>
PersistentVector<T> PersistentVector<T>::set(size_t index, const T& value) const {
    if (index >= size()) {
        throw std::out_of_range("Index out of range");
    }

    auto newRoot = assocNode(data->root, data->shift, index, value);
    auto newData = std::make_shared<Data>(newRoot, data->size, data->shift);

    PersistentVector result;
    result.data = newData;
    return result;
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

    size_t depth = data->shift / BITS_PER_LEVEL + 1;
    size_t capacity = 1;
    for (size_t i = 0; i < depth; ++i) {
        capacity *= BRANCHING_FACTOR;
    }

    // Очевидный случай: Есть место в текущем корне (shift = 0)
    if (data->size < capacity) {
        // Есть место в текущем дереве
        auto newRoot = assocNode(data->root, data->shift, data->size, value);
        return std::make_shared<Data>(newRoot, data->size + 1, data->shift);
    }
    else {
        // Нужно увеличить глубину
        auto newRoot = std::make_shared<Node>();
        newRoot->children[0] = data->root;
        size_t newShift = data->shift + BITS_PER_LEVEL;

        auto updatedRoot = assocNode(newRoot, newShift, data->size, value);
        return std::make_shared<Data>(updatedRoot, data->size + 1, newShift);
    }
}
// -----------------------------------------
// ----- Удаление последнего элемента ------
// -----------------------------------------
// Реализация удаления последнего элемента
template<typename T>
PersistentVector<T> PersistentVector<T>::pop_back() const {
    // Очевидный случай
    if (empty()) {
        throw std::runtime_error("Cannot pop from empty vector");
    }

    if (data->size == 1) {
        return PersistentVector<T>();
    }
    // Уменьшаем счетчик
    auto newRoot = data->root->clone();
    auto newData = std::make_shared<Data>(newRoot, data->size - 1, data->shift);

    PersistentVector result;
    result.data = newData;
    return result;
}

// -----------------------------------------
// -- Преобразование в встроенный вектор ---
// -----------------------------------------
template<typename T>
std::vector<T> PersistentVector<T>::toStdVector() const {
    std::vector<T> result;
    result.reserve(size());
    // Простой обход 
    for (size_t i = 0; i < size(); ++i) {
        result.push_back(get(i));
    }

    return result;
}

#endif