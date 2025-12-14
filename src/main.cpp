#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>

#include "persistent_vector.hpp"
#include "persistent_list.hpp"
#include "persistent_map.hpp"
#include "persistent_value.hpp"
#include "persistent_data_structure.hpp"

#include "persistent_vector_impl.hpp"
#include "persistent_list_impl.hpp"
#include "persistent_map_impl.hpp"

// -----------------------------------------
// ------ ТЕСТЫ ДЛЯ PERSISTENT VECTOR ------
// -----------------------------------------

class PersistentVectorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};
// Создание пустого вектора
TEST_F(PersistentVectorTest, EmptyVectorCreation) {
    PersistentVector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
}
// Добавление элементов
TEST_F(PersistentVectorTest, AppendingElements) {
    PersistentVector<int> vec;
    auto vec1 = vec.append(1);
    auto vec2 = vec1.append(2);
    auto vec3 = vec2.append(3);

    EXPECT_EQ(vec1.size(), 1);
    EXPECT_EQ(vec2.size(), 2);
    EXPECT_EQ(vec3.size(), 3);
    EXPECT_EQ(vec1.get(0), 1);
    EXPECT_EQ(vec2.get(1), 2);
    EXPECT_EQ(vec3.get(2), 3);
}
// Изменение элементов
TEST_F(PersistentVectorTest, ModifyingElements) {
    PersistentVector<int> vec;
    auto vec1 = vec.append(1).append(2).append(3);
    auto vec2 = vec1.set(1, 99);

    EXPECT_EQ(vec1.get(0), 1);
    EXPECT_EQ(vec1.get(1), 2);
    EXPECT_EQ(vec1.get(2), 3);
    EXPECT_EQ(vec2.get(0), 1);
    EXPECT_EQ(vec2.get(1), 99);
    EXPECT_EQ(vec2.get(2), 3);
}
// Удаление элементов
TEST_F(PersistentVectorTest, RemovingElementsPopBack) {
    PersistentVector<int> vec;
    auto vec1 = vec.append(1).append(2).append(3);
    auto vec2 = vec1.pop_back();

    EXPECT_EQ(vec1.size(), 3);
    EXPECT_EQ(vec2.size(), 2);
    EXPECT_EQ(vec1.get(2), 3);
}
// Доступ по индексу
TEST_F(PersistentVectorTest, IndexAccess) {
    PersistentVector<std::string> vec;
    auto vec1 = vec.append("a").append("b").append("c");

    EXPECT_EQ(vec1.get(0), "a");
    EXPECT_EQ(vec1.get(1), "b");
    EXPECT_EQ(vec1.get(2), "c");
}
// Обработка исключений
TEST_F(PersistentVectorTest, ExceptionHandling) {
    PersistentVector<int> vec;
    auto vec1 = vec.append(1);

    EXPECT_THROW(vec1.get(5), std::out_of_range);
    EXPECT_THROW(vec1.set(5, 10), std::out_of_range);
}
// Цепочки операций
TEST_F(PersistentVectorTest, OperationChaining) {
    PersistentVector<int> vec;
    auto result = vec.append(1)
        .append(2)
        .set(0, 10)
        .append(3)
        .pop_back()
        .set(1, 20);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.get(0), 10);
    EXPECT_EQ(result.get(1), 20);
}
// Сравнение векторов
TEST_F(PersistentVectorTest, VectorComparison) {
    PersistentVector<int> vec1, vec2;
    auto v1 = vec1.append(1).append(2).append(3);
    auto v2 = vec2.append(1).append(2).append(3);
    auto v3 = v1.set(1, 99);
}
// Векторы разных типов
TEST_F(PersistentVectorTest, VectorWithDifferentTypes) {
    PersistentVector<int> intVec;
    auto intResult = intVec.append(1).append(2).append(3);
    EXPECT_EQ(intResult.size(), 3);
    EXPECT_EQ(intResult.get(1), 2);

    PersistentVector<std::string> strVec;
    auto strResult = strVec.append("hello").append("world");
    EXPECT_EQ(strResult.size(), 2);
    EXPECT_EQ(strResult.get(0), "hello");

    PersistentVector<double> doubleVec;
    auto doubleResult = doubleVec.append(1.1).append(2.2);
    EXPECT_EQ(doubleResult.size(), 2);
    EXPECT_DOUBLE_EQ(doubleResult.get(0), 1.1);
}
// Тест производительности
TEST_F(PersistentVectorTest, PerformanceTest) {
    PersistentVector<int> vec;
    auto current = vec;

    for (int i = 0; i < 1000; ++i) {
        current = current.append(i);
    }

    EXPECT_EQ(current.size(), 1000);
    EXPECT_EQ(current.get(999), 999);
}

