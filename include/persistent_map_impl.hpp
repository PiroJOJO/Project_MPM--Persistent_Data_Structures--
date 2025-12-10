#ifndef PERSISTENT_MAP_IMPL_HPP
#define PERSISTENT_MAP_IMPL_HPP

#include "persistent_map.hpp"
#include <stdexcept>
#include <stack>

// -----------------------------------------
// --- Реализация ассоциативного массива ---
// -----------------------------------------

// -----------------------------------------
// -------------- Конструкторы -------------
// -----------------------------------------
// Пустой массив
template<typename K, typename V>
PersistentMap<K, V>::PersistentMap() 
    : root(std::make_shared<Node>()), map_size(0) {}

// Конструктор из вектора пар (Ключ, Значение)
template<typename K, typename V>
PersistentMap<K, V>::PersistentMap(const std::vector<std::pair<K, V>>& items) 
    : root(std::make_shared<Node>()), map_size(0) {
    for (const auto& [key, value] : items) {
        auto new_map = set(key, value);
        root = new_map.root;
        map_size = new_map.map_size;
    }
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
    uint32_t mask = (1 << hash_fragment) - 1;
    return __builtin_popcount(bitmap & mask);
}
// -----------------------------------------
// -------- Методы с ключами массива -------
// -----------------------------------------
// Проверка на наличие значения по ключу
template<typename K, typename V>
bool PersistentMap<K, V>::contains(const K& key) const {
    size_t hash = hasher(key);
    return findNode(root, hash, key, 0).has_value();
}

// Получение значения по ключу
template<typename K, typename V>
const V& PersistentMap<K, V>::at(const K& key) const {
    size_t hash = hasher(key);
    auto value = findNode(root, hash, key, 0);
    if (!value.has_value()) {
        throw std::out_of_range("Key not found");
    }
    return value.value();
}

// Получение узла по ключу
template<typename K, typename V>
std::optional<V> PersistentMap<K, V>::get(const K& key) const {
    size_t hash = hasher(key);
    return findNode(root, hash, key, 0);
}

