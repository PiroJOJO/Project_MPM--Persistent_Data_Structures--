#ifndef PERSISTENT_LIST_IMPL_HPP
#define PERSISTENT_LIST_IMPL_HPP

#include "persistent_list.hpp"
#include <stdexcept>
#include <vector>

// -----------------------------------------
// ---- Реализация двухсвязного списка -----
// -----------------------------------------

// -----------------------------------------
// -------------- Конструкторы -------------
// -----------------------------------------
// Создание пустого списка
template<typename T>
PersistentList<T>::PersistentList() : head(nullptr), list_size(0) {}

// Создание списка с одним значением
template<typename T>
PersistentList<T>::PersistentList(const T& value)
    : head(std::make_shared<Node>(value)), list_size(1) {
}

// Создание списка с узлом(голова) и размером
template<typename T>
PersistentList<T>::PersistentList(const std::shared_ptr<Node>& node, size_t size)
    : head(node), list_size(size) {
}

// Создание списка из вектора
template<typename T>
PersistentList<T>::PersistentList(const std::vector<T>& values)
    : head(nullptr), list_size(0) {
    // Строим список в обратном порядке
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        head = std::make_shared<Node>(*it, head);
        list_size++;
    }
}

// -----------------------------------------
// ------ Методы IPersistentStructure ------
// -----------------------------------------
// Размер
template<typename T>
size_t PersistentList<T>::size() const {
    return list_size;
}

// Проверка на пустоту
template<typename T>
bool PersistentList<T>::empty() const {
    return list_size == 0;
}

// Возвращение пустого списка
template<typename T>
std::shared_ptr<IPersistentStructure<T>> PersistentList<T>::clear() const {
    return std::make_shared<PersistentList<T>>();
}

// Поверхностное копирование (копирование указателей)
template<typename T>
std::shared_ptr<IPersistentStructure<T>> PersistentList<T>::clone() const {
    return std::make_shared<PersistentList<T>>(head, list_size);
}

// -----------------------------------------
// - Основные функции односвязного списка --
// -----------------------------------------
// Ссылка на первый элемент(голову) списка
template<typename T>
const T& PersistentList<T>::front() const {
    if (empty()) {
        throw std::runtime_error("List is empty");
    }
    return head->value;
}

// Возвращение хвоста списка
template<typename T>
PersistentList<T> PersistentList<T>::tail() const {
    if (empty()) {
        throw std::runtime_error("Cannot get tail of empty list");
    }
    return PersistentList<T>(head->next, list_size - 1);
}

// Добавление нового элемента в начало списка
template<typename T>
PersistentList<T> PersistentList<T>::prepend(const T& value) const {
    auto new_head = std::make_shared<Node>(value, head); // Переобозначим голову списка
    return PersistentList<T>(new_head, list_size + 1);
}

// Добавление элемента в конец списка
template<typename T>
PersistentList<T> PersistentList<T>::append(const T& value) const {
    if (empty()) {
        return PersistentList<T>(value);
    }

    // Рекурсивно создаем копию списка с новым элементом в конце
    auto new_head = std::make_shared<Node>(front());
    auto current = head->next;
    auto new_current = new_head;

    while (current) {
        new_current->next = std::make_shared<Node>(current->value);
        new_current = new_current->next;
        current = current->next;
    }

    // Добавляем новый элемент в конец
    new_current->next = std::make_shared<Node>(value);

    return PersistentList<T>(new_head, list_size + 1);
}

// Объединение двух списков
template<typename T>
PersistentList<T> PersistentList<T>::concat(const PersistentList<T>& other) const {
    if (empty()) 
        return other;
    if (other.empty()) 
        return *this;

    // Создаем копию текущего списка
    auto new_head = std::make_shared<Node>(front());
    auto current = head->next;
    auto new_current = new_head;

    while (current) {
        new_current->next = std::make_shared<Node>(current->value);
        new_current = new_current->next;
        current = current->next;
    }

    // Добавляем второй список
    auto other_current = other.head;
    while (other_current) {
        new_current->next = std::make_shared<Node>(other_current->value);
        new_current = new_current->next;
        other_current = other_current->next;
    }

    return PersistentList<T>(new_head, list_size + other.list_size);
}

