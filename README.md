# Персистентные структуры данных на C++
**Персистентность** — это свойство структур данных сохранять все свои предыдущие состояния при изменениях, позволяя к ним обращаться и использовать, что достигается путем создания новых узлов вместо изменения старых и использования ссылок для построения версий (например, в деревьях), а не просто в памяти, но и как постоянное хранение данных на диске или в базе данных, чтобы они переживали завершение программы, реализуя «вечные» объекты. 

---
## Архитектура проекта

### 1. Базовый абстрактный интерфейс для всех персистентных структур - **`persistent_data_structure.hpp`**

Определяет единый API для всех структур.

**Базовый интерфейс IPersistentStructure**:
```cpp
// Все структуры реализуют эти методы:
virtual size_t size() const = 0;              // Размер структуры
virtual bool empty() const = 0;               // Проверка на пустоту
virtual std::shared_ptr<IPersistentStructure<T>> clear() const = 0;  // Очистка
virtual std::shared_ptr<IPersistentStructure<T>> clone() const = 0;  // Копирование
```


### 2. Универсальный контейнер для хранения любых типов данных - **`persistent_value.hpp/.cpp`**

- Хранит значения разных типов данных
- Поддерживает вложенность структур
- Обеспечивает проверку типов во время выполнения

### **❗️ Реализует пункт 1 из дополнительных требований** - "произвольная вложенность данных" ❗️

### 3. Реализация персистентного массива (вектора) с константным временем доступа - **`persistent_vector.hpp` + `persistent_vector_impl.hpp`**

**Алгоритм**: Bitmapped Vector Trie (как в Clojure)
- Вместо копирования всего массива при изменении создаются только измененные узлы дерева
- Неизмененные узлы разделяются между версиями

**Доступные методы**:
```cpp
// Конструкторы
PersistentVector()                              // Пустой вектор
PersistentVector(const std::vector<T>& values) // Из std::vector

// Копирование и очистка
std::shared_ptr<IPersistentStructure<T>> clone() const     // Поверхностная копия
std::shared_ptr<IPersistentStructure<T>> clear() const     // Новый пустой вектор

// Базовые операции
size_t size() const                            // Текущий размер
bool empty() const                             // Проверка на пустоту
const T& operator[](size_t index) const        // Доступ по индексу
const T& get(size_t index) const               // Безопасный доступ

// Модификации (возвращают новую версию)
PersistentVector set(size_t index, const T& value) const  // Установка значения
PersistentVector append(const T& value) const            // Добавление в конец
PersistentVector push_back(const T& value) const         // Синоним для append()
PersistentVector pop_back() const                        // Удаление последнего

// Итераторы
Iterator begin() const                            // Итератор на первый элемент
Iterator end() const                              // Итератор за последним элементом

// Класс итератора
Iterator(const PersistentVector* v, size_t i)     // Конструктор итератора
T operator*() const                               // Разыменование итератора
Iterator& operator++()                            // Префиксный инкремент
bool operator!=(const Iterator& other) const      // Проверка неравенства

// Преобразования
std::vector<T> toStdVector() const           // В std::vector

// Вспомогательные методы для работы с деревом
std::shared_ptr<Node> assocNode(...) const   // Рекурсивное клонирование узла
const T& getNodeValue(size_t index) const    // Рекурсивный поиск в дереве

// Внутренние операции модификации
std::shared_ptr<Data> push(const T& value) const  // Внутренняя реализация append
std::shared_ptr<Data> pop() const                 // Внутренняя реализация pop_back
```
### ❗️ **Как реализована персистентность:** Через дерево с копированием пути. ❗️ 
1. **Структура данных:** Вектор представлен как сбалансированное дерево 
2. **При изменении элемента:**
   - От корня до листа с нужным элементом создается **новая цепочка узлов**
   - Каждый узел в этой цепочке клонируется
   - Все узлы вне этой цепочки **не копируются**, а переиспользуются
3. **Разделение памяти:** Неизмененные части дерева физически являются одними и теми же объектами в памяти для всех версий

### 4. Реализация персистентного двусвязного списка через zipper - **`persistent_list.hpp` + `persistent_list_impl.hpp`**

