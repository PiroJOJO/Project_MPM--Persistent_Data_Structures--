#ifndef PERSISTENT_VECTOR_IMPL_HPP
#define PERSISTENT_VECTOR_IMPL_HPP

#include "persistent_vector.hpp"
#include <stack>
// -----------------------------------------
// ---------- Реализация массива -----------
// -----------------------------------------

// -----------------------------------------
// -------------- Конструктор --------------
// -----------------------------------------
template<typename T>
PersistentVector<T>::PersistentVector() : data(std::make_shared<Data>()) {}

// -----------------------------------------
// ------ Конструктор из std::vector -------
// -----------------------------------------
template<typename T>
PersistentVector<T>::PersistentVector(const std::vector<T>& values) : data(std::make_shared<Data>()) {
    for (const auto& value : values) {
        data = push(value);
    }
}

// -----------------------------------------
// ------ Методы IPersistentStructure ------
// -----------------------------------------
// Размер
template<typename T>
size_t PersistentVector<T>::size() const {
    return data->size;
}

// Проверка на пустоту
template<typename T>
bool PersistentVector<T>::empty() const {
    return data->size == 0;
}

// Возвращение пустого вектора
template<typename T>
std::shared_ptr<IPersistentStructure<T>> PersistentVector<T>::clear() const {
    return std::make_shared<PersistentVector<T>>();
}

// Поверхностное копирование (копирование указателей)
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
    
    auto node = data->root;
    size_t shift = data->shift;
    
    while (shift > 0) {
        // Поиск потомка, содержащего элемент
        size_t pos = (index >> shift) & BIT_MASK;
        if (!node->children[pos]) {
            throw std::runtime_error("Internal error: node not found");
        }
        node = node->children[pos];
        shift -= BITS_PER_LEVEL;
    }
    
    // Извлечение значения из найденного узла
    size_t pos = index & BIT_MASK;
    if (!node->values[pos]) {
        throw std::runtime_error("Internal error: value not found");
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
    // Клонирование корня для нового вектора
    auto new_root = data->root->clone();
    auto node = new_root;
    auto new_data = std::make_shared<Data>(new_root, data->size, data->shift);
    
    size_t shift = data->shift;
    
    while (shift > 0) {
        // Находим ухел по индексу
        size_t pos = (index >> shift) & BIT_MASK;
        auto child = node->children[pos];
        if (!child) { 
            child = std::make_shared<Node>(); // Если не существует - создаем новый
        } else {
            child = child->clone(); // Иначе - клонируем в новый вектор
        }
        node->children[pos] = child;
        node = child;
        shift -= BITS_PER_LEVEL;
    }
    
    // Устанавливаем значение в листе
    size_t pos = index & BIT_MASK;
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
    // Очевидный случай: добавление в корневой узел
    if (data->size < BRANCHING_FACTOR) {
        auto new_root = data->root->clone();
        new_root->values[data->size] = value;
        return std::make_shared<Data>(new_root, data->size + 1, data->shift);
    }
    
    // !Нужно создавать новые узлы!
    throw std::runtime_error("Push not fully implemented for large vectors");
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
    if (data->size <= 1) {
        return std::make_shared<Data>();
    }
    
    auto new_root = data->root->clone();
    auto new_data = std::make_shared<Data>(new_root, data->size - 1, data->shift);
    
    // !Реализовать полную логику удаления!
    return new_data;
}
// -----------------------------------------
// -- Преобразование в встроенный вектор ---
// -----------------------------------------
template<typename T>
std::vector<T> PersistentVector<T>::toStdVector() const {
    std::vector<T> result;
    result.reserve(size());
    for (const auto& item : *this) {
        result.push_back(item);
    }
    return result;
}

#endif