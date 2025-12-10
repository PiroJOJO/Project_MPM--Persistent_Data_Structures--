#ifndef PERSISTENT_MAP_HPP
#define PERSISTENT_MAP_HPP

#include "persistent_data_structure.hpp"
#include <functional>
#include <optional>
#include <cstdint>
#include <memory>
#include <vector>

// -----------------------------------------
// --------- Ассоциативный массив ----------
// -----------------------------------------
// Hash Array Mapped Trie:
// Хэш-таблица для бытсрого доступа;
// Дерево для персистентности;
// Битовые маски для хранения.

template<typename K, typename V>
class PersistentMap : public IPersistentStructure<std::pair<K, V>> {
private:
    // -----------------------------------------
    // ---------- Константы массива ------------
    // -----------------------------------------
    static constexpr size_t BITS_PER_LEVEL = 5; // Количество битов на уровень
    static constexpr size_t BRANCHING_FACTOR = 1 << BITS_PER_LEVEL; // Количество потомков в узле
    static constexpr size_t BIT_MASK = BRANCHING_FACTOR - 1; // Маска 
    
    // -----------------------------------------
    // ------- Структура узла массива ----------
    // -----------------------------------------
    struct Node {
        uint32_t bitmap = 0; // Битовая маска для существующих потомков
        std::vector<std::shared_ptr<Node>> children; // Узлы потомков
        std::vector<std::pair<K, V>> entries; // Пары ключ-значение
        
        // Проверка на лист
        bool isLeaf() const { 
            return entries.size() > 0; 
        }
        
        // -----------------------------------------
        // ------ Клонирование узла массива --------
        // -----------------------------------------
        std::shared_ptr<Node> clone() const {
            auto new_node = std::make_shared<Node>();
            new_node->bitmap = bitmap;
            new_node->children = children;
            new_node->entries = entries;
            return new_node;
        }
    };
    
    std::shared_ptr<Node> root;
    size_t map_size;
    std::hash<K> hasher;
    
    // -----------------------------------------
    // --- Вспомогательные методы для узлов ----
    // -----------------------------------------
    // Получение индекса по хешу
    size_t getIndex(uint32_t bitmap, size_t hash_fragment) const;
    // Вставка элемента
    std::shared_ptr<Node> insertNode(std::shared_ptr<Node> node, 
                                     size_t hash, const K& key, 
                                     const V& value, size_t level) const;
    // Удаление элемента
    std::shared_ptr<Node> removeNode(std::shared_ptr<Node> node,
                                    size_t hash, const K& key,
                                    size_t level) const;
    // ПОиск элемента
    std::optional<V> findNode(std::shared_ptr<Node> node,
                             size_t hash, const K& key,
                             size_t level) const;
    
public:
    // -----------------------------------------
    // -------------- Конструкторы -------------
    // -----------------------------------------
    PersistentMap();
    PersistentMap(const std::vector<std::pair<K, V>>& items);
    
    // -----------------------------------------
    // ---------- IPersistentStructure ---------
    // -----------------------------------------
    size_t size() const override; 
    bool empty() const override;
    std::shared_ptr<IPersistentStructure<std::pair<K, V>>> clear() const override;
    std::shared_ptr<IPersistentStructure<std::pair<K, V>>> clone() const override;
    
    // -----------------------------------------
    // -------- Методы с ключами массива -------
    // -----------------------------------------
    bool contains(const K& key) const;
    const V& at(const K& key) const;
    std::optional<V> get(const K& key) const;
    
    PersistentMap<K, V> set(const K& key, const V& value) const; // Установка новго значения по ключу
    PersistentMap<K, V> insert(const K& key, const V& value) const { 
        return set(key, value); 
    }
    PersistentMap<K, V> erase(const K& key) const; // Удаление значения по ключу
    PersistentMap<K, V> remove(const K& key) const {
        return erase(key); 
    }
    
    // -----------------------------------------
    // ----------- Итератор по массиву ---------
    // -----------------------------------------
    class Iterator {
    private:
        struct StackFrame {
            std::shared_ptr<Node> node; // Текущий узел
            size_t child_index; // Индекс следующего потомка для итерации
            size_t entry_index; // Индекс следующей записи в листе
        };
        
        std::vector<StackFrame> stack;
        std::pair<K, V> current_value;
        bool has_value;
        
        void advance(); // Метод для обхода итератором
        
    public:
        Iterator(std::shared_ptr<Node> root);
        // -----------------------------------------
        // ---------- Перекрытие операторов --------
        // -----------------------------------------
        // Разыменование указателя (значение)
        const std::pair<K, V>& operator*() const {
            return current_value; 
        }
        // Следующий элемент
        Iterator& operator++();
        // Оператор неравенства
        bool operator!=(const Iterator& other) const;
    };
    // -----------------------------------------
    // ------------ Для работы цикла -----------
    // -----------------------------------------
    // Итератор на первый элемент
    Iterator begin() const;
    // Итератор за последний эелемент
    Iterator end() const;
};

#include "persistent_map_impl.hpp"

#endif