**Доступные методы**:
```cpp
// Конструкторы
PersistentList()                              // Пустой список
PersistentList(const T& value)               // С одним элементом
PersistentList(const std::vector<T>& values) // Из std::vector

// Базовые операции
size_t size() const                          // Размер списка
bool empty() const                           // Проверка на пустоту
const T& front() const                       // Первый элемент
const T& back() const                        // Последний элемент
const T& at(size_t position) const           // Элемент по позиции

// Модификации (возвращают новую версию)
PersistentList prepend(const T& value) const   // Добавление в начало
PersistentList append(const T& value) const    // Добавление в конец
PersistentList concat(const PersistentList& other) const  // Объединение
PersistentList insertAt(size_t position, const T& value) const  // Вставка
PersistentList removeAt(size_t position) const                 // Удаление
PersistentList tail() const                   // Список без первого элемента
PersistentList init() const                   // Список без последнего элемента
PersistentList reverse() const                // Обратный список
PersistentList take(size_t n) const           // Первые n элементов
PersistentList drop(size_t n) const           // Без первых n элементов

// Zipper API для навигации
class ZipperView {
    ZipperView next() const                    // Следующий элемент
    ZipperView prev() const                    // Предыдущий элемент
    ZipperView moveTo(size_t position) const   // Перемещение к позиции
    PersistentList insertBefore(const T& value) const  // Вставка перед
    PersistentList insertAfter(const T& value) const   // Вставка после
    PersistentList removeCurrent() const       // Удаление текущего
    PersistentList updateCurrent(const T& value) const // Обновление
    PersistentList toList() const              // Преобразование обратно в список
    const T& getCurrent() const                // Текущий элемент
}

ZipperView getZipper(size_t position) const    // Создание zipper'а

// Преобразования
std::vector<T> toVector() const               // В std::vector
template<typename Container> Container toContainer() const  // В произвольный контейнер
```
### ❗️ **Как реализована персистентность:** Двумя способами в зависимости от операции. ❗️ 

#### **А) Для добавления в начало (`prepend`):**
1. **Создается новый узел**, который указывает на старую голову списка
2. **Старый список полностью переиспользуется** как хвост нового
3. **Ничего не копируется**, кроме одного нового узла

#### **Б) Для других операций (добавление в конец, вставка в середину):**
Используется **Zipper-подход**:
1. **Zipper (бегунок)** делит список на три части:
   - Левая часть (до текущего элемента, в обратном порядке)
   - Текущий элемент
   - Правая часть (после текущего элемента)
2. **При изменении:** Zipper создает новый список, собирая его из:
   - Неизмененных частей (которые переиспользуются)
   - Новых элементов (которые создаются)
3. **Навигация:** Zipper может двигаться по списку, создавая новые представления

### 5. Реализация персистентного ассоциативного массива (словаря) - **`persistent_map.hpp` + `persistent_map_impl.hpp`**

**Алгоритм**: Hash Array Mapped Trie (HAMT)

**Доступные методы**:
```cpp
// Конструкторы
PersistentMap()                                                   // Пустая мапа
PersistentMap(const std::vector<std::pair<K, V>>& items)         // Из вектора пар

// Базовые операции
size_t size() const                                              // Количество пар
bool empty() const                                               // Проверка на пустоту
bool contains(const K& key) const                                // Проверка наличия ключа
const V& at(const K& key) const                                  // Доступ по ключу (бросает исключение)
std::optional<V> get(const K& key) const                         // Безопасный доступ

// Модификации (возвращают новую версию)
PersistentMap set(const K& key, const V& value) const            // Установка/обновление значения
PersistentMap insert(const K& key, const V& value) const         // Синоним для set()
PersistentMap erase(const K& key) const                          // Удаление по ключу
PersistentMap remove(const K& key) const                         // Синоним для erase()

// Конструктор итератора
Iterator(std::shared_ptr<Node> root)                            // Создает итератор для обхода дерева

// Итераторы
Iterator begin() const                                           // Начало
Iterator end() const                                             // Конец

// Операторы итератора:
operator*() const -> const std::pair<K, V>&                     // Разыменование (текущая пара)
operator++() -> Iterator&                                        // Префиксный инкремент (следующий элемент)
operator!=(const Iterator& other) const -> bool                 // Сравнение с другим итератором

// Хэширование и индексация
size_t getIndex(uint32_t bitmap, size_t hash_fragment) const    // Преобразование битовой маски в индекс

// Рекурсивные операции с деревом HAMT
std::shared_ptr<Node> insertNode(std::shared_ptr<Node> node,
    size_t hash, const K& key, const V& value, size_t level) const  // Рекурсивная вставка

const V* findNode(std::shared_ptr<Node> node,
    size_t hash, const K& key, size_t level) const                   // Рекурсивный поиск

// Метод обхода для итератора
void advance()                                                    // Перемещение к следующему элементу
```

