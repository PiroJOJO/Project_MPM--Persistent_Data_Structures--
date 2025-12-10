#ifndef PERSISTENT_LIST_HPP
#define PERSISTENT_LIST_HPP

#include "persistent_data_structure.hpp"
#include <memory>
#include <optional>
#include <stack>

// -----------------------------------------
// ---------- Двухсвязный список  ----------
// -----------------------------------------
// Реализация через Zipper для эмуляции двунаправленности.
// Похоже на функциональный подход.

template<typename T>
class PersistentList : public IPersistentStructure<T> {
private:
    // -----------------------------------------
    // ----------- Структура Zipper ------------
    // -----------------------------------------
    struct Zipper {
        PersistentList<T> left;   // Пройденные элементы в обратном порядке
        T current;                // Текущий элемент
        PersistentList<T> right;  // Оставшиеся элементы

        Zipper(const PersistentList<T>& l, const T& c, const PersistentList<T>& r)
            : left(l), current(c), right(r) {
        }
    };

    // -----------------------------------------
    // ------------ Структура узла -------------
    // -----------------------------------------
    struct Node {
        T value; // Значение
        std::shared_ptr<Node> next; // Следующий элемент

        Node(const T& val, std::shared_ptr<Node> nxt = nullptr)
            : value(val), next(nxt) {
        }
    };

    std::shared_ptr<Node> head;
    size_t list_size;

    // Отразить список
    PersistentList<T> reverse() const;
    // Взять первые n элементов
    PersistentList<T> take(size_t n) const;
    // Отбросить первые n элементов
    PersistentList<T> drop(size_t n) const;

public:
    // -----------------------------------------
    // -------------- Конструкторы -------------
    // -----------------------------------------
    PersistentList();
    PersistentList(const T& value);
    PersistentList(const std::shared_ptr<Node>& node, size_t size);
    PersistentList(const std::vector<T>& values);

    // -----------------------------------------
    // ---------- IPersistentStructure ---------
    // -----------------------------------------
    size_t size() const override;
    bool empty() const override;
    std::shared_ptr<IPersistentStructure<T>> clear() const override;
    std::shared_ptr<IPersistentStructure<T>> clone() const override;

    // -----------------------------------------
    // ------------ Односвязный API ------------
    // -----------------------------------------
    const T& front() const;
    PersistentList<T> tail() const; // Хвост списка
    PersistentList<T> prepend(const T& value) const; // Добавление элемента в начало списка
    PersistentList<T> append(const T& value) const; // Добавление элемента в конец списка
    PersistentList<T> concat(const PersistentList<T>& other) const; // Объединение двух списков

    // -----------------------------------------
    // ------ Двухсвязный API через Zipper -----
    // -----------------------------------------
    class ZipperView {
    private:
        PersistentList<T> left; // Пройденные элементы в обратном порядке
        T current; // Текущее значение
        PersistentList<T> right; // Следующие элементы

    public:
        // -----------------------------------------
        // -------------- Конструктор --------------
        // -----------------------------------------
        ZipperView(const PersistentList<T>& list, size_t position = 0);

        // -----------------------------------------
        // --------------- Навигация ---------------
        // -----------------------------------------
        ZipperView next() const; // Следующий элемент
        ZipperView prev() const; // Предыдущий элемент
        ZipperView moveTo(size_t position) const; // Сместиться на заданную позицию

        // -----------------------------------------
        // ---------- Функции через Zipper ---------
        // -----------------------------------------
        PersistentList<T> insertBefore(const T& value) const; // Добавить элемент до текующей позиции
        PersistentList<T> insertAfter(const T& value) const; // Добавить элемент после текущей позиции
        PersistentList<T> removeCurrent() const; // Удалить элемент
        PersistentList<T> updateCurrent(const T& value) const; // Обновить текущее значение

        // -----------------------------------------
        // ----------- Получение значений ----------
        // -----------------------------------------
        const T& getCurrent() const { 
            return current; 
        }
        bool hasNext() const { 
            return !right.empty(); 
        }
        bool hasPrev() const { 
            return !left.empty(); 
        }

        // -----------------------------------------
        // -------- Преобразование в список --------
        // -----------------------------------------
        PersistentList<T> toList() const;
    };

    // Создание zipper
    ZipperView getZipper(size_t position = 0) const;

    // -----------------------------------------
    // ---- Работа с значениями по позициям ----
    // -----------------------------------------
    PersistentList<T> insertAt(size_t position, const T& value) const;
    PersistentList<T> removeAt(size_t position) const;
    const T& at(size_t position) const;

    // -----------------------------------------
    // ----------- Итератор по списку ----------
    // -----------------------------------------
    class Iterator {
    private:
        std::shared_ptr<Node> current; // Текущее значение

    public:
        Iterator(std::shared_ptr<Node> node) : current(node) {}
        // -----------------------------------------
        // ---------- Перекрытие операторов --------
        // -----------------------------------------
        // Разыменование указателя (значение)
        const T& operator*() const {
            return current->value; 
        }
        // Следующий элемент
        Iterator& operator++() {
            if (current) current = current->next;
            return *this;
        }
        // Оператор неравенства
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };
    // -----------------------------------------
    // ------------ Для работы цикла -----------
    // -----------------------------------------
    // Итератор на первый элемент
    Iterator begin() const {
        return Iterator(head);
    }
    // Итератор за последний эелемент
    Iterator end() const { 
        return Iterator(nullptr); 
    }

    // -----------------------------------------
    // ------------- Преобразования ------------
    // -----------------------------------------
    std::vector<T> toVector() const; // В вектор
    template<typename Container>
    Container toContainer() const; // В контейнер

    // -----------------------------------------
    // ------ Получение элементов с конца ------
    // -----------------------------------------
    const T& back() const; // Последний элемент
    PersistentList<T> init() const;  // Все кроме последнего
};

#include "persistent_list_impl.hpp"

#endif