// -----------------------------------------
// ------- ТЕСТЫ ДЛЯ PERSISTENT LIST -------
// -----------------------------------------

class PersistentListTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
// Создание пустого списка
TEST_F(PersistentListTest, EmptyListCreation) {
    PersistentList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}
// Добавление в начало
TEST_F(PersistentListTest, PrependingElements) {
    PersistentList<int> list;
    auto list1 = list.prepend(1);
    auto list2 = list1.prepend(2);
    auto list3 = list2.prepend(3);

    EXPECT_EQ(list1.size(), 1);
    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list3.size(), 3);
    EXPECT_EQ(list1.front(), 1);
    EXPECT_EQ(list2.front(), 2);
    EXPECT_EQ(list3.front(), 3);
}
// Получение хвоста списка
TEST_F(PersistentListTest, GettingTail) {
    PersistentList<std::string> list;
    auto list1 = list.prepend("c").prepend("b").prepend("a");
    auto tail1 = list1.tail();
    auto tail2 = tail1.tail();

    EXPECT_EQ(list1.front(), "a");
    EXPECT_EQ(tail1.front(), "b");
    EXPECT_EQ(tail2.front(), "c");
    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(tail1.size(), 2);
    EXPECT_EQ(tail2.size(), 1);
}
// Конкатенация списков
TEST_F(PersistentListTest, ConcatenatingLists) {
    PersistentList<int> list1, list2;
    auto l1 = list1.prepend(3).prepend(2).prepend(1);  // [1, 2, 3]
    auto l2 = list2.prepend(6).prepend(5).prepend(4);  // [4, 5, 6]
    auto concatenated = l1.concat(l2);  // [1, 2, 3, 4, 5, 6]

    EXPECT_EQ(concatenated.size(), 6);

    auto current = concatenated;
    int expected[] = { 1, 2, 3, 4, 5, 6 };
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(current.front(), expected[i]);
        current = current.tail();
    }
}
// Обработка исключений
TEST_F(PersistentListTest, ExceptionHandling) {
    PersistentList<int> emptyList;

    EXPECT_THROW(emptyList.front(), std::runtime_error);
    EXPECT_THROW(emptyList.tail(), std::runtime_error);
}
// Цепочки операций
TEST_F(PersistentListTest, OperationChaining) {
    PersistentList<std::string> list;
    auto result = list.prepend("world")
        .prepend("hello")
        .tail()
        .prepend("there")
        .concat(list.prepend("!"));

    EXPECT_EQ(result.size(), 3);
}
// Сравнение списков
TEST_F(PersistentListTest, ListComparison) {
    PersistentList<int> list1, list2;
    auto l1 = list1.prepend(3).prepend(2).prepend(1);
    auto l2 = list2.prepend(3).prepend(2).prepend(1);
    auto l3 = l1.tail();
}
// Списки разных типов
TEST_F(PersistentListTest, ListWithDifferentTypes) {
    PersistentList<int> intList;
    auto intResult = intList.prepend(3).prepend(2).prepend(1);
    EXPECT_EQ(intResult.size(), 3);
    EXPECT_EQ(intResult.front(), 1);

    PersistentList<std::string> strList;
    auto strResult = strList.prepend("world").prepend("hello");
    EXPECT_EQ(strResult.size(), 2);
    EXPECT_EQ(strResult.front(), "hello");

    PersistentList<double> doubleList;
    auto doubleResult = doubleList.prepend(3.14).prepend(2.71);
    EXPECT_EQ(doubleResult.size(), 2);
    EXPECT_DOUBLE_EQ(doubleResult.front(), 2.71);
}
// Проверка неизменяемости
TEST_F(PersistentListTest, ImmutabilityTest) {
    PersistentList<int> base;
    auto list1 = base.prepend(1).prepend(2).prepend(3);
    auto list2 = list1.tail();
    auto list3 = list1.prepend(0);

    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list3.size(), 4);
    EXPECT_EQ(list1.front(), 3);
    EXPECT_EQ(list3.front(), 0);
}
// Большой список
TEST_F(PersistentListTest, LargeListTest) {
    PersistentList<int> list;
    auto current = list;

    for (int i = 0; i < 100; ++i) {
        current = current.prepend(i);
    }

    EXPECT_EQ(current.size(), 100);

    auto temp = current;
    for (int i = 99; i >= 0; --i) {
        EXPECT_EQ(temp.front(), i);
        temp = temp.tail();
    }
}