// -----------------------------------------
// ------------ Односвязный API ------------
// -----------------------------------------
// Возвращение обратного списка
template<typename T>
PersistentList<T> PersistentList<T>::reverse() const {
    PersistentList<T> result;
    auto current = head;
    while (current) {
        result = result.prepend(current->value); // Добавляем в начало нового списка
        current = current->next;
    }
    return result;
}

// Взять первые n элементов
template<typename T>
PersistentList<T> PersistentList<T>::take(size_t n) const {
    // Очевидный случай
    if (n == 0 || empty()) {
        return PersistentList<T>();
    }

    auto new_head = std::make_shared<Node>(front());
    auto current = head->next;
    auto new_current = new_head;
    size_t count = 1;

    // Добавляем в новый список элемент до счетчика
    while (current && count < n) {
        new_current->next = std::make_shared<Node>(current->value);
        new_current = new_current->next;
        current = current->next;
        count++;
    }

    return PersistentList<T>(new_head, count);
}

// Отбросить первые n элементов
template<typename T>
PersistentList<T> PersistentList<T>::drop(size_t n) const {
    // Очевидный случай
    if (n >= list_size) {
        return PersistentList<T>();
    }

    // Отбрасываем нужное количество элементов от начала списка
    auto current = head;
    for (size_t i = 0; i < n; ++i) {
        if (current) { 
            current = current->next; 
        }
    }

    if (!current) { 
        return PersistentList<T>() 
    };

    // Копируем оставшуюся часть
    auto new_head = std::make_shared<Node>(current->value);
    auto new_current = new_head;
    current = current->next;
    size_t new_size = 1;

    while (current) {
        new_current->next = std::make_shared<Node>(current->value);
        new_current = new_current->next;
        current = current->next;
        new_size++;
    }

    return PersistentList<T>(new_head, new_size);
}

// -----------------------------------------
// ------ Двухсвязный API через Zipper -----
// -----------------------------------------
// Конструктор
template<typename T>
PersistentList<T>::ZipperView::ZipperView(const PersistentList<T>& list, size_t position) {
    if (list.empty()) {
        throw std::runtime_error("Cannot create zipper from empty list");
    }

    if (position >= list.size()) {
        throw std::out_of_range("Position out of range");
    }
    // Берем первые position элементов как left (в обратном порядке)
    left = list.take(position).reverse();
    // Текущий элемент
    right = list.drop(position);
    current = right.front();
    // Правую часть без текущего элемента
    right = right.tail();
}

// -----------------------------------------
// --------------- Навигация ---------------
// -----------------------------------------
// Смещение на следующий элемент
template<typename T>
typename PersistentList<T>::ZipperView
PersistentList<T>::ZipperView::next() const {
    if (right.empty()) {
        throw std::runtime_error("No next element");
    }
    // Добавляем current в left (в начало)
    PersistentList<T> new_left = left.prepend(current);
    // Новый current - первый элемент right
    T new_current = right.front();
    // Новая right - tail от старой right
    PersistentList<T> new_right = right.tail();

    return ZipperView(new_left.reverse(), new_current, new_right);
}

// Смещение на предыдущий элемент
template<typename T>
typename PersistentList<T>::ZipperView
PersistentList<T>::ZipperView::prev() const {
    if (left.empty()) {
        throw std::runtime_error("No previous element");
    }
    // Добавляем current в right (в начало)
    PersistentList<T> new_right = right.prepend(current);
    // Новый current - первый элемент left
    T new_current = left.front();
    // Новая left - tail от старой left
    PersistentList<T> new_left = left.tail();

    return ZipperView(new_left.reverse(), new_current, new_right);
}

// Смещение на заданную позицию
template<typename T>
typename PersistentList<T>::ZipperView
PersistentList<T>::ZipperView::moveTo(size_t position) const {
    // Преобразуем zipper обратно в список
    auto list = toList();
        
    return list.getZipper(position); // Создаем новый zipper с нужной позицией
}

// -----------------------------------------
// ---------- Функции через Zipper ---------
// -----------------------------------------
// Добавление элемент перед текущим
template<typename T>
PersistentList<T> PersistentList<T>::ZipperView::insertBefore(const T& value) const {
    // Левая часть + новый элемент + current + правая часть
    auto new_left = left.prepend(value);  // Новый элемент в начало left
    auto full_list = new_left.reverse().concat(right.prepend(current)); // Текущий элемент в начало right и объединение

    return full_list;
}