### ❗️ **Как реализована персистентность:** Через **персистентное хеш-дерево (HAMT)**. ❗️ 

1. **Структура данных:** Комбинация хеш-таблицы и префиксного дерева
2. **Ключи** распределяются по дереву на основе их хеш-кода
3. **При добавлении/изменении пары ключ-значение:**
   - От корня до листа создается новая цепочка узлов
   - В листовом узле добавляется/изменяется запись
   - Если узел переполняется - он делится на несколько дочерних
4. **Коллизии** хранятся в маленьких массивах в листах

### 6. Фабрика для преобразования между структурами - **`persistent_factory.hpp`**

**Доступные методы**:
```cpp
// Преобразования между списками и векторами
template<typename T>
static PersistentVector<T> listToVector(const PersistentList<T>& list)

template<typename T>
static PersistentList<T> vectorToList(const PersistentVector<T>& vector)

// Преобразования с участием словарей
template<typename K, typename V>
static PersistentVector<std::pair<K, V>> mapToVector(const PersistentMap<K, V>& map)

template<typename K, typename V>
static PersistentList<std::pair<K, V>> mapToList(const PersistentMap<K, V>& map)

template<typename K, typename V>
static PersistentMap<K, V> vectorToMap(const std::vector<std::pair<K, V>>& vec)

template<typename K, typename V>
static PersistentMap<K, V> persistentVectorToMap(const PersistentVector<std::pair<K, V>>& vec)
```
### ❗️ **Как реализована персистентность:** Через **умное переиспользование**. ❗️

**Конкретный механизм:**
1. **При преобразовании** между структурами фабрика старается **максимально использовать существующие данные**
2. **Пример:** Преобразование списка в вектор:
   - Не копирует все элементы заново
   - Использует существующие узлы списка при создании дерева вектора
   - Там, где возможно, сохраняет те же объекты в памяти
3. **Цель:** Минимизировать копирование при сохранении персистентности
### ❗️ **Реализует пункт 4 из дополнительных требований** - "экономичное преобразование структур". Фабрика старается максимально использовать разделение данных вместо полного копирования. ❗️

---

## Реализация пункта 3: "Более эффективное представление чем fat-node"

### **1. PersistentVector (persistent_vector.hpp/impl.hpp)**
**В коде (persistent_vector_impl.hpp)**:
```cpp
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
```

### **2. PersistentMap (persistent_map.hpp/impl.hpp)**
**Hash Array Mapped Trie (HAMT) вместо fat-node**

```cpp
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

### **Пример на vector:**

#### **Обычный `std::vector`:**
- Изменение: Модифицирует существующий объект
- Копирование: Полное копирование всех элементов
- Версионность: Невозможна без явного копирования

#### **Наш `PersistentVector`:**
- Изменение: Возвращает новый объект, старый неизменен
- Копирование: Только измененные части дерева
- Версионность: Встроена в саму структуру


## Примеры использования

### Работа с вектором
```cpp
PersistentVector<int> vec1;
auto vec2 = vec1.append(1).append(2).append(3);
auto vec3 = vec2.set(1, 42);  // Изменяем второй элемент

// vec2 остается неизменным: [1, 2, 3]
// vec3: [1, 42, 3]
```

### Работа со списком и zipper
```cpp
PersistentList<int> list;
auto list2 = list.prepend(3).prepend(2).prepend(1);

auto zipper = list2.getZipper(1);  // Позиция на элементе 2
auto list3 = zipper.insertAfter(99).toList();  // [1, 2, 99, 3]
```

### Работа с массивом
```cpp
PersistentMap<std::string, int> map;
auto map2 = map.set("apple", 5).set("banana", 3);
auto map3 = map2.set("apple", 10);  // Обновляем значение