// -----------------------------------------
// -------- ТЕСТЫ ДЛЯ PERSISTENT MAP -------
// -----------------------------------------
class PersistentMapTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
// Создание пустого массива
TEST_F(PersistentMapTest, EmptyMapCreation) {
    PersistentMap<std::string, int> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}
// Добавление элементов
TEST_F(PersistentMapTest, AddingElements) {
    PersistentMap<std::string, int> map;
    auto map1 = map.set("one", 1);
    auto map2 = map1.set("two", 2);
    auto map3 = map2.set("three", 3);

    EXPECT_EQ(map1.size(), 1);
    EXPECT_EQ(map2.size(), 2);
    EXPECT_EQ(map3.size(), 3);
    EXPECT_EQ(map1.at("one"), 1);
    EXPECT_EQ(map2.at("two"), 2);
    EXPECT_EQ(map3.at("three"), 3);
}
// Обновление элементов
TEST_F(PersistentMapTest, UpdatingElements) {
    PersistentMap<std::string, std::string> map;
    auto map1 = map.set("name", "Alice").set("age", "30");
    auto map2 = map1.set("name", "Bob");

    EXPECT_EQ(map1.at("name"), "Alice");
    EXPECT_EQ(map1.at("age"), "30");
    EXPECT_EQ(map2.at("name"), "Bob");
    EXPECT_EQ(map2.at("age"), "30");
    EXPECT_EQ(map1.size(), 2);
    EXPECT_EQ(map2.size(), 2);
}
// Удаление элементов
TEST_F(PersistentMapTest, RemovingElements) {
    PersistentMap<std::string, int> map;
    auto map1 = map.set("a", 1).set("b", 2).set("c", 3);
    auto map2 = map1.erase("b");

    EXPECT_EQ(map1.size(), 3);
    EXPECT_EQ(map2.size(), 2);
    EXPECT_TRUE(map1.contains("b"));
    EXPECT_FALSE(map2.contains("b"));
    EXPECT_TRUE(map2.contains("a"));
    EXPECT_TRUE(map2.contains("c"));
}
// Проверка наличия ключа
TEST_F(PersistentMapTest, CheckingKeyExistence) {
    PersistentMap<std::string, double> map;
    auto map1 = map.set("pi", 3.14).set("e", 2.71);

    EXPECT_TRUE(map1.contains("pi"));
    EXPECT_TRUE(map1.contains("e"));
    EXPECT_FALSE(map1.contains("phi"));
}
// Доступ к значениям
TEST_F(PersistentMapTest, AccessingValues) {
    PersistentMap<std::string, std::string> map;
    auto map1 = map.set("language", "C++").set("version", "17");

    EXPECT_EQ(map1.at("language"), "C++");
    EXPECT_EQ(map1.at("version"), "17");
}
// Обработка исключений
TEST_F(PersistentMapTest, ExceptionHandling) {
    PersistentMap<std::string, int> map;
    EXPECT_THROW(map.at("nonexistent"), std::out_of_range);
}
// Цепочки операций
TEST_F(PersistentMapTest, OperationChaining) {
    PersistentMap<std::string, int> map;
    auto result = map.set("a", 1)
        .set("b", 2)
        .erase("a")
        .set("c", 3)
        .set("b", 20);

    EXPECT_EQ(result.size(), 2);
    EXPECT_FALSE(result.contains("a"));
    EXPECT_EQ(result.at("b"), 20);
    EXPECT_EQ(result.at("c"), 3);
}
// Массивы с разными типами значений
TEST_F(PersistentMapTest, MapWithDifferentValueTypes) {
    PersistentMap<std::string, int> intMap;
    auto intResult = intMap.set("count", 42).set("index", 0);
    EXPECT_EQ(intResult.size(), 2);
    EXPECT_EQ(intResult.at("count"), 42);

    PersistentMap<std::string, std::string> strMap;
    auto strResult = strMap.set("name", "Alice").set("city", "Wonderland");
    EXPECT_EQ(strResult.size(), 2);
    EXPECT_EQ(strResult.at("name"), "Alice");

    PersistentMap<std::string, double> doubleMap;
    auto doubleResult = doubleMap.set("pi", 3.14159).set("e", 2.71828);
    EXPECT_EQ(doubleResult.size(), 2);
    EXPECT_DOUBLE_EQ(doubleResult.at("pi"), 3.14159);
}
// Большой массив
TEST_F(PersistentMapTest, LargeMapTest) {
    PersistentMap<int, std::string> map;
    auto current = map;

    for (int i = 0; i < 100; ++i) {
        current = current.set(i, "Value_" + std::to_string(i));
    }

    EXPECT_EQ(current.size(), 100);
    EXPECT_EQ(current.at(0), "Value_0");
    EXPECT_EQ(current.at(50), "Value_50");
    EXPECT_EQ(current.at(99), "Value_99");
}
// Проверка неизменяемости
TEST_F(PersistentMapTest, ImmutabilityTest) {
    PersistentMap<std::string, int> base;
    auto map1 = base.set("x", 10).set("y", 20);
    auto map2 = map1.set("x", 100);
    auto map3 = map1.erase("y");

    EXPECT_EQ(map1.at("x"), 10);
    EXPECT_EQ(map1.at("y"), 20);
    EXPECT_EQ(map2.at("x"), 100);
    EXPECT_EQ(map2.at("y"), 20);
    EXPECT_EQ(map3.at("x"), 10);
    EXPECT_FALSE(map3.contains("y"));
}
// Сравнение массивов
TEST_F(PersistentMapTest, MapComparison) {
    PersistentMap<std::string, int> map1, map2;
    auto m1 = map1.set("a", 1).set("b", 2);
    auto m2 = map2.set("a", 1).set("b", 2);
    auto m3 = m1.set("a", 100);
}