// Добавление элемента после текущего
template<typename T>
PersistentList<T> PersistentList<T>::ZipperView::insertAfter(const T& value) const {
    // Левая часть + current + новый элемент + правая часть
    auto new_right = right.prepend(value);  // Новый элемент перед right
    auto full_list = left.reverse().concat(new_right.prepend(current)); // Текущий и новый в начало right и объединение

    return full_list;
}

// Удаление текущего значения
template<typename T>
PersistentList<T> PersistentList<T>::ZipperView::removeCurrent() const {
    // Удаляем единственный элемент
    if (left.empty() && right.empty()) {
                return PersistentList<T>();
    }
    // Заменяем current на первый элемент right
    if (!right.empty()) {
        T new_current = right.front();
        PersistentList<T> new_right = right.tail();
        return left.reverse().concat(new_right.prepend(new_current));
    }
    else {
        // current был последним, возвращаем left в обратном порядке
        return left.reverse();
    }
}

// Обновить текущее значение
template<typename T>
PersistentList<T> PersistentList<T>::ZipperView::updateCurrent(const T& value) const {
    // Левая часть + новое значение + правая часть
    return left.reverse().concat(right.prepend(value));
}

// -----------------------------------------
// -------- Преобразование в список --------
// -----------------------------------------
template<typename T>
PersistentList<T> PersistentList<T>::ZipperView::toList() const {
    // left (в обратном порядке) + current + right
    return left.reverse().concat(right.prepend(current));
}

// -----------------------------------------
// Методы PersistentList для работы с Zipper
// -----------------------------------------
// Создание zipper
template<typename T>
typename PersistentList<T>::ZipperView
PersistentList<T>::getZipper(size_t position) const {
    return ZipperView(*this, position);
}

// -----------------------------------------
// ---- Работа с значениями по позициям ----
// -----------------------------------------
//Добавление значения по позиции
template<typename T>
PersistentList<T> PersistentList<T>::insertAt(size_t position, const T& value) const {
    // Очевидный случай
    if (position > list_size) {
        throw std::out_of_range("Position out of range");
    }
    if (position == 0) {
        return prepend(value);
    }
    if (position == list_size) {
        return append(value);
    }

    // Создаем zipper в нужной позиции и вставляем значение
    auto zipper = getZipper(position);
    return zipper.insertBefore(value);
}

// Удаление значения по позиции
template<typename T>
PersistentList<T> PersistentList<T>::removeAt(size_t position) const {
    // Очевидный случай
    if (position >= list_size) {
        throw std::out_of_range("Position out of range");
    }
    if (position == 0) {
        return tail();
    }

    // Создаем zipper в нужной позиции и удаляем значение
    auto zipper = getZipper(position);
    return zipper.removeCurrent();
}

// Полуыение значения по позиции
template<typename T>
const T& PersistentList<T>::at(size_t position) const {
    // Очевидный случай
        if (position >= list_size) {
        throw std::out_of_range("Position out of range");
    }

    auto zipper = getZipper(position);
    return zipper.getCurrent();
}

// -----------------------------------------
// ------ Получение элементов с конца ------
// -----------------------------------------
// Получение последнего элемента
template<typename T>
const T& PersistentList<T>::back() const {
    // Очевидный случай
    if (empty()) {
        throw std::runtime_error("List is empty");
    }

    // Создаем zipper в последней позиции
    auto zipper = getZipper(list_size - 1);
    return zipper.getCurrent();
}

template<typename T>
PersistentList<T> PersistentList<T>::init() const {
    // Очевидный случай
    if (empty()) {
        throw std::runtime_error("Cannot get init of empty list");
    }
    if (list_size == 1) {
        return PersistentList<T>();
    }

    // Удаляем последний элемент
    return removeAt(list_size - 1);
}

// -----------------------------------------
// ------------- Преобразования ------------
// -----------------------------------------
// Преобразования в вектор
template<typename T>
std::vector<T> PersistentList<T>::toVector() const {
    std::vector<T> result;
    result.reserve(list_size);

    auto current = head;
    while (current) {
        result.push_back(current->value);
        current = current->next;
    }

    return result;
}

// Преобразования в контейнер
template<typename T>
template<typename Container>
Container PersistentList<T>::toContainer() const {
    Container result;
    auto current = head;
    while (current) {
        result.insert(result.end(), current->value);
        current = current->next;
    }
    return result;
}

#endif 