std::cout << map2.at("apple");  // 5
std::cout << map3.at("apple");  // 10
```

### Использование фабрики
```cpp
PersistentList<int> list = PersistentList<int>({1, 2, 3, 4, 5});
auto vector = PersistentFactory::listToVector(list);
auto map = PersistentFactory::vectorToMap({{"a", 1}, {"b", 2}});
```

---

## Визуализация связей между файлами

```
                    ┌─────────────────────┐
                    │   persistent_value  │ ← Универсальное значение
                    │    (вложенность)    │
                    └──────────┬──────────┘
                               │
         ┌─────────────┬─────────────────┬───────────────┐
         │             │                 │               │
         ▼             ▼                 ▼               ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌────────────────┐
│  persistent  │ │  persistent  │ │  persistent  │ │   persistent   │
│   vector     │ │    list      │ │     map      │ │    factory     │
│  (массив)    │ │  (список)    │ │  (словарь)   │ │(преобразования)│
└──────┬───────┘ └──────┬───────┘ └──────┬───────┘ └────────────────┘
       │                │                │
       └────────┬───────┴───────┬────────┘
                │               │
                ▼               ▼
         ┌─────────────┐ ┌──────────────┐
         │ IPersistent │ │  Алгоритмы:  │
         │ Structure   │ │ • VectorTrie │
         │ (интерфейс) │ │ • HAMT       │
         └─────────────┘ └──────────────┘
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

# Создайте папку сборки проекта
mkdir build
cd build

# Скомпилируйте программу
cmake ..
cmake --build . --config Debug