// -----------------------------------------
// ------ ТЕСТЫ ДЛЯ ВЛОЖЕННЫХ СТРУКТУР -----
// -----------------------------------------
class NestingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
// Вектор векторов
TEST_F(NestingTest, VectorOfVectors) {
    PersistentVector<int> inner1, inner2, inner3;
    inner1 = inner1.append(1).append(2).append(3);
    inner2 = inner2.append(4).append(5);
    inner3 = inner3.append(6).append(7).append(8).append(9);

    PersistentVector<PersistentVector<int>> matrix;
    matrix = matrix.append(inner1).append(inner2).append(inner3);

    EXPECT_EQ(matrix.size(), 3);
    EXPECT_EQ(matrix.get(0).size(), 3);
    EXPECT_EQ(matrix.get(1).size(), 2);
    EXPECT_EQ(matrix.get(2).size(), 4);
    EXPECT_EQ(matrix.get(0).get(0), 1);
    EXPECT_EQ(matrix.get(1).get(1), 5);
    EXPECT_EQ(matrix.get(2).get(3), 9);
}
// Список списков
TEST_F(NestingTest, ListOfLists) {
    PersistentList<std::string> listA, listB, listC;
    listA = listA.prepend("C").prepend("B").prepend("A");
    listB = listB.prepend("World").prepend("Hello");
    listC = listC.prepend("Z").prepend("Y").prepend("X");

    PersistentList<PersistentList<std::string>> listOfLists;
    listOfLists = listOfLists.prepend(listC).prepend(listB).prepend(listA);

    EXPECT_EQ(listOfLists.size(), 3);
    EXPECT_EQ(listOfLists.front().size(), 3);
    EXPECT_EQ(listOfLists.tail().front().size(), 2);
}
// Массив со значениями векторами
TEST_F(NestingTest, MapWithVectorValues) {
    PersistentVector<int> scores, ages, weights;
    scores = scores.append(95).append(87).append(92).append(88);
    ages = ages.append(25).append(30).append(28);
    weights = weights.append(70).append(65).append(68).append(72).append(67);

    PersistentMap<std::string, PersistentVector<int>> data;
    data = data.set("scores", scores)
        .set("ages", ages)
        .set("weights", weights);

    EXPECT_EQ(data.size(), 3);
    EXPECT_EQ(data.at("scores").size(), 4);
    EXPECT_EQ(data.at("ages").size(), 3);
    EXPECT_EQ(data.at("weights").size(), 5);
    EXPECT_EQ(data.at("scores").get(0), 95);
    EXPECT_EQ(data.at("ages").get(1), 30);
    EXPECT_EQ(data.at("weights").get(4), 67);
}
// Глубокое вложение (списки в массивах в векторах)
TEST_F(NestingTest, DeepNesting) {
    PersistentList<int> evenNumbers, oddNumbers, primeNumbers;
    evenNumbers = evenNumbers.prepend(8).prepend(6).prepend(4).prepend(2);
    oddNumbers = oddNumbers.prepend(9).prepend(7).prepend(5).prepend(3).prepend(1);
    primeNumbers = primeNumbers.prepend(11).prepend(7).prepend(5).prepend(3).prepend(2);

    PersistentMap<std::string, PersistentList<int>> student1, student2;
    student1 = student1.set("even", evenNumbers).set("odd", oddNumbers);
    student2 = student2.set("even", evenNumbers).set("prime", primeNumbers);

    PersistentVector<PersistentMap<std::string, PersistentList<int>>> classData;
    classData = classData.append(student1).append(student2);

    EXPECT_EQ(classData.size(), 2);
    EXPECT_TRUE(classData.get(0).contains("even"));
    EXPECT_TRUE(classData.get(0).contains("odd"));
    EXPECT_FALSE(classData.get(0).contains("prime"));
    EXPECT_TRUE(classData.get(1).contains("even"));
    EXPECT_TRUE(classData.get(1).contains("prime"));
    EXPECT_EQ(classData.get(0).at("even").front(), 2);
    EXPECT_EQ(classData.get(1).at("prime").front(), 2);
}
// Тестирует создание PersistentValue разных типов
TEST_F(NestingTest, PersistentValueConstructors) {
    PersistentValue nullValue;
    PersistentValue intValue(100);
    PersistentValue doubleValue(3.14);
    PersistentValue boolValue(false);
    PersistentValue stringValue("test");

    PersistentVector<int> testVec;
    testVec = testVec.append(1).append(2);
    PersistentValue vectorValue(testVec);

    PersistentMap<std::string, int> testMap;
    testMap = testMap.set("key", 123);
    PersistentValue mapValue(testMap);

    EXPECT_TRUE(nullValue.isNull());
    EXPECT_TRUE(intValue.isInt());
    EXPECT_TRUE(doubleValue.isDouble());
    EXPECT_TRUE(boolValue.isBool());
    EXPECT_TRUE(stringValue.isString());

    EXPECT_EQ(intValue.asInt(), 100);
    EXPECT_DOUBLE_EQ(doubleValue.asDouble(), 3.14);
    EXPECT_EQ(boolValue.asBool(), false);
    EXPECT_EQ(stringValue.asString(), "test");
}
// Модификация вложенных структур
TEST_F(NestingTest, ModifyingNestedStructures) {
    PersistentVector<int> row1, row2;
    row1 = row1.append(1).append(2).append(3);
    row2 = row2.append(4).append(5).append(6);

    PersistentVector<PersistentVector<int>> matrix;
    matrix = matrix.append(row1).append(row2);

    int originalRow1Size = row1.size();
    int originalMatrixSize = matrix.size();
    int originalValue = matrix.get(0).get(1);

    PersistentVector<int> modifiedRow1 = row1.set(1, 99);
    PersistentVector<PersistentVector<int>> modifiedMatrix = matrix.set(0, modifiedRow1);

    EXPECT_EQ(row1.size(), originalRow1Size);
    EXPECT_EQ(matrix.size(), originalMatrixSize);
    EXPECT_EQ(matrix.get(0).get(1), originalValue);
    EXPECT_EQ(modifiedMatrix.get(0).get(1), 99);
    EXPECT_EQ(modifiedMatrix.get(1).get(2), 6);
}

