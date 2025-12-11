# Проект: Персистентные структуры данных на C++

## Архитектура проекта

### 1. **`persistent_data_structure.hpp`**
**Назначение**: Базовый абстрактный интерфейс для всех персистентных структур

Определяет единый API для всех структур (вектор, список, мапа). Это позволяет:
- Единообразно работать с разными структурами
- Реализовать полиморфизм
- Упростить разработку фабрик и утилит

### 2. **`persistent_value.hpp/.cpp`**
**Назначение**: Универсальный контейнер для хранения любых типов данных

- Хранит значения разных типов через `std::variant`
- Поддерживает вложенность структур (вектор может содержать другие векторы, списки и т.д.)
- Обеспечивает проверку типов во время выполнения

**Реализует пункт 1 из дополнительных требований** - "произвольная вложенность данных". Без него была бы строгая типизация:

```cpp
PersistentVector<int> // только int
// С PersistentValue:
PersistentVector<PersistentValue> // может содержать что угодно
```

### 3. **`persistent_vector.hpp` + `persistent_vector_impl.hpp`**
**Назначение**: Реализация персистентного массива (вектора) с константным временем доступа

**Алгоритм**: Bitmapped Vector Trie (как в Clojure)
- Вместо копирования всего массива при изменении
- Создаются только измененные узлы дерева
- Неизмененные узлы разделяются между версиями
- Эффективная персистентная структура O(log₃₂ n) доступа вместо O(n) копирования

**Разделение на .hpp и _impl.hpp**:
- **`.hpp`** - объявление класса, публичный интерфейс
- **`_impl.hpp`** - реализация шаблонных методов (техническая необходимость C++)

### 4. **`persistent_list.hpp` + `persistent_list_impl.hpp`**
**Назначение**: Реализация персистентного двусвязного списка через zipper

- Добавление в начало - O(1)
- Разделение хвоста между версиями

**Пример использования**:
```cpp
auto list1 = PersistentList<int>();
auto list2 = list1.prepend(3).prepend(2).prepend(1);
// list2: 1 → 2 → 3
// list1: (пустой) - исходный список не изменился!
```

### 5. **`persistent_map.hpp` + `persistent_map_impl.hpp`**
**Назначение**: Реализация персистентного ассоциативного массива (словаря)

**Алгоритм**: Hash Array Mapped Trie (HAMT)
- Хеш-таблица + префиксное дерево (trie)
- Коллизии разрешаются через дерево
- Эффективные операции: O(log n) в худшем случае

Реализована персистентная версия `std::unordered_map` с сохранением предыдущих версий.

### 6. **`persistent_factory.hpp`**
**Назначение**: Фабрика для преобразования между структурами

**Содержит методы**:
- `listToVector()` - список → вектор
- `vectorToList()` - вектор → список  
- `mapToVector()` - словарь → вектор пар
- `vectorToMap()` - вектор пар → словарь

**Здесь реализован пункт 4 из дополнительных требований** - "экономичное преобразование структур". Фабрика старается максимально использовать разделение данных вместо полного копирования.

---

## Реализация пункта 3: "Более эффективное представление чем fat-node"

### **1. PersistentVector (persistent_vector.hpp/impl.hpp)**

**Fat-node (неэффективный подход)**:
```cpp
// Fat-node подход: каждый узел хранит ВСЕ версии
struct FatNode {
    std::vector<std::pair<Version, Value>> history;
    // При доступе нужно искать по версии - O(log m)
};
```

**Наш подход (эффективный)**:
```cpp
struct Node {
    // Одна версия значения, не история!
    std::optional<T> values[BRANCHING_FACTOR];
    // Разделение через неизменяемость
};
```

**В коде (persistent_vector_impl.hpp)**:
```cpp
template<typename T>
const T& PersistentVector<T>::getNodeValue(size_t index) const {
    // Прямой доступ через дерево - O(log32 n)
    auto node = data->root;
    size_t shift = data->shift;
    
    while (shift > 0) {
        size_t pos = (index >> shift) & BIT_MASK;  // Битовые операции
        node = node->children[pos];  // Прямой переход
        shift -= BITS_PER_LEVEL;
    }
    
    size_t pos = index & BIT_MASK;
    return *node->values[pos];  // Непосредственный доступ
}
```

### **2. PersistentMap (persistent_map.hpp/impl.hpp)**
**Hash Array Mapped Trie (HAMT) вместо fat-node**