# Запустите скомпилированную программу
.\Debug\persistent_tests.exe
```
# Тесты

## **PersistentVectorTest** (Тесты для неизменяемого вектора)

### 1. `EmptyVectorCreation` - Создание пустого вектора
- Проверяет корректность создания пустого вектора
- Убеждается, что вектор действительно пуст (empty() = true)
- Проверяет размер равен 0

### 2. `AppendingElements` - Добавление элементов
- Тестирует последовательное добавление элементов
- Проверяет, что размер увеличивается правильно
- Убеждается, что элементы сохраняются в правильном порядке

### 3. `ModifyingElements` - Изменение элементов
- Тестирует метод set() для изменения существующих элементов
- Проверяет, что оригинальный вектор остаётся неизменным
- Убеждается, что новый вектор содержит изменённые значения

### 4. `RemovingElementsPopBack` - Удаление элементов (pop_back)
- Проверяет удаление последнего элемента
- Убеждается, что размер уменьшается на 1
- Проверяет, что оригинальный вектор не изменяется

### 5. `IndexAccess` - Доступ по индексу
- Тестирует получение элементов по индексу
- Проверяет корректность работы с различными типами данных (std::string)

### 6. `ExceptionHandling` - Обработка исключений
- Проверяет выбрасывание исключений при выходе за границы
- Тестирует get() с недопустимым индексом
- Тестирует set() с недопустимым индексом

### 7. `OperationChaining` - Цепочки операций
- Тестирует последовательное выполнение операций
- Проверяет корректность работы комбинаций методов

### 8. `VectorComparison` - Сравнение векторов
- (Закомментирован) Предполагает проверку равенства векторов
- Тестировал бы сравнение через toString() если бы был реализован

### 9. `VectorWithDifferentTypes` - Векторы разных типов
- Тестирует работу с int, string, double
- Проверяет типизацию шаблонного класса

### 10. `PerformanceTest` - Тест производительности
- Проверяет производительность при добавлении 1000 элементов
- Тестирует масштабируемость структуры данных

## **PersistentListTest** (Тесты для неизменяемого списка)

### 1. `EmptyListCreation` - Создание пустого списка
- Аналогично вектору, проверяет пустой список

### 2. `PrependingElements` - Добавление в начало
- Тестирует prepend() для добавления элементов в начало
- Проверяет порядок элементов (LIFO)

### 3. `GettingTail` - Получение хвоста списка
- Тестирует метод tail()
- Проверяет, что tail() возвращает список без первого элемента
- Убеждается в корректности размеров

### 4. `ConcatenatingLists` - Конкатенация списков
- Тестирует объединение двух списков
- Проверяет порядок элементов после конкатенации

### 5. `ExceptionHandling` - Обработка исключений
- Проверяет исключения для пустого списка
- Тестирует front() и tail() на пустом списке

### 6. `OperationChaining` - Цепочки операций
- Тестирует комбинации методов списка

### 7. `ListComparison` - Сравнение списков
- (Закомментирован) Предполагаемая проверка равенства списков

### 8. `ListWithDifferentTypes` - Списки разных типов
- Тестирует списки с int, string, double

### 9. `ImmutabilityTest` - Проверка неизменяемости
- Тестирует основное свойство persistent структур
- Убеждается, что операции создают новые объекты, не изменяя старые

### 10. `LargeListTest` - Большой список
- Тестирует производительность при добавлении 100 элементов
- Проверяет корректность последовательного обхода

## **PersistentMapTest** (Тесты для неизменяемого массива)

### 1. `EmptyMapCreation` - Создание пустой массива
- Проверяет создание пустого массива

### 2. `AddingElements` - Добавление элементов
- Тестирует set() для добавления пар ключ-значение
- Проверяет увеличение размера

### 3. `UpdatingElements` - Обновление элементов
- Тестирует перезапись значений по существующему ключу
- Проверяет, что оригинальный массив не изменяется

### 4. `RemovingElements` - Удаление элементов
- Тестирует erase() для удаления по ключу
- Проверяет наличие/отсутствие ключей

### 5. `CheckingKeyExistence` - Проверка наличия ключа
- Тестирует метод contains()
- Проверяет как существующие, так и отсутствующие ключи

### 6. `AccessingValues` - Доступ к значениям
- Тестирует метод at() для получения значений

### 7. `ExceptionHandling` - Обработка исключений
- Проверяет at() с несуществующим ключом

### 8. `OperationChaining` - Цепочки операций
- Тестирует комбинации set(), erase()

### 9. `MapWithDifferentValueTypes` - Массивы с разными типами значений
- Тестирует массивы с int, string, double значениями

### 10. `LargeMapTest` - Большая массив
- Тестирует производительность при добавлении 100 элементов

### 11. `ImmutabilityTest` - Проверка неизменяемости
- Проверяет, что операции не модифицируют оригинальные массивы

### 12. `MapComparison` - Сравнение миссивов
- (Закомментирован) Предполагаемое сравнение массивов

## **NestingTest** (Тесты для вложенных структур)

### 1. `VectorOfVectors` - Вектор векторов
- Тестирует вложение векторов друг в друга
- Проверяет доступ к элементам вложенных структур

### 2. `ListOfLists` - Список списков
- Тестирует вложение списков
- Проверяет корректность размеров и элементов

### 3. `MapWithVectorValues` - Массив со значениями-векторами
- Тестирует массив, где значения являются векторами
- Проверяет сложную структуру данных

### 4. `DeepNesting` - Глубокое вложение
- Тестирует многоуровневые структуры (списки в массивах в векторах)
- Проверяет корректность работы с глубокими структурами

### 5. `PersistentValueConstructors` - Конструкторы PersistentValue
- Тестирует создание PersistentValue разных типов
- Проверяет методы определения типа (isInt, isDouble и т.д.)
- Тестирует преобразование значений

### 6. `ModifyingNestedStructures` - Модификация вложенных структур
- Тестирует изменение элементов в сложных структурах
- Проверяет неизменяемость оригинальных структур

## **EdgeCasesTest** (Тесты граничных случаев)

### 1. `VectorWithMaximumOperations` - Вектор с максимальным количеством операций
- Тестирует смешанные операции (append + set)
- Проверяет стабильность при интенсивном использовании

### 2. `ListWithOperationAlternation` - Список с чередованием операций
- Тестирует чередование разных операций над списками

### 3. `MapWithOverwriteChain` - Массив с цепочкой перезаписей
- Тестирует многократную перезапись одного ключа
- Проверяет конечное значение

### 4. `CombinedStructures` - Комбинированные структуры
- Тестирует сложные комбинации разных структур данных