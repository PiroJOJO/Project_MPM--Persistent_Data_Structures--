#ifndef PERSISTENT_MAP_IMPL_HPP
#define PERSISTENT_MAP_IMPL_HPP

#include "persistent_map.hpp"
#include <stdexcept>
#include <stack>
#include <cstdint>

// -----------------------------------------
// --- Реализация ассоциативного массива ---
// -----------------------------------------

// -----------------------------------------
// ----- Кастомная реализация смещения -----
// -----------------------------------------
namespace persistent_map_detail {
    inline uint32_t popcount(uint32_t x) {
        x = x - ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F;
        x = x + (x >> 8);
        x = x + (x >> 16);
        return x & 0x3F;
    }
}

// -----------------------------------------
// -------------- Конструкторы -------------
// -----------------------------------------
// Пустой массив
template<typename K, typename V>
PersistentMap<K, V>::PersistentMap()
    : root(std::make_shared<Node>()), map_size(0) {
}

// Конструктор из вектора пар (Ключ, Значение)
template<typename K, typename V>
PersistentMap<K, V>::PersistentMap(const std::vector<std::pair<K, V>>& items)
    : root(std::make_shared<Node>()), map_size(0) {
    PersistentMap<K, V> current;
    for (const auto& [key, value] : items) {
        current = current.set(key, value);
    }
    root = current.root;
    map_size = current.map_size;
}

// -----------------------------------------
// ------ Методы IPersistentStructure ------
// -----------------------------------------
// Размер
template<typename K, typename V>
size_t PersistentMap<K, V>::size() const {
    return map_size;
}

// Проверка на пустоту
template<typename K, typename V>
bool PersistentMap<K, V>::empty() const {
    return map_size == 0;
}

// Возвращение пустого массива
template<typename K, typename V>
std::shared_ptr<IPersistentStructure<std::pair<K, V>>>
PersistentMap<K, V>::clear() const {
    return std::make_shared<PersistentMap<K, V>>();
}

// Поверхностное копирование (копирование указателей)
template<typename K, typename V>
std::shared_ptr<IPersistentStructure<std::pair<K, V>>>
PersistentMap<K, V>::clone() const {
    auto result = std::make_shared<PersistentMap<K, V>>();
    result->root = root;
    result->map_size = map_size;
    return result;
}

// -----------------------------------------
// --- Вспомогательные методы для узлов ----
// -----------------------------------------
// Получение индекса по хешу
template<typename K, typename V>
size_t PersistentMap<K, V>::getIndex(uint32_t bitmap, size_t hash_fragment) const {
    // Проверка границ
    if (hash_fragment >= 32) {
        return 0;  // или можно бросить исключение
    }

    // Создаем маску для битов до hash_fragment
    uint32_t mask = (hash_fragment == 0) ? 0 : ((1u << hash_fragment) - 1);

    // Применяем маску к bitmap
    uint32_t masked = bitmap & mask;

    // Подсчитываем установленные биты
    return persistent_map_detail::popcount(masked);
}

// -----------------------------------------
// -------- Методы с ключами массива -------
// -----------------------------------------
// Проверка на наличие значения по ключу
template<typename K, typename V>
bool PersistentMap<K, V>::contains(const K& key) const {
    size_t hash = hasher(key);
    return findNode(root, hash, key, 0) != nullptr;
}

// Получение значения по ключу
template<typename K, typename V>
const V& PersistentMap<K, V>::at(const K& key) const {
    size_t hash = hasher(key);
    const V* value = findNode(root, hash, key, 0);
    if (!value) {
        throw std::out_of_range("Key not found");
    }
    return *value;
}

// Получение узла по ключу
template<typename K, typename V>
std::optional<V> PersistentMap<K, V>::get(const K& key) const {
    size_t hash = hasher(key);
    const V* value = findNode(root, hash, key, 0);
    if (value) {
        return *value;
    }
    return std::nullopt;
}