```cpp
// Вставка без fat-node
template<typename K, typename V>
std::shared_ptr<typename PersistentMap<K, V>::Node> 
PersistentMap<K, V>::insertNode(std::shared_ptr<Node> node, 
                               size_t hash, const K& key, 
                               const V& value, size_t level) const {
    // Копируем ТОЛЬКО путь в дереве, не все узлы
    auto new_node = node->clone();  // Глубокое копирование только этого узла
    
    if (new_node->isLeaf()) {
        // Обработка коллизий в листе
        for (auto& [k, v] : new_node->entries) {
            if (k == key) {
                v = value;  // Обновление
                return new_node;
            }
        }
        new_node->entries.emplace_back(key, value);
        return new_node;
    }
    
    // Разделение через копирование пути
    size_t hash_fragment = (hash >> (level * BITS_PER_LEVEL)) & BIT_MASK;
    
    if (!(new_node->bitmap & (1 << hash_fragment))) {
        // Новый узел
        new_node->bitmap |= (1 << hash_fragment);
        size_t index = getIndex(new_node->bitmap, hash_fragment);
        auto leaf = std::make_shared<Node>();
        leaf->entries.emplace_back(key, value);
        new_node->children.insert(
            new_node->children.begin() + index,
            leaf
        );
    } else {
        // Рекурсивное обновление
        size_t index = getIndex(new_node->bitmap, hash_fragment);
        new_node->children[index] = insertNode(
            new_node->children[index],
            hash, key, value, level + 1
        );
    }
    
    return new_node;
}
```

### **3. Чем наш подход лучше fat-node?**

**Fat-Node**:
- Узел: `{версия1: значение1, версия2: значение2, ...}`
- Доступ: `O(log m)` где `m` = число версий
- Память: хранит все версии

**Наш подход (Path Copying / Structural Sharing)**:
- При изменении: создается новый путь от корня к листу
- Старые узлы: остаются неизменными и разделяются
- Доступ: `O(log n)` где `n` = размер структуры
- Память: только последняя версия + разделяемые части

### **4. Конкретная демонстрация эффективности**
В файле `persistent_vector_impl.hpp`:

```cpp
template<typename T>
PersistentVector<T> PersistentVector<T>::set(size_t index, const T& value) const {
    if (index >= size()) {
        throw std::out_of_range("Index out of range");
    }
    
    PersistentVector result;
    // assoc создает ТОЛЬКО новые узлы на пути
    result.data = assoc(index, value);  // O(log32 n) копирований
    return result;
}

template<typename T>
std::shared_ptr<typename PersistentVector<T>::Data> 
PersistentVector<T>::assoc(size_t index, const T& value) const {
    // Копируем корень
    auto new_root = data->root->clone();
    
    // Только путь к изменяемому элементу
    auto node = new_root;
    size_t shift = data->shift;
    
    while (shift > 0) {
        size_t pos = (index >> shift) & BIT_MASK;
        auto child = node->children[pos];
        if (child) {
            child = child->clone();  // Копируем ТОЛЬКО этот узел
        }
        node->children[pos] = child;
        node = child;
        shift -= BITS_PER_LEVEL;
    }
    
    // Изменяем значение в листе
    size_t pos = index & BIT_MASK;
    node->values[pos] = value;
    
    return std::make_shared<Data>(new_root, data->size, data->shift);
}
```

---

## Визуализация связей между файлами

```
                    ┌─────────────────────┐
                    │   persistent_value  │ ← Универсальное значение
                    │    (вложенность)    │
                    └──────────┬──────────┘
                               │
         ┌─────────────┬─────────────┬─────────────┐
         │             │             │             │
         ▼             ▼             ▼             ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  persistent  │ │  persistent  │ │  persistent  │ │  persistent  │
│   vector     │ │    list      │ │     map      │ │   factory    │
│  (массив)    │ │  (список)    │ │ (словарь)    │ │(преобразования)│
└──────┬───────┘ └──────┬───────┘ └──────┬───────┘ └──────────────┘
       │                │                │
       └────────┬───────┴───────┬────────┘
                │               │
                ▼               ▼
         ┌─────────────┐ ┌─────────────┐
         │ IPersistent │ │  Алгоритмы: │
         │ Structure   │ │ • VectorTrie │
         │ (интерфейс) │ │ • HAMT      │
         └─────────────┘ └─────────────┘
```

## Структура проекта
```
persistent_project/
├── include/
│   ├── persistent_data_structure.hpp
│   ├── persistent_value.hpp
│   ├── persistent_vector.hpp
│   ├── persistent_vector_impl.hpp
│   ├── persistent_list.hpp
│   ├── persistent_list_impl.hpp
│   ├── persistent_map.hpp
│   ├── persistent_map_impl.hpp
│   └── persistent_factory.hpp
├── src/
│   ├── persistent_value.cpp
│   └── main.cpp
└── CMakeLists.txt
```

## Запуск проекта

```cmd
# Откройте "Командную строку разработчика"
# Перейдите в папку проекта
cd C:\путь\к\проекту\src

# Скомпилируйте программу
cl /EHsc /std:c++17 main.cpp

# Запустите скомпилированную программу
.\main.exe
```