// -----------------------------------------
// --- Вспомогательные методы для узлов ----
// -----------------------------------------
// Поиск элемента по ключу и значению
template<typename K, typename V>
std::optional<V> PersistentMap<K, V>::findNode(std::shared_ptr<Node> node,
                                             size_t hash, const K& key,
                                             size_t level) const {
    // Очевидные случаи
    // Если лист, то ищем в нем значения
    if (node->isLeaf()) {
        for (const auto& [k, v] : node->entries) {
            if (k == key) {
                return v;
            }
        }
        return std::nullopt;
    }
    // Если в узле нет потомков
    if (node->children.empty()) {
        return std::nullopt;
    }
    
    // Вычисление хэша для текующего уровня и получение индекса в массиве потомков
    size_t hash_fragment = (hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;
    size_t index = getIndex(node->bitmap, hash_fragment);
    // Если не сщуетсвует потомка с вычисленным фрагментом
    if (!(node->bitmap & (1 << hash_fragment))) {
        return std::nullopt;
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
    result.map_size = map_size + (contains(key) ? 0 : 1); // Размер увеличаваем, если ключа не было
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
    auto new_node = node->clone();
    
    // Узел является листом или имеет записи
    if (new_node->isLeaf() || new_node->entries.size() > 0) {
        bool found = false;
        for (auto& [k, v] : new_node->entries) {
            if (k == key) {
                v = value; // Обновление существующего ключа
                found = true;
                break;
            }
        }
        
        if (!found) {
            // Добавление новой пары (ключ, значение)
            new_node->entries.emplace_back(key, value);
            
            // Проверяем, нужно ли разделить узел
            if (new_node->entries.size() > BRANCHING_FACTOR / 2 && level < (sizeof(size_t) * 8 / BITS_PER_LEVEL)) {
                // Разделяем узел
                auto split_node = std::make_shared<Node>();
                for (const auto& [k, v] : new_node->entries) {
                    size_t entry_hash = hasher(k);
                    size_t fragment = (entry_hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;
                    
                    if (!(split_node->bitmap & (1 << fragment))) {
                        // Создание нового слота для хранения нового значения
                        split_node->bitmap |= (1 << fragment);
                        size_t index = getIndex(split_node->bitmap, fragment);
                        split_node->children.insert(
                            split_node->children.begin() + index,
                            std::make_shared<Node>()
                        );
                    }
                    // Рекурсивно вставляем новое значение в соответствующий узел потомка
                    size_t index = getIndex(split_node->bitmap, fragment);
                    split_node->children[index] = insertNode(
                        split_node->children[index],
                        entry_hash, k, v, level + 1
                    );
                }
                return split_node;
            }
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
    } else {
        // Рекурсивно обновляем существующий дочерний узел
        size_t index = getIndex(new_node->bitmap, hash_fragment);
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
std::shared_ptr<typename PersistentMap<K, V>::Node>
PersistentMap<K, V>::removeNode(std::shared_ptr<Node> node,
    size_t hash, const K& key,
    size_t level) const {

    if (!node) return nullptr;

    auto new_node = std::make_shared<Node>();
    *new_node = *node;  // Копируем все поля

    // Если узел лист, то удаляем найденное по ключу значение
    if (new_node->entries.size() > 0) {
        for (size_t i = 0; i < new_node->entries.size(); i++) {
            if (new_node->entries[i].first == key) {
                new_node->entries.erase(new_node->entries.begin() + i);
                break;
            }
        }

        if (new_node->entries.empty()) {
            return nullptr;
        }

        return new_node;
    }
    // Если это внутренний узел
    size_t fragment = (hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;

    // Если нет такого потомка - ничего не делаем
    if (!(new_node->bitmap & (1 << fragment))) {
        return new_node;
    }
    size_t child_index = getIndex(new_node->bitmap, fragment);

    // Рекурсивно удаляем из потомка
    auto old_child = new_node->children[child_index];
    auto new_child = removeNode(old_child, hash, key, level + 1);

    // Если потомок стал пустым
    if (!new_child) {
        // Удаляем потомка
        new_node->bitmap &= ~(1 << fragment);
        new_node->children.erase(new_node->children.begin() + child_index);

        // Если узел опустел
        if (new_node->children.empty()) {
            return nullptr;
        }
    }
    else {
        // Обновляем потомка
        new_node->children[child_index] = new_child;
    }
    return new_node;
}

// Удаление значения по ключу
template<typename K, typename V>
PersistentMap<K, V> PersistentMap<K, V>::erase(const K& key) const {
    if (!contains(key)) {
        return *this;
    }
    
    size_t hash = hasher(key);
    auto new_root = removeNode(root, hash, key, 0);
    
    PersistentMap<K, V> result;
    result.root = new_root;
    result.map_size = map_size - 1;
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
    // Если у корня есть потомки - запускаем кастомный обход
    if (root && !root->children.empty()) {
        stack.push_back({root, 0, 0});
        advance();
    } else if (root && !root->entries.empty()) { // Корень является листом
        current_value = root->entries[0];
        has_value = true;
    } else { // Корень не существует
        has_value = false;
    }
}
// Метод для обхода итератором
template<typename K, typename V>
void PersistentMap<K, V>::Iterator::advance() {
    while (!stack.empty()) {
        auto& frame = stack.back();

        // Обработка листа
        if (frame.node->isLeaf()) {
            // Если существуют значения в листе, то итерируем
            if (frame.entry_index < frame.node->entries.size()) {
                current_value = frame.node->entries[frame.entry_index++];
                has_value = true;
                return;
            }
            // Иначе удаляем лист из стека
            stack.pop_back();
        } else {
            // Обработка внутреннего узла
            // Если есть не обработанный потомок, то итерируем
            if (frame.child_index < frame.node->children.size()) {
                auto child = frame.node->children[frame.child_index++];
                stack.push_back({child, 0, 0});
            } else {
                // Иначе удаляем узел из стека
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
    return has_value != other.has_value || 
           (has_value && current_value != other.current_value);
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