// -----------------------------------------
// --- Вспомогательные методы для узлов ----
// -----------------------------------------
// Поиск элемента по ключу и значению
template<typename K, typename V>
const V* PersistentMap<K, V>::findNode(std::shared_ptr<Node> node,
    size_t hash, const K& key,
    size_t level) const {
    if (!node) {
        return nullptr;
    }

    // Очевидные случаи
    // Если лист, то ищем в нем значения
    if (!node->entries.empty()) {
        return node->findValue(key);
    }

    // Если узел пустой
    if (node->children.empty()) {
        return nullptr;
    }

    // Вычисляем фрагмент хэша для текущего уровня
    size_t hash_fragment = (hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;

    // Если не сщуетсвует потомка с вычисленным фрагментом
    if (!(node->bitmap & (1 << hash_fragment))) {
        return nullptr;
    }

    // Получаем индекс потомка
    size_t index = getIndex(node->bitmap, hash_fragment);

    // Проверяем границы
    if (index >= node->children.size()) {
        return nullptr;
    }

    // Рекурсивно ищем в найденном потомке
    return findNode(node->children[index], hash, key, level + 1);
}

// Утсановка нового значения с возвращением новго массива
template<typename K, typename V>
PersistentMap<K, V> PersistentMap<K, V>::set(const K& key, const V& value) const {
    // Вычиление нового хэша и создание новго дерев с добавлением узла
    size_t hash = hasher(key);
    auto new_root = insertNode(root, hash, key, value, 0);

    PersistentMap<K, V> result;
    result.root = new_root;

    // Размер увеличаваем, если ключа не было
    const V* existing = findNode(root, hash, key, 0);
    result.map_size = existing ? map_size : map_size + 1;

    return result;
}

// -----------------------------------------
// -------- Добавление нового узла ---------
// -----------------------------------------
template<typename K, typename V>
std::shared_ptr<typename PersistentMap<K, V>::Node>
PersistentMap<K, V>::insertNode(std::shared_ptr<Node> node,
    size_t hash, const K& key,
    const V& value, size_t level) const {
    if (!node) {
        node = std::make_shared<Node>();
    }

    auto new_node = node->clone();

    // Узел является листом или не имеет записи
    if (!new_node->entries.empty() || new_node->children.empty()) {
        // Используем метод updateOrAdd
        bool was_updated = new_node->updateOrAdd(key, value);

        // Если ключ уже существовал, просто возвращаем обновленный узел
        if (was_updated) {
            return new_node;
        }

        // Проверяем, не нужно ли разделить узел
        if (new_node->entries.size() > BRANCHING_FACTOR / 2 &&
            level < (sizeof(size_t) * 8 / BITS_PER_LEVEL)) {

            // Разделяем узел
            auto split_node = std::make_shared<Node>();

            // Создаем копии записей перед их перемещением
            auto entries_copy = new_node->entries;

            for (const auto& entry : entries_copy) {
                const K& k = entry.first;
                const V& v = entry.second;
                size_t entry_hash = hasher(k);
                size_t fragment = (entry_hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;
                // Создание нового слота для хранения нового значения
                if (!(split_node->bitmap & (1 << fragment))) {
                    split_node->bitmap |= (1 << fragment);
                    size_t index = getIndex(split_node->bitmap, fragment);

                    auto leaf = std::make_shared<Node>();
                    leaf->entries.emplace_back(k, v);

                    split_node->children.insert(
                        split_node->children.begin() + index,
                        leaf
                    );
                }
                else {
                    // Рекурсивно вставляем новое значение в соответствующий узел потомка
                    size_t index = getIndex(split_node->bitmap, fragment);
                    split_node->children[index] = insertNode(
                        split_node->children[index],
                        entry_hash, k, v, level + 1
                    );
                }
            }
            return split_node;
        }
        return new_node;
    }

    // Если рассматриваем внутренний узел
    size_t hash_fragment = (hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;

    if (!(new_node->bitmap & (1 << hash_fragment))) {
        // Создаем новый лист
        new_node->bitmap |= (1 << hash_fragment);
        size_t index = getIndex(new_node->bitmap, hash_fragment);

        auto leaf = std::make_shared<Node>();
        leaf->entries.emplace_back(key, value);

        new_node->children.insert(
            new_node->children.begin() + index,
            leaf
        );
    }
    else {
        // Обновляем существующий узел потомка
        size_t index = getIndex(new_node->bitmap, hash_fragment);

        // Проверяем границы
        if (index >= new_node->children.size()) {
            return new_node;
        }
        // Рекурсивно обновляем существующий узел потомка
        new_node->children[index] = insertNode(
            new_node->children[index],
            hash, key, value, level + 1
        );
    }

    return new_node;
}

// -----------------------------------------
// ------ Удаление существующего узла ------
// -----------------------------------------
template<typename K, typename V>
PersistentMap<K, V> PersistentMap<K, V>::erase(const K& key) const {
    if (!contains(key)) {
        return *this;
    }

    // Простая реализация - создаем новую карту без удаленного ключа
    PersistentMap<K, V> result;

    for (auto it = begin(); it != end(); ++it) {
        const auto& pair = *it;  
        if (pair.first != key) {
            result = result.set(pair.first, pair.second);
        }
    }

    return result;
}

// -----------------------------------------
// ------------ Для работы цикла -----------
// -----------------------------------------

// -----------------------------------------
// ---------- Реализация итератора ---------
// -----------------------------------------
// Конструктор итератора
template<typename K, typename V>
PersistentMap<K, V>::Iterator::Iterator(std::shared_ptr<Node> root) {
    if (root && (!root->children.empty() || !root->entries.empty())) {
        stack.push_back({ root, 0, 0 });
        advance();
    }
    else {
        has_value = false;
    }
}

// Метод для обхода итератором
template<typename K, typename V>
void PersistentMap<K, V>::Iterator::advance() {
    while (!stack.empty()) {
        auto& frame = stack.back();

        // Обработка листа
        if (!frame.node->entries.empty()) {
            // Если существуют значения в листе, то итерируем
            if (frame.entry_index < frame.node->entries.size()) {
                current_value = frame.node->entries[frame.entry_index++];
                has_value = true;
                return;
            }
            stack.pop_back();
        }
        // Внутренний узел
        else {
            if (frame.child_index < frame.node->children.size()) {
                auto child = frame.node->children[frame.child_index++];
                if (child) {
                    stack.push_back({ child, 0, 0 });
                }
            }
            else {
                // Иначе удаляем лист из стека
                stack.pop_back();
            }
        }
    }
    has_value = false;
}

// -----------------------------------------
// ---------- Перекрытие операторов --------
// -----------------------------------------
// Следующий элемент
template<typename K, typename V>
typename PersistentMap<K, V>::Iterator&
PersistentMap<K, V>::Iterator::operator++() {
    advance();
    return *this;
}
// Оператор неравенства
template<typename K, typename V>
bool PersistentMap<K, V>::Iterator::operator!=(const Iterator& other) const {
    if (has_value != other.has_value) return true;
    if (!has_value) return false;  // оба end()
    return current_value != other.current_value;
}

// Итератор на первый элемент
template<typename K, typename V>
typename PersistentMap<K, V>::Iterator PersistentMap<K, V>::begin() const {
    return Iterator(root);
}

// Итератор за последний эелемент
template<typename K, typename V>
typename PersistentMap<K, V>::Iterator PersistentMap<K, V>::end() const {
    return Iterator(nullptr);
}

#endif 