// -----------------------------------------
// ------ ТЕСТЫ ДЛЯ ГРАНИЧНЫХ СЛУЧАЕВ ------
// -----------------------------------------

class EdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};
// Вектор с максимальным количеством операций
TEST_F(EdgeCasesTest, VectorWithMaximumOperations) {
    PersistentVector<int> vec;
    auto current = vec;
    for (int i = 0; i < 50; ++i) {
        current = current.append(i);
        if (i % 10 == 0) {
            current = current.set(i / 2, i * 10);
        }
    }

    EXPECT_EQ(current.size(), 50);
}
// Список с чередованием операций
TEST_F(EdgeCasesTest, ListWithOperationAlternation) {
    PersistentList<int> list;
    auto l1 = list.prepend(1).prepend(2).prepend(3);
    auto l2 = l1.tail().prepend(0);
    auto l3 = l1.concat(l2);

    EXPECT_EQ(l1.size(), 3);
    EXPECT_EQ(l2.size(), 3);
    EXPECT_EQ(l3.size(), 6);
}
// Многократную перезапись одного ключа в MAP
TEST_F(EdgeCasesTest, MapWithOverwriteChain) {
    PersistentMap<std::string, int> map;
    auto result = map.set("a", 1)
        .set("a", 2)
        .set("a", 3)
        .set("a", 4);

    EXPECT_EQ(result.at("a"), 4);
    EXPECT_EQ(result.size(), 1);
}
// Комбинированные структуры
TEST_F(EdgeCasesTest, CombinedStructures) {
    PersistentVector<PersistentList<int>> vectorOfLists;
    PersistentList<int> list1, list2;
    list1 = list1.prepend(3).prepend(2).prepend(1);
    list2 = list2.prepend(6).prepend(5).prepend(4);

    auto vec = vectorOfLists.append(list1).append(list2);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec.get(0).size(), 3);

    PersistentMap<std::string, PersistentVector<int>> mapOfVectors;
    PersistentVector<int> vec1, vec2;
    vec1 = vec1.append(1).append(2);
    vec2 = vec2.append(3).append(4).append(5);

    auto mp = mapOfVectors.set("first", vec1).set("second", vec2);
    EXPECT_EQ(mp.size(), 2);
    EXPECT_EQ(mp.at("first").size(), 2);
    EXPECT_EQ(mp.at("second").size(), 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}