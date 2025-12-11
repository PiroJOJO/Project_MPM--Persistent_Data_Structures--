#include <iostream>
#include "../include/persistent_vector.hpp"
#include "../include/persistent_list.hpp"
#include "../include/persistent_map.hpp"
#include "../include/persistent_factory.hpp"

int main() {
    // -----------------------------------------
    // ---- Тестирование PersistentVector ------
    // -----------------------------------------
    std::cout << "=== Testing PersistentVector ===" << std::endl;
    PersistentVector<int> vec1;
    auto vec2 = vec1.append(1).append(2).append(3);
    auto vec3 = vec2.set(1, 42);
    
    std::cout << "vec2[1] = " << vec2.get(1) << std::endl; // 2
    std::cout << "vec3[1] = " << vec3.get(1) << std::endl; // 42
    std::cout << "vec2 size: " << vec2.size() << std::endl;
    
    // -----------------------------------------
    // ------ Тестирование PersistentList ------
    // -----------------------------------------
    std::cout << "\n=== Testing PersistentList ===" << std::endl;
    PersistentList<std::string> list1;
    auto list2 = list1.prepend("world").prepend("hello");
    
    std::cout << "First element: " << list2.front() << std::endl;
    
    // -----------------------------------------
    // ------ Тестирование преобразований ------
    // -----------------------------------------
    std::cout << "\n=== Testing Conversions ===" << std::endl;
    auto list3 = PersistentFactory::vectorToList(vec2);
    std::cout << "Converted list front: " << list3.front() << std::endl;
    
    // -----------------------------------------
    // ------- Тестирование PersistentMap ------
    // -----------------------------------------
    std::cout << "\n=== Testing PersistentMap ===" << std::endl;
    PersistentMap<std::string, int> map1;
    auto map2 = map1.set("one", 1).set("two", 2).set("three", 3);
    auto map3 = map2.set("two", 42);
    
    std::cout << "map2[\"two\"] = " << map2.at("two") << std::endl; // 2
    std::cout << "map3[\"two\"] = " << map3.at("two") << std::endl; // 42
    std::cout << "map3 contains \"four\": " << map3.contains("four") << std::endl;
    
    return 0;
}